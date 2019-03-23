/**
 * SWiF Codec: an open-source sliding window FEC codec in C
 * https://github.com/irtf-nwcrg/swif-codec
 */


/**
 * Control block for an RLC encoder.
 * Encoder structure that contains whatever is needed for encoding.
 * The first field of this structure is the generic swif encoder
 * so that one can cast a pointer to one of the two structures,
 * depending on the context.
 */
typedef struct swif_encoder_rlc_cb {
	swif_encoder_t generic_encoder;

	/* desired verbosity: 0 is the minimum verbosity, the maximum
	 * level being implementation specific. */
	uint32_t		verbosity;

	/* maximum number of source symbols used for any repair symbol */
	uint32_t		max_coding_window_size;

	/* exact size (in bytes) of any source or repair symbol */
	uint32_t		symbol_size;

	/* coding coefficients table. To be initialized before building a new repair symbol */
	uint8_t			*cc_tab;
	
	/* pointer to the table containing source symbols */
	void			**ew_tab;

	/* the index of the firt source symbol (included) */
	uint32_t 			ew_left;

	/* the index of the last source symbol added (included) */
	uint32_t 			ew_right;

	/** identifier of the last source symbol added
	* we suppose that our source symbols are ordered
	 */
	esi_t 			ew_esi_right;

	/** number of source currently in the encoding window 
	* useful to differentiate the state of the buffer
	*/
	uint32_t 		ew_ss_nb;

	void (*source_symbol_removed_from_coding_window_callback) (
                                        void*   context,
                                        esi_t   old_symbol_esi);
	
	void 			*context_4_callback;
	/* add whatever may be needed hereafter... */
} swif_encoder_rlc_cb_t;


/**
 * Control block for an RLC decoder.
 * Decoder structure that contains whatever is needed for decoding.
 * The first field of this structure is the generic swif decoder
 * so that one can cast a pointer to one of the two structures,
 * depending on the context.
 */
typedef struct swif_decoder_rlc_cb {
	swif_decoder_t generic_decoder;

	/* desired verbosity: 0 is the minimum verbosity, the maximum
	 * level being implementation specific. */
	uint32_t		verbosity;

	/* maximum number of source symbols used for any repair symbol */
	uint32_t		max_coding_window_size;

	/* max. number of source symbols kepts in current linear system.
	 * If the linear system grows above this limit, old source
	 * symbols in excess are removed and the application callback
	 * called. This value should be larger than the
	 * max_coding_window_size. */
	uint32_t		max_linear_system_size;

	/* exact size (in bytes) of any source or repair symbol */
	uint32_t		symbol_size;

	/* add whatever may be needed hereafter... */
} swif_decoder_rlc_cb_t;
