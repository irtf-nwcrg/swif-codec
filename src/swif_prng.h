/**
 * SWiF Codec: an open-source sliding window FEC codec in C
 * https://github.com/irtf-nwcrg/swif-codec
 */


/**
 * This function initializes the internal state array with a 32-bit
 * unsigned integer seed.
 * @param s pointer to tinymt internal state.
 * @param seed  a 32-bit unsigned integer used as a seed.
 */
void tinymt32_init     (tinymt32_t *	s,
			uint32_t	seed);


/**
 * This function outputs an integer in the [0 .. maxv-1] range.
 * theJPster answer in:
 * https://stackoverflow.com/questions/2509679/
 *      how-to-generate-a-random-integer-number-from-within-a-range
 *
 * @param s pointer to tinymt internal state.
 * @return  32-bit unsigned integer between 0 and maxv-1 inclusive.
 */
uint32_t tinymt32_rand (tinymt32_t *	s,
			uint32_t	maxv);
