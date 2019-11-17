/*
 * Simple demo application on top of the SWIF-codec API.
 *
 * It is inspired from the same application from openFEC
 * (http://openfec.org/downloads.html) modified in order
 * to be used with the appropriate API.
 *
 * Author: Vincent Roca (Inria)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>	/* for gettimeofday */

#include "../../src/swif_api.h"

/*
 * OS dependant definitions
 */
#define SOCKET		int
#define SOCKADDR	struct sockaddr
#define SOCKADDR_IN	struct sockaddr_in
#define INVALID_SOCKET	(-1)
#define SOCKET_ERROR	(-1)
#define closesocket	close
#define SLEEP(t)	usleep(t*1000)


/*
 * Simulation parameters...
 * Change as required
 */
#define SYMBOL_SIZE	16		/* symbol size, in bytes (must be multiple of 4 in this simple example) */
#define	DEFAULT_EW_SIZE	10		/* default encoding window size assumed constant */
#define CODE_RATE	0.667		/* k/n = 2/3 means we add 50% of repair symbols */
#define LOSS_RATE	0.30		/* we consider 30% of packet losses... It assumes there's no additional loss during UDP transmissions */
#define VERBOSITY	1		/* Define the verbosity level:
					 *	0 : no trace
					 *	1 : main traces
					 *	2 : full traces with packet dumps */
#define DEST_IP		"127.0.0.1"	/* Destination IPv4 address */
#define DEST_PORT	10978		/* Destination port (UDP) */


/*
 * Source symbol status at a decoder (in our case the simple_client receiver), namely:
 * - is it a source symbol that is still missing, neither received nor decoded (default status)?
 * - is it a source symbol associated to a received FEC source packet?
 * - is it a decoded source symbol?
 */
typedef enum {
	SRC_SYMBOL_STATUS_MISSING = 0,
	SRC_SYMBOL_STATUS_RECEIVED,
	SRC_SYMBOL_STATUS_DECODED
} src_symbol_status_at_recv_t;


/*
 * Simplified FEC Object Transmission Information structure, used to synchronize sender and receiver.
 * NB: all the fields MUST be in Network Endianess while sent over the network, so use htonl (resp. ntohl) at the sender (resp. receiver).
 */
typedef struct {
	uint32_t	codepoint;
	uint32_t	ew_size;
	uint32_t	tot_src;	/* total number of source symbols */
	uint32_t	tot_enc;	/* total number of encoding symbols */
} fec_oti_t;


/*
 * Simplified, non optimized FEC Payload Information, valid both for source and repair packets.
 * NB: all the fields MUST be in Network Endianess while sent over the network, so use htonl (resp. ntohl) at the sender (resp. receiver).
 */
typedef struct {
	uint16_t	is_source;	/* 1 if source, 0 if repair */
	uint16_t	repair_key;	/* only meaningful in case of a repair */
	uint16_t	nss;		/* only meaningful in case of a repair */
	esi_t		esi;		/* esi of a source symbol, or esi of the first source symbol of the encoding window in case of a repair */
} fpi_t;

