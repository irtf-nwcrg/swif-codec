/*
 * Simple demo application on top of the SWIF-codec API.
 *
 * It is inspired from the same application from openFEC
 * (http://openfec.org/downloads.html) modified in order
 * to be used with the appropriate API.
 *
 * Author: Vincent Roca (Inria)
 */


Principles:
-----------

This directory contains a simple client-server application, showing how to use
a SWiF Codec in a simple case, with a single object, using a UDP socket and
rudimentary signaling.
Many parameters are set statically (e.g., the code rate) in order to simplify the
example as much as possible. The object size (in number of symbols) is the only
input argument.

    +--------------+                     +--------------+
    |    SERVER    |                     |    CLIENT    |
    +--------------+                     +--------------+
    |{set of source|                     |{set of source|
    |   symbols}   |                     |   symbols}   |
    |      |       |                     |      ^       |
    |      v       |                     |      |       |
    | FEC encoding |                     | FEC decoding |
    |      |       |                     |      ^       |
    |      v       |                     |      |       |
    | UDP sendto   |                     | UDP recvfrom |
    |      |       |                     |      ^       |
    +------|-------+                     +------|-------+
           |           UDP connection           |       
           +------------------------------------+
              signaling traffic + data traffic

In this simple example, a packet always contain a single source or repair symbol.
In practice an application will generate a flow of data units that have no reason
to be of fixed size, that of a symbol.

Signaling is rudimentary in this toy example:
	- FEC-Object Transmission Information (FEC-OTI) that enables encoder/
	  decoder synchronisation does not comply with any RFC;
	- FEC Payload ID (FPI) that carries information to identify each symbol
	  sent in a UDP packet does not comply with any RFC;


About packet erasures:
----------------------

We simulate packet erasures on the UDP connection. See simple_client_server.h
header file to control the loss rate (BTW, make sure it is compatible with the
code rate). By default:
#define CODE_RATE	0.667		/* k/n = 2/3 means we add 50% of repair symbols */
#define LOSS_RATE	0.30		/* we consider 30% of packet losses... It assumes
					   there's no additional loss during UDP transmissions */

However, since this is a UDP connection, since the server transmits at full
rate, the client may lose additional packets at the incoming UDP socket, even if
we already increased this socket size. With a real server, transmissions would
be rate-controlled with a leaky or token bucket, or any similar system.

So if you experience such errors as:
	Failed to recover all erased source symbols even after receiving XXX packets
then you can try to add a usleep(500) (for instance) in file simple_server.c in the
transmission loop in order to slow down transmissions. It should help! And as
explained above, the true solution is to implement rate control.


Usage: 
------

On the same machine (we are sending at 127.0.0.1 by default), use two terminals.
Start the receiver first:
$ ./simple_client

Then the server, using default parameters:
$ ./simple_server
or, if you want to specify the object size (in symbols):
$ ./simple_server 1000

The codec being used will be selected based on the object size (small => RS, medium or high => LDPC-Staircase).


References:
-----------

[RLC I-D] "Sliding Window Random Linear Code (RLC) Forward Erasure Correction (FEC) Schemes for FECFRAME", https://datatracker.ietf.org/doc/draft-ietf-tsvwg-rlc-fec-scheme/"
