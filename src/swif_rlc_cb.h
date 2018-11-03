/**
 * SWiF Codec: an open-source sliding window FEC codec in C
 * https://github.com/irtf-nwcrg/swif-codec
 */


/**
 * Control block for an RLC encoder.
 * Encoder structure that contains whatever is needed for encoding.
 * This structure MUST be aligned with swif_encoder_t (same first items) in
 * order to be able to cast a pointer to one of the two structures,
 * depending on the context.
 */
typedef struct swif_encoder_rlc_cb {
        /* when a function returns with SWIF_STATUS_ERROR, the errno
         * variable contains a more detailed error type. */
        swif_errno_t    errno;

        /* desired verbosity: 0 is the minimum verbosity, the maximum
         * level being implementation specific. */
        uint32_t        verbosity;

        /* maximum number of source symbols used for any repair symbol */
        uint32_t        max_coding_window_size;

        /* exact size (in bytes) of any source or repair symbol */
        uint32_t        symbol_size;

        /* add whatever may be needed hereafter... */
} swif_encoder_rlc_cb_t;


/**
 * Control block for an RLC decoder.
 * Decoder structure that contains whatever is needed for decoding.
 * This structure MUST be aligned with swif_decoder_t (same first items) in
 * order to be able to cast a pointer to one of the two structures,
 * depending on the context.
 */
typedef struct swif_decoder_rlc_cb {
        /* when a function returns with SWIF_STATUS_ERROR, the errno
         * variable contains a more detailed error type. */
        swif_errno_t    errno;

        /* desired verbosity: 0 is the minimum verbosity, the maximum
         * level being implementation specific. */
        uint32_t        verbosity;

        /* maximum number of source symbols used for any repair symbol */
        uint32_t        max_coding_window_size;

        /* max. number of source symbols kepts in current linear system.
         * If the linear system grows above this limit, old source
         * symbols in excess are removed and the application callback
         * called. This value should be larger than the
         * max_coding_window_size. */
        uint32_t        max_linear_system_size;

        /* exact size (in bytes) of any source or repair symbol */
        uint32_t        symbol_size;

        /* add whatever may be needed hereafter... */
} swif_decoder_rlc_cb_t;