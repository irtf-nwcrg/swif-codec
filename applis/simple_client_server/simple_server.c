/*
 * Simple demo application on top of the SWiF Codec API.
 *
 * It is inspired from the same application from openFEC
 * (http://openfec.org/downloads.html) modified in order
 * to be used with the appropriate API.
 */

/* $Id: simple_server.c 216 2014-12-13 13:21:07Z roca $ */
/*
 * OpenFEC.org AL-FEC Library.
 * (c) Copyright 2009-2014 INRIA - All rights reserved
 * Contact: vincent.roca@inria.fr
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software.  You can  use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and,  more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
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
	uint32_t	symb_sz_32	= SYMBOL_SIZE / 4;	/* symbol size in units of 32 bit words */
	uint32_t	ew_size;				/* encoding window size */
	uint32_t	k;					/* total number of source symbols */
	uint32_t	n;					/* total number of encoding symbols (i.e. source + repair) in the session */
	esi_t		esi;					/* source symbol id */
	uint32_t	i;
	uint32_t	idx;					/* index in the source+repair table */
	SOCKET		so		= INVALID_SOCKET;	/* UDP socket for server => client communications */
	char		*pkt_with_fpi	= NULL;			/* buffer containing a fixed size packet plus a header consisting only of the FPI */
	fec_oti_t	fec_oti;				/* FEC Object Transmission Information as sent to the client */
	INT32		lost_after_index= -1;			/* all the packets to send after this index are considered as lost during transmission */
	SOCKADDR_IN	dst_host;
	uint32_t	ret		= -1;

	if (argc == 1) {
		/* ew_size value is ommited, so use default */
		ew_size = DEFAULT_EW_SIZE;
	} else {
		ew_size = atoi(argv[1]);
	}
	k = 1000;
	n = (uint32_t)floor((double)ew_size / (double)CODE_RATE);

	printf("\nInitialize a SWiF Codec instance, (n, k)=(%u, %u)...\n", n, k);
	codepoint = SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC;
	params->encoding_symbol_length	= SYMBOL_SIZE;

	/* Open and initialize the openfec session now... */
	if ((ses = swif_encoder_create(codepoint, codec_id, SYMBOL_SIZE, ew_size)) != SWIF_STATUS_OK) {
		printf(stderr, "swif_encoder_create() failed\n");
		ret = -1;
		goto end;
	}
	if (swif_encoder_set_callback_functions(ses, source_symbol_removed_from_coding_window_callback(), NULL) != SWIF_STATUS_OK) {
		printf(stderr, "swif_encoder_set_callback_functions() failed\n");
		ret = -1;
		goto end;
	}

	/* Allocate and initialize our source symbols...
	 * In case of a file transmission, the opposite takes place: the file is read and partitionned into a set of k source symbols.
	 * At the end, it's just equivalent since there is a set of k source symbols that need to be sent reliably thanks to an FEC
	 * encoding. */
	printf("\nFilling source symbols...\n");
	if ((enc_symbols_tab = (void**) calloc(n, sizeof(void*))) == NULL) {
		printf(stderr, "no memory (calloc failed for enc_symbols_tab, n=%u)\n", n);
		ret = -1;
		goto end;
	}
	/* In order to detect corruption, the first symbol is filled with 0x1111..., the second with 0x2222..., etc.
	 * NB: the 0x0 value is avoided since it is a neutral element in the target finite fields, i.e. it prevents the detection
	 * of symbol corruption */
	uint32_t	int_between_repairs;
	interval_between_repairs = k / (n - k);
	idx = 0;
	key = 0;
	for (esi = 0; esi < k; esi++) {
		if ((enc_symbols_tab[idx] = calloc(symb_sz_32, sizeof(uint32_t))) == NULL) {
			printf(stderr, "no memory (calloc failed for enc_symbols_tab[%u]/esi=%u)\n", idx, esi);
			ret = -1;
			goto end;
		}
		memset(enc_symbols_tab[idx], (char)(esi + 1), SYMBOL_SIZE);
		if (VERBOSITY > 1) {
			printf("src[%03d]= ", esi);
			dump_buffer_32(enc_symbols_tab[idx], 1);
		}
		idx++;
		if ((esi %  interval_between_repairs) == 0 || esi == k-1) {
			/* it's time to produce repair packets, they are regularly spaced plus a last one at the end of session */
			if ((enc_symbols_tab[idx] = calloc(symb_sz_32, sizeof(uint32_t))) == NULL) {
				printf(stderr, "no memory (calloc failed for enc_symbols_tab[%d])\n", esi);
				ret = -1;
				goto end;
			}
			if (swif_decoder_generate_coding_coefs(ses, key++, 0) != SWIF_STATUS_OK) {
				printf(stderr, "ERROR:  swif_decoder_generate_coding_coefs() failed after esi=%u\n", esi);
				ret = -1;
				goto end;
			}
			if (swif_build_repair_symbol(ses, enc_symbols_tab[idx]) != SWIF_STATUS_OK) {
				printf(stderr, "ERROR:  swif_build_repair_symbol() failed after esi=%u\n", esi);
				ret = -1;
				goto end;
			}
			if (VERBOSITY > 1) {
				printf("repair[%03d]= ", esi);
				dump_buffer_32(enc_symbols_tab[idx], 4);
			}
			idx++;
		}
	}

	/* Finally initialize the UDP socket and throw our packets... */
	if ((so = init_socket(&dst_host)) == INVALID_SOCKET) {
		printf(stderr, "Error initializing socket!\n");
		ret = -1;
		goto end;
	}
	printf("First of all, send the FEC OTI for this object to %s/%d\n", DEST_IP, DEST_PORT);
	/* Initialize and send the FEC OTI to the client */
	/* convert back to host endianess */
	fec_oti.codepoint	= htonl(codepoint);
	fec_oti.ew_size		= htonl(ew_size);
	fec_oti.k		= htonl(k);
	fec_oti.n		= htonl(n);
	if ((ret = sendto(so, (void*)&fec_oti, sizeof(fec_oti), 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) != sizeof(fec_oti)) {
		printf(stderr, "Error while sending the FEC OTI\n");
		ret = -1;
		goto end;
	}

	/* Allocate a buffer where we'll copy each symbol plus its simplistif FPI (in this example consisting only of the ESI).
	 * This needs to be fixed in real applications, with the actual FPI required for this code. Also doing a memcpy is
	 * rather suboptimal in terms of performance! */
	if ((pkt_with_fpi = malloc(4 + SYMBOL_SIZE)) == NULL) {
		printf(stderr, "no memory (malloc failed for pkt_with_fpi)\n");
		ret = -1;
		goto end;
	}
	for (i = 0; i < n; i++) {
		if (i == lost_after_index) {
			/* the remaining packets are considered as lost, exit loop */
			break;
		}
		/* Add a pkt header wich only countains the ESI, i.e. a 32bits sequence number, in network byte order in order
		 * to be portable regardless of the local and remote byte endian representation (the receiver will do the
		 * opposite with ntohl()...) */
		*(uint32_t*)pkt_with_fpi = htonl(i);
		memcpy(4 + pkt_with_fpi, enc_symbols_tab[i], SYMBOL_SIZE);
		printf("%05d => sending symbol %u (%s)\n", i + 1, i, (i < k) ? "src" : "repair");
		if ((ret = sendto(so, pkt_with_fpi, SYMBOL_SIZE + 4, 0, (SOCKADDR *)&dst_host, sizeof(dst_host))) == SOCKET_ERROR) {
			printf(stderr, "sendto() failed!\n");
			ret = -1;
			goto end;
		}
		/* Perform a short usleep() to slow down transmissions and avoid UDP socket saturation at the receiver.
		 * Note that the true solution consists in adding some rate control mechanism here, like a leaky or token bucket. */
		usleep(500);
	}
	printf( "\nCompleted! %d packets sent successfully.\n", i);
	ret = 1;

end:
	/* Cleanup everything... */
	if (so!= INVALID_SOCKET) {
		close(so);
	}
	if (ses) {
		of_release_codec_instance(ses);
	}
	if (params) {
		free(params);
	}
	if (enc_symbols_tab) {
		for (esi = 0; esi < n; esi++) {
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
		printf("Error: call to socket() failed\n");
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
		if (++j == 10)
		{
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
	printf("callback: symbol %u removed\n", old_symbol_esi);
}
