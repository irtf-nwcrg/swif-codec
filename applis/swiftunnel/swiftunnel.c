/*
 * SWIF Codec TUN tunnel — FEC-protected bidirectional UDP tunnel.
 *
 * Each endpoint runs both an encoder (local TUN → UDP) and a decoder
 * (UDP → local TUN).  Packets read from the TUN interface are padded to
 * SYMBOL_SIZE and sent as FEC source symbols.  A repair symbol is
 * generated every k source symbols.  On the receiving side, source
 * symbols are written to TUN immediately; FEC-recovered symbols are
 * delivered via the decoded_source_symbol_callback.
 *
 * Inspired by tetrystunnel.c (ISAE / Tetrys project).
 *
 * Author: Emmanuel Lochin (ENAC)
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <stdbool.h>

#include "../../src/swif_api.h"

#define PERROR(x) do { perror(x); exit(1); } while (0)

/* buffer for reading from tun interface, must be >= 1500 */
#define BUFSIZE         1500
/* FEC symbol size: must cover the largest possible IP packet */
#define SYMBOL_SIZE     1500
#define DEFAULT_PORT    30001
#define DEFAULT_EW_SIZE 10      /* default max encoding window size */
#define DEFAULT_K       5       /* 1 repair symbol every k source symbols */

/*
 * Simplified FEC Payload Information, carried at the start of each UDP
 * packet.  All fields are in network byte order on the wire.
 */
typedef struct {
	uint16_t	is_source;	/* 1 = source symbol, 0 = repair symbol */
	uint16_t	repair_key;	/* PRNG seed (repair only) */
	uint16_t	nss;		/* number of source symbols in window (repair only) */
	esi_t		esi;		/* ESI of source, or first_esi of window for repair */
} fpi_t;

/*
 * The first PKT_LEN_SIZE bytes of every source symbol buffer store the
 * actual IP packet size so the receiver can trim the padding.
 */
#define PKT_LEN_SIZE	sizeof(uint16_t)

char		*progname;
char		 connexion[] = "connexion";
extern char	*optarg;

int		 tap_fd, sock_fd;
struct sockaddr_in from;
socklen_t	 fromlen;
uint8_t		*buf;		/* general-purpose send/recv buffer */
int		 verbosity = 0;

/**************************************************************************
 * tun_alloc: allocates or reconnects to a tun/tap device.               *
 **************************************************************************/
static int
tun_alloc(char *dev, int flags)
{
	struct ifreq	 ifr;
	int		 fd, err;
	char		*clonedev = "/dev/net/tun";

	if ((fd = open(clonedev, O_RDWR)) < 0) {
		perror("Opening /dev/net/tun");
		return fd;
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = flags;
	if (*dev) {
		snprintf(ifr.ifr_name, IFNAMSIZ, "%s", dev);
	}
	if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
		perror("ioctl(TUNSETIFF)");
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
	return fd;
}

/**************************************************************************
 * Encoder callback: source symbol removed from the left of the window.  *
 * Our pool is preallocated so there is nothing to free here.            *
 **************************************************************************/
static void
source_symbol_removed_from_coding_window_callback(void *context,
						  esi_t old_symbol_esi)
{
	(void)context;
	if (verbosity > 1)
		printf("encoder: symbol %u evicted from coding window\n",
		       old_symbol_esi);
}

/**************************************************************************
 * Decoder callback: a source symbol has been FEC-recovered.             *
 * Write the embedded IP packet to the TUN interface.                    *
 **************************************************************************/
static void
decoded_source_symbol_callback(void *context, void *new_symbol_buf,
				esi_t esi)
{
	uint16_t	 pkt_len;
	ssize_t		 nwrite;

	(void)context;
	pkt_len = ntohs(*(uint16_t *)new_symbol_buf);
	if (pkt_len == 0 || pkt_len > SYMBOL_SIZE - PKT_LEN_SIZE) {
		fprintf(stderr,
			"decoded callback: invalid pkt_len=%u for ESI=%u\n",
			pkt_len, esi);
		return;
	}
	if (verbosity > 0)
		printf("<= decoded: ESI=%u recovered (%u bytes), writing to TUN\n",
		       esi, pkt_len);
	nwrite = write(tap_fd, (uint8_t *)new_symbol_buf + PKT_LEN_SIZE, pkt_len);
	if (nwrite < 0)
		perror("write(tap_fd)");
}

/**************************************************************************
 * usage: prints usage and exits.                                         *
 **************************************************************************/
static void
usage(void)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s -i <ifname> [-s|-c <serverIP>] [options]\n",
		progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "Mandatory options:\n");
	fprintf(stderr, "\t-i <ifname>\t TUN interface name (e.g. tun0)\n");
	fprintf(stderr, "\t-s\t\t server mode (wait for client)\n");
	fprintf(stderr, "\t-c <serverIP>\t client mode (connect to server)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Optional options:\n");
	fprintf(stderr, "\t-p <port>\t UDP port (default %d)\n", DEFAULT_PORT);
	fprintf(stderr, "\t-k <value>\t 1 repair every k source packets"
		" (default %d)\n", DEFAULT_K);
	fprintf(stderr, "\t-w <size>\t max encoding window size"
		" (default %d)\n", DEFAULT_EW_SIZE);
	fprintf(stderr, "\t-v <level>\t verbosity 0=silent 1=info 2=debug"
		" (default 0)\n");
	fprintf(stderr, "\t-h\t\t prints this help text\n");
	exit(1);
}

/**************************************************************************
 * main                                                                   *
 **************************************************************************/
int
main(int argc, char *argv[])
{
	swif_codepoint_t codepoint = SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC;
	swif_encoder_t	*enc       = NULL;
	swif_decoder_t	*dec       = NULL;

	/* circular pool of source symbol buffers (one per window slot) */
	uint8_t		**src_pool  = NULL;

	/* encoding state */
	esi_t		  next_esi   = 0;  /* ESI for the next source symbol */
	uint32_t	  src_count  = 0;  /* source symbols since last repair */
	uint32_t	  repair_key = 0;  /* monotonically increasing repair key */

	int		  option;
	int		  flags    = IFF_TUN;
	char		  if_name[IFNAMSIZ] = "";
	char		  remote_ip[16]     = "";
	unsigned short	  port  = DEFAULT_PORT;
	uint32_t	  ew_size = DEFAULT_EW_SIZE;
	uint32_t	  k       = DEFAULT_K;
	int		  cliserv = -1;	/* 0=client, 1=server */
	int		  maxfd;
	struct sockaddr_in sin;
	ssize_t		  l;
	fd_set		  fdset, tmp_fdset;
	int		  select_ret;

	progname = argv[0];

	buf = malloc(sizeof(fpi_t) + SYMBOL_SIZE);
	if (buf == NULL) PERROR("malloc(buf)");

	/* parse options */
	while ((option = getopt(argc, argv, "i:sc:p:k:w:v:h")) > 0) {
		switch (option) {
		case 'h':
			usage();
			break;
		case 'i':
			strncpy(if_name, optarg, IFNAMSIZ - 1);
			break;
		case 's':
			cliserv = 1;
			break;
		case 'c':
			cliserv = 0;
			strncpy(remote_ip, optarg, 15);
			break;
		case 'p':
			port = (unsigned short)atoi(optarg);
			break;
		case 'k':
			k = (uint32_t)atoi(optarg);
			if (k < 1) {
				fprintf(stderr, "k must be >= 1, using 1\n");
				k = 1;
			}
			break;
		case 'w':
			ew_size = (uint32_t)atoi(optarg);
			if (ew_size < 2) {
				fprintf(stderr,
					"window size must be >= 2, using 2\n");
				ew_size = 2;
			}
			break;
		case 'v':
			verbosity = atoi(optarg);
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", option);
			usage();
		}
	}

	if (*if_name == '\0') {
		fprintf(stderr, "Must specify interface name (-i)\n");
		usage();
	}
	if (cliserv < 0) {
		fprintf(stderr, "Must specify client (-c) or server (-s) mode\n");
		usage();
	}
	if (cliserv == 0 && *remote_ip == '\0') {
		fprintf(stderr,
			"Client mode requires server address (-c <ip>)\n");
		usage();
	}

	/* open TUN interface */
	if ((tap_fd = tun_alloc(if_name, flags | IFF_NO_PI)) < 0) {
		fprintf(stderr,
			"Error connecting to TUN interface %s\n", if_name);
		exit(1);
	}
	printf("Connected to TUN interface %s\n", if_name);

	/* initialize SWIF encoder */
	if ((enc = swif_encoder_create(codepoint, verbosity, SYMBOL_SIZE,
				       ew_size)) == NULL) {
		fprintf(stderr, "swif_encoder_create() failed\n");
		exit(1);
	}
	if (swif_encoder_set_callback_functions(
			enc,
			source_symbol_removed_from_coding_window_callback,
			NULL) != SWIF_STATUS_OK) {
		fprintf(stderr, "swif_encoder_set_callback_functions() failed\n");
		exit(1);
	}

	/* initialize SWIF decoder (linear system = 2 x window) */
	if ((dec = swif_decoder_create(codepoint, verbosity, SYMBOL_SIZE,
				       ew_size, 2 * ew_size)) == NULL) {
		fprintf(stderr, "swif_decoder_create() failed\n");
		exit(1);
	}
	if (swif_decoder_set_callback_functions(
			dec,
			NULL,				/* removed from LS cb */
			NULL,				/* decodable cb */
			decoded_source_symbol_callback,
			NULL) != SWIF_STATUS_OK) {
		fprintf(stderr, "swif_decoder_set_callback_functions() failed\n");
		exit(1);
	}

	/* preallocate source symbol pool */
	if ((src_pool = calloc(ew_size, sizeof(uint8_t *))) == NULL)
		PERROR("calloc(src_pool)");
	for (uint32_t i = 0; i < ew_size; i++) {
		if ((src_pool[i] = calloc(1, SYMBOL_SIZE)) == NULL)
			PERROR("calloc(src_pool[i])");
	}

	/* create UDP socket and bind */
	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		PERROR("socket()");
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port        = htons(port);
	if (bind(sock_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		PERROR("bind()");

	fromlen = sizeof(from);

	/* C/S handshake — same mechanism as tetrystunnel */
	if (cliserv == 1) {	/* server: wait for client */
		while (1) {
			l = recvfrom(sock_fd, buf,
				     sizeof(fpi_t) + SYMBOL_SIZE, 0,
				     (struct sockaddr *)&from, &fromlen);
			if (l < 0) PERROR("INIT recvfrom()");
			if (strcmp(connexion, (char *)buf) == 0) {
				printf("Connection from %s:%d requested\n",
				       inet_ntoa(from.sin_addr),
				       ntohs(from.sin_port));
				break;
			}
		}
		l = sendto(sock_fd, connexion, sizeof(connexion), 0,
			   (struct sockaddr *)&from, fromlen);
		if (l < 0) PERROR("sendto()");
	} else {		/* client: connect to server */
		from.sin_family = AF_INET;
		from.sin_port   = htons(port);
		inet_aton(remote_ip, &from.sin_addr);
		l = sendto(sock_fd, connexion, sizeof(connexion), 0,
			   (struct sockaddr *)&from, sizeof(from));
		if (l < 0) PERROR("sendto()");
		printf("Connection request to %s:%d\n",
		       inet_ntoa(from.sin_addr), ntohs(from.sin_port));
		l = recvfrom(sock_fd, buf, sizeof(fpi_t) + SYMBOL_SIZE, 0,
			     (struct sockaddr *)&from, &fromlen);
		if (l < 0) PERROR("INIT recvfrom()");
	}
	printf("Connection established with %s:%d"
	       " [ew=%u, k=%u, symbol_size=%u]\n",
	       inet_ntoa(from.sin_addr), ntohs(from.sin_port),
	       ew_size, k, SYMBOL_SIZE);

	/* use select() to handle TUN and UDP simultaneously */
	maxfd = (tap_fd > sock_fd) ? tap_fd : sock_fd;
	FD_ZERO(&tmp_fdset);
	FD_SET(tap_fd,  &tmp_fdset);
	FD_SET(sock_fd, &tmp_fdset);

	while (1) {

		fdset = tmp_fdset;
		do {
			select_ret = select(maxfd + 1, &fdset, NULL, NULL, NULL);
		} while ((select_ret == -1) && (errno == EINTR));

		if (select_ret <= 0) {
			printf("select(): %s\n", strerror(errno));
			continue;
		}

		/*
		 * Packet from TUN: encode as a source symbol and send over UDP.
		 * Every k source symbols, also generate and send a repair symbol.
		 */
		if (FD_ISSET(tap_fd, &fdset)) {
			fpi_t		*fpi        = (fpi_t *)buf;
			uint8_t		*symbol_buf = buf + sizeof(fpi_t);
			uint8_t		*src_sym;
			esi_t		 first, last;
			uint32_t	 nss;

			/* pick a buffer from the circular pool */
			src_sym = src_pool[next_esi % ew_size];

			/* read IP packet; payload starts after the 2-byte length field */
			l = read(tap_fd, src_sym + PKT_LEN_SIZE,
				 SYMBOL_SIZE - PKT_LEN_SIZE);
			if (l <= 0) {
				perror("read(tap_fd)");
				continue;
			}
			/* store the actual packet length */
			*(uint16_t *)src_sym = htons((uint16_t)l);
			/* zero-pad the remainder of the symbol */
			memset(src_sym + PKT_LEN_SIZE + l, 0,
			       SYMBOL_SIZE - PKT_LEN_SIZE - l);

			if (verbosity > 1)
				printf("TUN -> source symbol ESI=%u (%zd bytes)\n",
				       next_esi, l);

			/* add to the encoding window */
			if (swif_encoder_add_source_symbol_to_coding_window(
					enc, src_sym, next_esi) != SWIF_STATUS_OK) {
				fprintf(stderr,
					"add_source_symbol_to_coding_window()"
					" failed (ESI=%u)\n", next_esi);
				continue;
			}

			/* send source packet: [FPI | symbol] */
			fpi->is_source  = htons(1);
			fpi->repair_key = htons(0);
			fpi->nss        = htons(0);
			fpi->esi        = htonl(next_esi);
			memcpy(symbol_buf, src_sym, SYMBOL_SIZE);
			if (sendto(sock_fd, buf, sizeof(fpi_t) + SYMBOL_SIZE, 0,
				   (struct sockaddr *)&from, fromlen) < 0) {
				perror("sendto(source)");
				continue;
			}

			if (verbosity > 0)
				printf("=> source ESI=%u (%zd bytes)\n",
				       next_esi, l);

			next_esi++;
			src_count++;

			/* generate a repair symbol every k source symbols */
			if (src_count % k == 0) {
				void *repair_buf = NULL;

				if (swif_encoder_generate_coding_coefs(
						enc, repair_key, 0)
						!= SWIF_STATUS_OK) {
					fprintf(stderr,
						"generate_coding_coefs()"
						" failed\n");
					continue;
				}
				if (swif_build_repair_symbol(enc, &repair_buf)
						!= SWIF_STATUS_OK) {
					fprintf(stderr,
						"build_repair_symbol()"
						" failed\n");
					continue;
				}
				if (swif_encoder_get_coding_window_information(
						enc, &first, &last, &nss)
						!= SWIF_STATUS_OK) {
					fprintf(stderr,
						"get_coding_window_information()"
						" failed\n");
					free(repair_buf);
					continue;
				}
				fpi->is_source  = htons(0);
				fpi->repair_key = htons((uint16_t)repair_key);
				fpi->nss        = htons((uint16_t)nss);
				fpi->esi        = htonl(first);
				memcpy(symbol_buf, repair_buf, SYMBOL_SIZE);
				free(repair_buf);

				if (sendto(sock_fd, buf,
					   sizeof(fpi_t) + SYMBOL_SIZE, 0,
					   (struct sockaddr *)&from,
					   fromlen) < 0)
					perror("sendto(repair)");

				if (verbosity > 0)
					printf("=> repair key=%u"
					       " window=[%u..%u] nss=%u\n",
					       repair_key, first, last, nss);
				repair_key++;
			}

		/*
		 * Packet from UDP: parse FPI, then either write directly to TUN
		 * (source symbol) or submit to the decoder (repair symbol).
		 */
		} else if (FD_ISSET(sock_fd, &fdset)) {
			struct sockaddr_in sout;
			socklen_t	   soutlen   = sizeof(sout);
			fpi_t		  *fpi       = (fpi_t *)buf;
			uint8_t		  *symbol_buf = buf + sizeof(fpi_t);
			uint16_t	   is_src, rkey, nss;
			esi_t		   esi;
			uint32_t	   i;

			l = recvfrom(sock_fd, buf, sizeof(fpi_t) + SYMBOL_SIZE,
				     0, (struct sockaddr *)&sout, &soutlen);
			if (l < 0) {
				perror("recvfrom()");
				continue;
			}
			if ((size_t)l < sizeof(fpi_t) + SYMBOL_SIZE) {
				fprintf(stderr,
					"short UDP packet (%zd bytes),"
					" discarding\n", l);
				continue;
			}

			is_src = ntohs(fpi->is_source);
			rkey   = ntohs(fpi->repair_key);
			nss    = ntohs(fpi->nss);
			esi    = ntohl(fpi->esi);

			if (is_src) {
				/* source symbol: write to TUN and submit to decoder */
				uint16_t pkt_len = ntohs(*(uint16_t *)symbol_buf);
				ssize_t  nwrite;

				if (verbosity > 0)
					printf("<= source ESI=%u (%u bytes)\n",
					       esi, pkt_len);

				if (pkt_len > 0 &&
				    pkt_len <= SYMBOL_SIZE - PKT_LEN_SIZE) {
					nwrite = write(tap_fd,
						       symbol_buf + PKT_LEN_SIZE,
						       pkt_len);
					if (nwrite < 0)
						perror("write(tap_fd)");
				} else {
					fprintf(stderr,
						"source ESI=%u: invalid"
						" pkt_len=%u\n", esi, pkt_len);
				}

				/* submit to decoder to help future decoding */
				swif_decoder_decode_with_new_source_symbol(
					dec, symbol_buf, esi);

			} else {
				/* repair symbol: set up coding window, then decode */
				if (verbosity > 0)
					printf("<= repair key=%u"
					       " window=[%u..%u] nss=%u\n",
					       rkey, esi, esi + nss - 1, nss);

				if (swif_decoder_reset_coding_window(dec)
						!= SWIF_STATUS_OK) {
					fprintf(stderr,
						"reset_coding_window()"
						" failed\n");
					continue;
				}
				for (i = esi; i < esi + nss; i++) {
					if (swif_decoder_add_source_symbol_to_coding_window(
							dec, i) != SWIF_STATUS_OK) {
						fprintf(stderr,
							"add_source_symbol_to"
							"_coding_window() failed"
							" (ESI=%u)\n", i);
						break;
					}
				}
				if (swif_decoder_generate_coding_coefs(
						dec, rkey, 0) != SWIF_STATUS_OK) {
					fprintf(stderr,
						"generate_coding_coefs()"
						" failed\n");
					continue;
				}
				if (swif_decoder_decode_with_new_repair_symbol(
						dec, symbol_buf, esi)
						!= SWIF_STATUS_OK) {
					fprintf(stderr,
						"decode_with_new_repair_symbol()"
						" failed\n");
					continue;
				}
			}
		}
	}

	/* cleanup (unreachable in normal operation) */
	swif_encoder_release(enc);
	swif_decoder_release(dec);
	for (uint32_t i = 0; i < ew_size; i++)
		free(src_pool[i]);
	free(src_pool);
	free(buf);
	return 0;
}
