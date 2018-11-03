/**
 * SWiF Codec: an open-source sliding window FEC codec in C
 * https://github.com/irtf-nwcrg/swif-codec
 */

/*
 * Coding coefficients generator coming from RLC FEC Scheme, version 10, I-D.
 * https://datatracker.ietf.org/doc/draft-ietf-tsvwg-rlc-fec-scheme/
 */


/*
 * Fills in the table of coding coefficients (of the right size)
 * provided with the appropriate number of coding coefficients to
 * use for the repair symbol key provided.
 *
 * (in) repair_key    key associated to this repair symbol. This
 *                    parameter is ignored (useless) if m=2 and dt=15
 * (in/out) cc_tab[]  pointer to a table of the right size to store
 *                    coding coefficients. All coefficients are
 *                    stored as bytes, regardless of the m parameter,
 *                    upon return of this function.
 * (in) cc_nb         number of entries in the table. This value is
 *                    equal to the current encoding window size.
 * (in) dt            integer between 0 and 15 (inclusive) that
 *                    controls the density. With value 15, all
 *                    coefficients are guaranteed to be non zero
 *                    (i.e. equal to 1 with GF(2) and equal to a
 *                    value in {1,... 255} with GF(2^^8)), otherwise
 *                    a fraction of them will be 0.
 * (in) m             Finite Field GF(2^^m) parameter. In this
 *                    document only values 1 and 8 are considered.
 * (out)              returns an error code
 */
int generate_coding_coefficients (uint16_t  repair_key,
                                 uint8_t   cc_tab[],
                                 uint16_t  cc_nb,
                                 uint8_t   dt,
                                 uint8_t   m)
{
   uint32_t	i;
   tinymt32_t	s;    /* PRNG internal state */

   if (dt > 15) {
       return SWIF_STATUS_ERROR; /* bad dt parameter */
   }
   switch (m) {
   case 1:
       if (dt == 15) {
           /* all coefficients are 1 */
           memset(cc_tab, 1, cc_nb);
       } else {
           /* here coefficients are either 0 or 1 */
           tinymt32_init(&s, repair_key);
           for (i = 0 ; i < cc_nb ; i++) {
               if (tinymt32_rand(&s, 16) <= dt) {
                   cc_tab[i] = (uint8_t) 1;
               } else {
                   cc_tab[i] = (uint8_t) 0;
               }
           }
       }
       break;

   case 8:
       tinymt32_init(&s, repair_key);
       if (dt == 15) {
           /* coefficient 0 is avoided here in order to include
            * all the source symbols */
           for (i = 0 ; i < cc_nb ; i++) {
               do {
                   cc_tab[i] = (uint8_t) tinymt32_rand(&s, 256);
               } while (cc_tab[i] == 0);
           }
       } else {
           /* here a certain fraction of coefficients should be 0 */
           for (i = 0 ; i < cc_nb ; i++) {
               if (tinymt32_rand(&s, 16) <= dt) {
                   do {
                       cc_tab[i] = (uint8_t) tinymt32_rand(&s, 256);
                   } while (cc_tab[i] == 0);
               } else {
                   cc_tab[i] = 0;
               }
           }
       }
       break;

   default:
       /* bad parameter m */
       return SWIF_STATUS_ERROR;
   }
   return SWIF_STATUS_OK;
}

