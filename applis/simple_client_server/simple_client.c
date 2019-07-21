/*
 * Simple demo application on top of the SWIF-codec API.
 *
 * It is inspired from the same application from openFEC
 * (http://openfec.org/downloads.html) modified in order
 * to be used with the appropriate API.
 *
 * Author: Vincent Roca (Inria)
 */

#include "simple_client_server.h"


/* Prototypes */

/**
 * Opens and initializes a UDP socket, ready for receptions.
 */
static SOCKET	init_socket (void);

/**
 * This function receives packets on the incoming UDP socket.
 * It allocates a buffer of size *len and updates the pkt/len arguments with what
 * has been actually received. It works in blocking mode the first time it's called
 * (as the client can be launched a few seconds before the server), and after that
 * in non blocking (i.e. polling) mode. If no packet is received even after having
 * waited a certain time (0.2s), it return SWIF_STATUS_FAILURE to indicate that the
 * sender probably stopped all transmissions.
 */
static swif_status_t	get_next_pkt (SOCKET	so,
				      void	**pkt,
				      int32_t	*len);

/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void	dump_buffer_32 (void		*buf,
				uint32_t	len32);


/*************************************************************************************************/


int
main (int argc, char* argv[])
{
	swif_codepoint_t codepoint;				/* identifier of the codec to use */
	swif_decoder_t*	ses		= NULL;
	uint32_t	ew_size;				/* encoding window size */
	uint32_t	tot_src;				/* total number of source symbols */
	uint32_t	tot_enc;				/* total number of encoding symbols (i.e. source + repair) in the session */
	esi_t		esi;					/* source symbol id */
	SOCKET		so		= INVALID_SOCKET;	/* UDP socket for server => client communications */
	char*		pkt_with_fpi	= NULL;			/* buffer containing a fixed size packet plus a header consisting only of the FPI */
	fec_oti_t*	fec_oti		= NULL;			/* FEC Object Transmission Information as sent to the client */
	fpi_t*		fpi;					/* header (FEC Payload Information) for source and repair symbols */
	bool		done		= false;		/* true as soon as all source symbols have been received or recovered */
	uint32_t	ret		= -1;
	void**		recvd_symbols_tab= NULL;		/* table containing pointers to received symbols (no FPI here).
								 * The allocated buffer start 4 bytes (i.e., sizeof(FPI)) before... */
	void**		src_symbols_tab	= NULL;			/* table containing pointers to the source symbol buffers (no FPI here) */
	int32_t		len;					/* len of the received packet */
	uint32_t	n_received	= 0;			/* number of symbols (source or repair) received so far */


	/* First of all, initialize the UDP socket and wait for the FEC OTI to be received. This is absolutely required to
	 * synchronize encoder and decoder. We assume this first packet is NEVER lost otherwise decoding is not possible.
	 * In practice the sender can transmit it periodically, or it is sent through a separate reliable channel. */
	if ((so = init_socket()) == INVALID_SOCKET) {
		fprintf(stderr, "Error initializing socket!\n");
		ret = -1;
		goto end;
	}
	len = sizeof(fec_oti_t);		/* size of the expected packet */
	if ((ret = get_next_pkt(so, (void**)&fec_oti, &len)) != SWIF_STATUS_OK) {
		fprintf(stderr, "Error, get_next_pkt failed (FEC OTI reception)\n");
		ret = -1;
		goto end;
	}
	if (len != sizeof(fec_oti_t)) {
		fprintf(stderr, "Error, FEC OTI reception failed: bad size, expected %lu but received %d instead\n", sizeof(fec_oti_t), ret);
		ret = -1;
		goto end;
	}
	/* convert back to host endianess */
	codepoint	= ntohl(fec_oti->codepoint);
	ew_size		= ntohl(fec_oti->ew_size);
	tot_src		= ntohl(fec_oti->tot_src);
	tot_enc		= ntohl(fec_oti->tot_enc);

	printf("\nReceiving packets from %s/%d: codepoint %u, ew_size=%u, tot_src=%u, tot_enc=%u\n", DEST_IP, DEST_PORT, codepoint, ew_size, tot_src, tot_enc);

	/* and check the correctness of data received */
	if (tot_src > tot_enc || tot_src > 100000 || tot_enc > 100000) {
		fprintf(stderr, "Error, invalid FEC OTI received: tot_src_symbols=%u or tot_encoding_symbols=%u received are probably out of range\n", tot_src, tot_enc);
		ret = -1;
		goto end;
	}
	if (codepoint != SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC) {
		fprintf(stderr, "Error, invalid Codepoint %u.\n", codepoint);
		ret = -1;
		goto end;
	}
	if (ew_size > tot_src) {
		fprintf(stderr, "Error, invalid ew_size (%u), cannot be larger than tot_src (%u).\n", ew_size, tot_src);
		ret = -1;
		goto end;
	}
	/* Open and initialize the openfec decoding session now that we know the various parameters used by the sender/encoder... */
	if ((ses = swif_decoder_create(codepoint, VERBOSITY, SYMBOL_SIZE, ew_size, 2 * ew_size)) == NULL) {
		fprintf(stderr, "Error, swif_decoder_create() failed\n");
		ret = -1;
		goto end;
	}

	printf( "\nDecoding in progress. Waiting for new packets...\n" );

	/* allocate a table for the received encoding symbol buffers. We'll update it progressively */
	if (((recvd_symbols_tab = (void**) calloc(tot_enc, sizeof(void*))) == NULL) ||
	    ((src_symbols_tab = (void**) calloc(tot_enc, sizeof(void*))) == NULL)) {
		fprintf(stderr, "Error, no memory (tot_enc=%u)\n", tot_enc);
		ret = -1;
		goto end;
	}
	len = SYMBOL_SIZE + sizeof(fpi_t);	/* size of the expected packet */
	/*
	 * submit each fresh symbol to the library ASAP, upon reception.
	 */
	while ((ret = get_next_pkt(so, (void**)&pkt_with_fpi, &len)) == SWIF_STATUS_OK) {
		uint16_t	is_source;	/* 1 if source, 0 if repair */
		uint16_t	repair_key;	/* only meaningful in case of a repair */
		uint16_t	nss;		/* only meaningful in case of a repair */
		esi_t		esi;		/* esi of a source symbol, or esi of the first source symbol of the encoding window in case of a repair */
		uint32_t	i;

		/* OK, new packet received... */
		n_received++;
		fpi		= (fpi_t*)pkt_with_fpi;
		is_source	= ntohs(fpi->is_source);
		repair_key	= ntohs(fpi->repair_key);
		nss		= ntohs(fpi->nss);
		esi		= ntohl(fpi->esi);
printf("is_source=%u, key=%u, nss=%u, esi=%u\n", is_source, repair_key, nss, esi);
		if (esi > tot_enc) {		/* a sanity check, in case... */
			fprintf(stderr, "Error, invalid esi=%u received in a packet's FPI\n", esi);
			ret = -1;
			goto end;
		}
		if (is_source != 0 && is_source != 1) {
			fprintf(stderr, "Error, bad is_source=%u received in the packet's FPI\n", is_source);
			ret = -1;
			goto end;
		}
		recvd_symbols_tab[esi] = pkt_with_fpi + sizeof(fpi_t);	/* remember */
		printf("%05d => receiving symbol esi=%u (%s)\n", n_received, esi, (is_source) ? "src" : "repair");
		if (is_source) {
			if (swif_decoder_decode_with_new_source_symbol(ses, (char*)pkt_with_fpi + sizeof(fpi_t), esi) != SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_decoder_decode_with_new_source_symbol() failed\n");
				ret = -1;
				goto end;
			}
		} else {
			/* a bit more complex, it's a repair symbol: specify the coding window, generate the coding coefficients,
			 * then submit the repair symbol  */
printf("before swif_decoder_reset_coding_window\n");
			if (swif_decoder_reset_coding_window(ses) != SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_decoder_reset_coding_window() failed\n");
				ret = -1;
				goto end;
			}
printf("before swif_decoder_add_source_symbol_to_coding_window\n");
			for (i = esi; i < esi + nss; i++) {
				if (swif_decoder_add_source_symbol_to_coding_window(ses, i) != SWIF_STATUS_OK) {
					fprintf(stderr, "Error, swif_decoder_reset_coding_window() failed\n");
					ret = -1;
					goto end;
				}
			}
printf("before swif_generate_coding_coefs\n");
			if (swif_decoder_generate_coding_coefs(ses, repair_key, 0) != SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_decoder_generate_coding_coefs() failed\n");
				ret = -1;
				goto end;
			}
printf("before swif_decoder_decode_with_new_repair_symbol\n");
			if (swif_decoder_decode_with_new_repair_symbol(ses, pkt_with_fpi + sizeof(fpi_t)) != SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_decoder_decode_with_new_repair_symbol() failed\n");
				ret = -1;
				goto end;
			}
		}
		/* check if completed in case we received k packets or more */
		if ((n_received >= tot_src) && (0 == true) /* TODO: we need a termination test here! */) {
			/* done, we recovered everything, no need to continue reception */
			done = true;
			break;
		}
		len = SYMBOL_SIZE + sizeof(fpi_t);	/* make sure len contains the size of the expected packet */
	}
	if (done) {
		/* finally, get a copy of the pointers to all the source symbols, those received (that we already know) and those decoded.
		 * In case of received symbols, the library does not change the pointers (same value). */
		printf("\nDone! All source symbols rebuilt after receiving %u packets\n", n_received);
		if (VERBOSITY > 1) {
			for (esi = 0; esi < tot_src; esi++) {
				printf("src[%u]= ", esi);
				dump_buffer_32(src_symbols_tab[esi], 1);
			}
		}
	} else {
		printf("\nFailed to recover all erased source symbols even after receiving %u packets\n", n_received);
	}


end:
	/* Cleanup everything... */
	if (so!= INVALID_SOCKET) {
		close(so);
	}
	if (ses) {
		swif_decoder_release(ses);
	}
	if (fec_oti) {
		free(fec_oti);
	}
	if (recvd_symbols_tab && src_symbols_tab) {
		for (esi = 0; esi < tot_enc; esi++) {
			if (recvd_symbols_tab[esi]) {
				/* this is a symbol received from the network, without its FPI that starts 4 bytes before */
				free((char*)recvd_symbols_tab[esi] - 4);
			} //else if (esi < k && src_symbols_tab[esi]) {
			//	/* this is a source symbol decoded by the openfec codec, so free it */
			//	ASSERT(recvd_symbols_tab[esi] == NULL);
			//	free(src_symbols_tab[esi]);
			//}
		}
		free(recvd_symbols_tab);
		free(src_symbols_tab);
	}
	return ret;
}


/**
 * Opens and initializes a UDP socket, ready for receptions.
 */
static SOCKET
init_socket ()
{
	SOCKET		s;
	SOCKADDR_IN	bindAddr;
	uint32_t		sz = 1024 * 1024;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		fprintf(stderr, "Error: call to socket() failed\n");
		return INVALID_SOCKET;
	}
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons((short)DEST_PORT);
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (SOCKADDR*) &bindAddr, sizeof(bindAddr)) == SOCKET_ERROR) {
		fprintf(stderr, "bind() failed. Port %d may be already in use\n", DEST_PORT);
		return INVALID_SOCKET;
	}
	/* increase the reception socket size as the default value may lead to a high datagram loss rate */
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz)) == -1) {
		fprintf(stderr, "setsockopt() failed to set new UDP socket size to %u\n", sz);
		return INVALID_SOCKET;
	}
	return s;
}


/**
 * Receives packets on the incoming UDP socket.
 */
static swif_status_t
get_next_pkt   (SOCKET		so,
		void		**pkt,
		int32_t		*len)
{
	static bool	first_call = true;
	int32_t		saved_len = *len;	/* save it, in case we need to do several calls to recvfrom */

	if ((*pkt = malloc(saved_len)) == NULL) {
		fprintf(stderr, "Error, no memory (malloc failed for p)\n");
		return SWIF_STATUS_ERROR;
	}
	if (first_call) {
		/* the first time we must be in blocking mode since the flow may be launched after a few seconds... */
		first_call = false;
		*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
		if (*len < 0) {
			/* this is an anormal error, exit */
			perror("recvfrom");
			fprintf(stderr, "Error, recvfrom failed\n");
			free(*pkt);	/* don't forget to free it, otherwise it will leak */
			return SWIF_STATUS_ERROR;
		}
		/* set the non blocking mode for this socket now that the flow has been launched */
		if (fcntl(so, F_SETFL, O_NONBLOCK) < 0) {
			fprintf(stderr, "Error, fcntl failed to set non blocking mode\n");
			exit(-1);
		}
		if (VERBOSITY > 1)
			printf("%s: pkt received 0, len=%u\n", __FUNCTION__, *len);
		return SWIF_STATUS_OK;
	}
	/* otherwise we are in non-blocking mode... */
	*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
	if (*len > 0) {
		if (VERBOSITY > 1)
			printf("%s: pkt received 1, len=%u\n", __FUNCTION__, *len);
		return SWIF_STATUS_OK;
	} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
		/* no packet available, sleep a little bit and retry */
		SLEEP(200);	/* (in milliseconds) */
		*len = recvfrom(so, *pkt, saved_len, 0, NULL, NULL);
		if (*len > 0) {
			if (VERBOSITY > 1)
				printf("%s: pkt received 2, len=%u\n", __FUNCTION__, *len);
			return SWIF_STATUS_OK;
		} else {
			/* that's the end of the test, no packet available any more, we're sure of that now... */
			if (VERBOSITY > 1)
				printf("%s: end of test, no packet after the sleep\n", __FUNCTION__);
			free(*pkt);	/* don't forget to free it, otherwise it will leak */
			return SWIF_STATUS_FAILURE;
		}
	} else {
		/* this is an anormal error, exit */
		perror("recvfrom");
		fprintf(stderr, "Error, recvfrom failed\n");
		free(*pkt);	/* don't forget to free it, otherwise it will leak */
		return SWIF_STATUS_ERROR;
	}
	return SWIF_STATUS_ERROR;	/* never called */
}


/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void
dump_buffer_32 (void	*buf,
		uint32_t	len32)
{
	uint32_t	*ptr;
	uint32_t	j = 0;

	printf("0x");
	for (ptr = (uint32_t*)buf; len32 > 0; len32--, ptr++) {
		/* convert to big endian format to be sure of byte order */
		printf( "%08X", htonl(*ptr));
		if (++j == 10) {
			j = 0;
			printf("\n");
		}
	}
	printf("\n");
}

