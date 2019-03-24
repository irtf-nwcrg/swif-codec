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
static SOCKET	init_socket (SOCKADDR_IN	*dst_host);

/**
 * Dumps len32 32-bit words of a buffer (typically a symbol).
 */
static void	dump_buffer_32 (void	*buf,
				uint32_t	len32);

/**
 * Callback (not really required).
 */
static void	source_symbol_removed_from_coding_window_callback (void*   context,
								   esi_t   old_symbol_esi);


/*************************************************************************************************/


int
main(int argc, char* argv[])
{
	swif_codepoint_t codepoint;				/* identifier of the codec to use */
	swif_encoder_t	*ses		= NULL;
	void**		enc_symbols_tab	= NULL;			/* table containing pointers to the encoding (i.e. source + repair) symbols buffers */
	uint32_t	ew_size;				/* encoding window size */
	uint32_t	tot_src;				/* total number of source symbols */
	uint32_t	tot_enc;				/* total number of encoding symbols (i.e. source + repair) in the session */
	esi_t		esi;					/* source symbol id */
	uint32_t	idx;					/* index in the source+repair table */
	uint32_t	interval_between_repairs;		/* number of source symbols between two repair symbols, in line with the code rate */
	SOCKET		so		= INVALID_SOCKET;	/* UDP socket for server => client communications */
	char		*pkt_with_fpi	= NULL;			/* buffer containing a fixed size packet plus a header consisting only of the FPI */
	fec_oti_t	fec_oti;				/* FEC Object Transmission Information as sent to the client */
	fpi_t		*fpi;					/* header (FEC Payload Information) for source and repair symbols */
	SOCKADDR_IN	dst_host;
	uint32_t	ret		= -1;

	if (argc == 1) {
		/* ew_size value is ommited, so use default */
		ew_size = DEFAULT_EW_SIZE;
	} else {
		ew_size = atoi(argv[1]);
	}
	if (ew_size < 2) {
		fprintf(stderr, "Error: invalid encoding window size (%ul). Cannot be < 2.\n", ew_size);
		ret = -1;
		goto end;
	}
	tot_src = 1000;
	tot_enc = (uint32_t)floor((double)tot_src / (double)CODE_RATE);
	if (tot_enc < tot_src) {
		fprintf(stderr, "Error initializing tot_enc (%u). Cannot be < tot_src (%u)!\n", tot_enc, tot_src);
		ret = -1;
		goto end;
	}
	codepoint = SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC;

	/* first initialize the UDP socket... */
	if ((so = init_socket(&dst_host)) == INVALID_SOCKET) {
		fprintf(stderr, "Error initializing socket!\n");
		ret = -1;
		goto end;
	}
	printf("First of all, send the FEC OTI to %s/%d\n", DEST_IP, DEST_PORT);
	/* initialize and send the FEC OTI to the client */
	fec_oti.codepoint	= htonl(codepoint);
	fec_oti.ew_size		= htonl(ew_size);
	fec_oti.tot_src		= htonl(tot_src);
	fec_oti.tot_enc		= htonl(tot_enc);
	if ((ret = sendto(so, (void*)&fec_oti, sizeof(fec_oti), 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) != sizeof(fec_oti)) {
		fprintf(stderr, "Error while sending the FEC OTI\n");
		ret = -1;
		goto end;
	}
	/* allocate a buffer where we'll copy each symbol plus its simplified FPI.
	 * This buffer will be reused during the whose session */
	if ((pkt_with_fpi = malloc(sizeof(fpi_t) + SYMBOL_SIZE)) == NULL) {
		fprintf(stderr, "no memory (malloc failed for pkt_with_fpi)\n");
		ret = -1;
		goto end;
	}
	/* continue with the SWIF codec */
	printf("\nInitialize a SWIF encoder instance: tot_src=%ul src symbols, ew_size=%ul, total %ul encoding symbols\n", tot_src, ew_size, tot_enc);
	if ((ses = swif_encoder_create(codepoint, VERBOSITY, SYMBOL_SIZE, ew_size)) == NULL) {
		fprintf(stderr, "Error, swif_encoder_create() failed\n");
		ret = -1;
		goto end;
	}
	if (swif_encoder_set_callback_functions(ses, source_symbol_removed_from_coding_window_callback, NULL) != SWIF_STATUS_OK) {
		fprintf(stderr, "Error, swif_encoder_set_callback_functions() failed\n");
		ret = -1;
		goto end;
	}
	/* allocate the table with pointers to source and repair symbols... */
	if ((enc_symbols_tab = (void**) calloc(tot_enc, sizeof(void*))) == NULL) {
		fprintf(stderr, "Error, no memory (calloc failed for enc_symbols_tab, tot_enc=%ul)\n", tot_enc);
		ret = -1;
		goto end;
	}
	/*
	 * main loop, where the application goes through all source symbols and submits them one by one to the codec, asking for
	 * a repair symbol from time to time.
	 */
	interval_between_repairs = tot_src / (tot_enc - tot_src);
	idx = 0;
	for (esi = 0; esi < tot_src; esi++) {
		if ((enc_symbols_tab[idx] = malloc(SYMBOL_SIZE)) == NULL) {
			fprintf(stderr, "Error, no memory (calloc failed for enc_symbols_tab[%ul]/esi=%ul)\n", idx, esi);
			ret = -1;
			goto end;
		}
		/* in order to detect corruption, the first source symbol is filled with 0x1111..., the second with 0x2222..., etc.
		 * NB: the 0x0 value is avoided since it is a neutral element in the target finite fields, i.e. it prevents the detection
		 * of symbol corruption */
		memset(enc_symbols_tab[idx], (char)(esi + 1), SYMBOL_SIZE);
		if (VERBOSITY > 1) {
			printf("src[%03d]= ", esi);
			dump_buffer_32(enc_symbols_tab[idx], 1);
		}
		/* add it to the encoding window (no need to do anything else for a source symbol) */
		if (swif_encoder_add_source_symbol_to_coding_window (ses, enc_symbols_tab[idx], esi) != SWIF_STATUS_OK) {
			fprintf(stderr, "Error, swif_encoder_add_source_symbol_to_coding_window failed for esi=%ul)\n", esi);
			ret = -1;
			goto end;
		}
		/* prepend a header in network byte order */
		fpi = (fpi_t*)pkt_with_fpi;
		fpi->is_source = htons(1);
		fpi->repair_key = htons(0);		/* only meaningful in case of a repair */
		fpi->nss = htons(0);			/* only meaningful in case of a repair */
		fpi->esi = htonl(esi);
		memcpy(pkt_with_fpi + sizeof(fpi_t), enc_symbols_tab[idx], SYMBOL_SIZE);
		printf(" => sending src symbol %u\n", esi);
		if ((ret = sendto(so, pkt_with_fpi, sizeof(fpi_t) + SYMBOL_SIZE, 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) == SOCKET_ERROR) {
			fprintf(stderr, "Error, sendto() failed!\n");
			ret = -1;
			goto end;
		}
		/* perform a short usleep() to slow down transmissions and avoid UDP socket saturation at the receiver.
		 * Note that the true solution consists in adding some rate control mechanism here... */
		usleep(500);
		idx++;
		if ((esi > 0 && (esi % interval_between_repairs) == 0) || esi == tot_src-1) {
			esi_t		first;
			esi_t		last;
			uint32_t	nss;

			/* it's time to produce repair packets. They are regularly spaced and we add a last one at the end of session */
			if ((enc_symbols_tab[idx] = calloc(SYMBOL_SIZE, 1)) == NULL) {
				fprintf(stderr, "Error, no memory (calloc failed for enc_symbols_tab[%d])\n", esi);
				ret = -1;
				goto end;
			}
			/* the index is the repair_key */
			if (swif_encoder_generate_coding_coefs(ses, idx, 0) != SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_decoder_generate_coding_coefs() failed for repair_key=%ul\n", idx);
				ret = -1;
				goto end;
			}
			if (swif_build_repair_symbol(ses, enc_symbols_tab[idx]) != SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_build_repair_symbol() failed for repair_key=%ul\n", idx);
				ret = -1;
				goto end;
			}
			if (VERBOSITY > 1) {
				printf("repair[%03d]= ", esi);
				dump_buffer_32(enc_symbols_tab[idx], 4);
			}
			/* prepend a header in network byte order */
			if (swif_encoder_get_coding_window_information(ses, &first, &last, &nss) == SWIF_STATUS_OK) {
				fprintf(stderr, "Error, swif_encoder_get_coding_window_information() failed for repair_key=%ul\n", idx);
				ret = -1;
				goto end;
			}
			/* in our simple case, there is no ESI loop back to zero, so check consistency */
			if (nss != last - first + 1) {
				fprintf(stderr, "Error, nss (%ul) != last (%ul) - first (%ul) + 1, it should be equal\n", nss, last, first);
				ret = -1;
				goto end;
			}
			fpi = (fpi_t*)pkt_with_fpi;
			fpi->is_source = htons(0);
			fpi->repair_key = htons(idx);
			fpi->nss = htons(nss);
			fpi->esi = htonl(first);
			memcpy(pkt_with_fpi + sizeof(fpi_t), enc_symbols_tab[idx], SYMBOL_SIZE);
			printf(" => sending src symbol %ul\n", esi);
			if ((ret = sendto(so, pkt_with_fpi, sizeof(fpi_t) + SYMBOL_SIZE, 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) == SOCKET_ERROR) {
				fprintf(stderr, "Error, sendto() failed!\n");
				ret = -1;
				goto end;
			}
			/* Perform a short usleep() to slow down transmissions and avoid UDP socket saturation at the receiver.
			 * Note that the true solution consists in adding some rate control mechanism here, like a leaky or token bucket. */
			usleep(500);
			idx++;
		}
	}
	printf( "\nCompleted, %d packets sent successfully.\n", idx);
	ret = 1;

end:
	/* Cleanup everything... */
	if (so!= INVALID_SOCKET) {
		close(so);
	}
	if (ses) {
		swif_encoder_release(ses);
	}
	if (enc_symbols_tab) {
		for (esi = 0; esi < tot_enc; esi++) {
			if (enc_symbols_tab[esi]) {
				free(enc_symbols_tab[esi]);
			}
		}
		free(enc_symbols_tab);
	}
	if (pkt_with_fpi) {
		free(pkt_with_fpi);
	}
	return ret;
}


/* Initialize our UDP socket */
static SOCKET
init_socket (SOCKADDR_IN	*dst_host)
{
	SOCKET s;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Error, call to socket() failed\n");
		return INVALID_SOCKET;
	}
	dst_host->sin_family = AF_INET;
	dst_host->sin_port = htons((short)DEST_PORT);
	dst_host->sin_addr.s_addr = inet_addr(DEST_IP);
	return s;
}


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


static void 
source_symbol_removed_from_coding_window_callback (void*   context,
						   esi_t   old_symbol_esi)
{
	printf("callback: symbol %ul removed\n", old_symbol_esi);
}
