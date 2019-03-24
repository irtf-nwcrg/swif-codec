#include "swif_includes.h"


/**
 * Release an encoder and its associated ressources.
 **/
swif_status_t   swif_rlc_encoder_release (swif_encoder_t*        enc)
{
	assert(enc);
	free(enc);
	return SWIF_STATUS_OK;
}


/**
 * Set the various callback functions for this encoder.
 * All the callback functions require an opaque context parameter, that must be
 * initialized accordingly by the application, since it is application specific.
 */
swif_status_t   swif_rlc_encoder_set_callback_functions (
                swif_encoder_t*        enc,
                void (*source_symbol_removed_from_coding_window_callback) (
                                        void*   context,
                                        esi_t   old_symbol_esi),
                void* context_4_callback)
{
    swif_encoder_rlc_cb_t *rlc_enc = (swif_encoder_rlc_cb_t *) enc;

    assert(enc);
    rlc_enc->source_symbol_removed_from_coding_window_callback =
				source_symbol_removed_from_coding_window_callback;
    rlc_enc->context_4_callback = context_4_callback;
	return SWIF_STATUS_OK;
}


/**
 * This function sets one or more FEC codec specific parameters,
 * using a type/length/value approach for maximum flexibility.
 */
swif_status_t   swif_rlc_encoder_set_parameters  (
                                swif_encoder_t* enc,
                                uint32_t        type,
                                uint32_t        length,
                                void*           value)
{
// NOT YET
	return SWIF_STATUS_OK;
}

/**
 * This function gets one or more FEC codec specific parameters,
 * using a type/length/value approach for maximum flexibility.
 */
swif_status_t   swif_rlc_encoder_get_parameters  (
                                swif_encoder_t* enc,
                                uint32_t        type,
                                uint32_t        length,
                                void*           value)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * Create a single repair symbol (i.e. perform an encoding).
 */
swif_status_t   swif_rlc_build_repair_symbol (
                                swif_encoder_t* generic_encoder,
                                void*           new_buf)
{
    swif_encoder_rlc_cb_t* enc = (swif_encoder_rlc_cb_t*) generic_encoder;
    uint32_t	i;

    if ((new_buf = calloc(1, sizeof(enc->symbol_size))) == NULL) {
        fprintf(stderr, "swif_rlc_build_repair_symbol failed! No memory\n");
        return SWIF_STATUS_ERROR;
    }
    for (i = enc->ew_left; i < enc->ew_ss_nb; i++) {
        symbol_add_scaled(new_buf, enc->cc_tab[i % enc->max_coding_window_size], enc->ew_tab[i % enc->max_coding_window_size], enc->symbol_size);
    }
    return SWIF_STATUS_OK;
}


/*******************************************************************************
 * Decoder functions
 */


/**
 * Create and initialize a decoder, providing only key parameters.
 */
swif_decoder_t* swif_rlc_decoder_create (
                                swif_codepoint_t codepoint,
                                uint32_t        verbosity,
                                uint32_t        symbol_size,
                                uint32_t        max_coding_window_size,
                                uint32_t        max_linear_system_size)
{
// NOT YET
	return NULL;
}


/**
 * Release a decoder and its associated ressources.
 **/
swif_status_t   swif_rlc_decoder_release (swif_decoder_t*        dec)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * Set the various callback functions for this decoder.
 * All the callback functions require an opaque context parameter, that
 * must be initialized accordingly by the application, since it is
 * application specific.
 */
swif_status_t   swif_rlc_decoder_set_callback_functions (
                swif_decoder_t*  dec,
                void (*source_symbol_removed_from_linear_system_callback) (
                                        void*   context,
                                        esi_t   old_symbol_esi),
                void* (*decodable_source_symbol_callback) (
                                        void    *context,
                                        esi_t   esi),
                void* (*decoded_source_symbol_callback) (
                                        void    *context,
                                        void    *new_symbol_buf,
                                        esi_t   esi),
                void*        context_4_callback)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * This function sets one or more FEC codec specific parameters,
 *        using a type/length/value approach for maximum flexibility.
 */
swif_status_t   swif_rlc_decoder_set_parameters  (
                                swif_decoder_t* dec,
                                uint32_t        type,
                                uint32_t        length,
                                void*           value)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * This function gets one or more FEC codec specific parameters,
 * using a type/length/value approach for maximum flexibility.
 */
swif_status_t   swif_rlc_decoder_get_parameters  (
                                swif_decoder_t* dec,
                                uint32_t        type,
                                uint32_t        length,
                                void*           value)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * Submit a received source symbol and try to progress in the decoding.
 * For each decoded source symbol (if any), the application is informed
 * through the dedicated callback functions.
 */
swif_status_t   swif_rlc_decoder_decode_with_new_source_symbol (
                                swif_decoder_t* dec,
                                void* const     new_symbol_buf,
                                esi_t           new_symbol_esi)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * Submit a received repair symbol and try to progress in the decoding.
 * For each decoded source symbol (if any), the application is informed
 * through the dedicated callback functions.
 */
swif_status_t   swif_rlc_decoder_decode_with_new_repair_symbol (
                                swif_decoder_t* dec,
                                void* const     new_symbol_buf)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/*******************************************************************************
 * Coding Window Functions at an Encoder and Decoder
 */


/**
 * This function resets the current coding window. We assume here that
 * this window is maintained by the FEC codec instance.
 * Encoder:     reset the encoding window for the encoding of future
 *              repair symbols.
 * Decoder:     reset the coding window under preparation associated to
 *              a repair symbol just received.
 */
swif_status_t   swif_rlc_encoder_reset_coding_window (swif_encoder_t*  enc)
{
// NOT YET
	return SWIF_STATUS_OK;
}

swif_status_t   swif_rlc_decoder_reset_coding_window (swif_encoder_t*  dec)
{
// NOT YET
	return SWIF_STATUS_OK;
}

/**
 * Add this source symbol to the coding window.
 * Encoder:     add a source symbol to the coding window.
 * Decoder:     add a source symbol to the coding window under preparation.
 */
swif_status_t   swif_rlc_encoder_add_source_symbol_to_coding_window (
                                swif_encoder_t* generic_enc,
                                void*           new_src_symbol_buf,
                                esi_t           new_src_symbol_esi)
{
    swif_encoder_rlc_cb_t	*enc = (swif_encoder_rlc_cb_t *) generic_enc;

    if ((enc->ew_esi_right != INVALID_ESI) && (new_src_symbol_esi != enc->ew_esi_right + 1)) {
        fprintf(stderr, "swif_rlc_encoder_add_source_symbol_to_coding_window() failed! new_src_symbol_esi (%u) is not in the right order (%u expected)\n",
		new_src_symbol_esi, enc->ew_esi_right + 1);
        return SWIF_STATUS_ERROR;  
    }
    if (enc->ew_ss_nb == enc->max_coding_window_size) {
        if (enc->source_symbol_removed_from_coding_window_callback != NULL) {
            enc->source_symbol_removed_from_coding_window_callback(enc->context_4_callback,
                                                                    enc->ew_esi_right  - enc->ew_ss_nb);
        }
        enc->ew_tab[enc->ew_left] = new_src_symbol_buf;
        enc->ew_right = enc->ew_left;
        enc->ew_left = (enc->ew_left +1) % enc->max_coding_window_size;
        enc->ew_esi_right = new_src_symbol_esi;
    }   else {
        enc->ew_right = (enc->ew_right +1) % enc->max_coding_window_size;
        enc->ew_ss_nb++;
        enc->ew_tab[enc->ew_right] = new_src_symbol_buf;
        enc->ew_esi_right = new_src_symbol_esi;
    }
  
    return SWIF_STATUS_OK;
}

swif_status_t   swif_rlc_decoder_add_source_symbol_to_coding_window (
                                swif_decoder_t* dec,
                                esi_t           new_src_symbol_esi)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * Remove this source symbol from the coding window.
 */
swif_status_t   swif_rlc_encoder_remove_source_symbol_from_coding_window (
                                swif_encoder_t* enc,
                                esi_t           old_src_symbol_esi)
{
// NOT YET
	return SWIF_STATUS_OK;
}

swif_status_t   swif_rlc_decoder_remove_source_symbol_from_coding_window (
                                swif_decoder_t* dec,
                                esi_t           old_src_symbol_esi)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * Get information on the current coding window at the encoder.
 */
swif_status_t   swif_rlc_encoder_get_coding_window_information (
                                swif_encoder_t* enc,
                                esi_t*          first,
                                esi_t*          last,
                                uint32_t*       nss)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/*******************************************************************************
 * Coding Coefficients Functions at an Encoder and Decoder
 */


/**
 * Encoder: this function specifies the coding coefficients chosen by
 *          the application if this is the way the codec works.
 * Decoder: communicate with this function the coding coefficients
 *          associated to a repair symbol and carried in the packet
 *          header.
 */
swif_status_t   swif_rlc_encoder_set_coding_coefs_tab (
                                swif_encoder_t* enc,
                                void*           coding_coefs_tab,
                                uint32_t        nb_coefs_in_tab)
{
// NOT YET
	return SWIF_STATUS_OK;
}


swif_status_t   swif_rlc_decoder_set_coding_coefs_tab (
                                swif_decoder_t* dec,
                                void*           coding_coefs_tab,
                                uint32_t        nb_coefs_in_tab)
{
// NOT YET
	return SWIF_STATUS_OK;
}


/**
 * The coding coefficients may be generated in a deterministic manner,
 * for instance by a PRNG known by the codec and a seed (perhaps with
 * other parameters) provided by the application.
 * The codec may also choose in an autonomous manner these coefficients.
 * This function is used to trigger this process.
 * When the choice is made in an autonomous manner, the actual coding
 * coefficient or key used by the codec can be retrieved with
 * swif_encoder_get_coding_coefs_tab().
 */
swif_status_t   swif_rlc_encoder_generate_coding_coefs (
                                swif_encoder_t* enc,
                                uint32_t        key,
                                uint32_t        add_param)
{
	return SWIF_STATUS_OK;
}


swif_status_t   swif_rlc_decoder_generate_coding_coefs (
                                swif_decoder_t* dec,
                                uint32_t        key,
                                uint32_t        add_param)
{
	return SWIF_STATUS_OK;
}


/**
 * This function enables the application to retrieve the set of coding
 * coefficients generated and used by build_repair_symbol(). This is
 * useful when the choice of coefficients is performed by the codec in
 * an autonomous manner but needs to be sent in the repair packet header.
 * This function is only used by an encoder.
 */
swif_status_t   swif_rlc_encoder_get_coding_coefs_tab (
                                swif_encoder_t* enc,
                                void**          coding_coefs_tab,
                                uint32_t*       nb_coefs_in_tab)
{
// NOT YET
	return SWIF_STATUS_OK;
}


swif_encoder_t* swif_rlc_encoder_create (swif_codepoint_t codepoint,
                                         uint32_t        verbosity,
                                         uint32_t        symbol_size,
                                         uint32_t        max_coding_window_size)
{
    /* initialize the encoder */

    assert(codepoint == SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC);
    swif_encoder_rlc_cb_t *enc = NULL;
    if ((enc = calloc(1,sizeof(swif_encoder_rlc_cb_t))) == NULL){
        fprintf(stderr, "swif_encoder_create() failed! No memory \n");
        return NULL;
    }
    enc->generic_encoder.codepoint = codepoint;
    enc->symbol_size = symbol_size;
    enc->max_coding_window_size = max_coding_window_size;
    if ((enc->cc_tab = calloc(max_coding_window_size, sizeof(uintptr_t))) == NULL){
        fprintf(stderr, "swif_encoder_create cc_tab failed! No memory \n");
        return NULL;
    }
    if ((enc->ew_tab = calloc(max_coding_window_size, sizeof(uintptr_t))) == NULL){
        fprintf(stderr, "swif_encoder_create ew_tab failed! No memory \n");
        return NULL;
    }
    enc->ew_left = 0;
    enc->ew_right = enc->ew_left;
    enc->ew_esi_right = INVALID_ESI;
    enc->ew_ss_nb = 0;

    enc->source_symbol_removed_from_coding_window_callback = NULL;
    enc->context_4_callback = NULL;

    enc->generic_encoder.generate_coding_coefs		= swif_rlc_encoder_generate_coding_coefs;
    enc->generic_encoder.get_coding_window_information	= swif_rlc_encoder_get_coding_window_information;
    enc->generic_encoder.get_coding_coefs_tab		= swif_rlc_encoder_get_coding_coefs_tab;
    enc->generic_encoder.get_parameters			= swif_rlc_encoder_get_parameters;
    enc->generic_encoder.set_parameters			= swif_rlc_encoder_set_parameters;
    enc->generic_encoder.set_callback_functions		= swif_rlc_encoder_set_callback_functions;
    enc->generic_encoder.set_coding_coefs_tab		= swif_rlc_encoder_set_coding_coefs_tab;
    enc->generic_encoder.remove_source_symbol_from_coding_window	= swif_rlc_encoder_remove_source_symbol_from_coding_window;
    enc->generic_encoder.add_source_symbol_to_coding_window		= swif_rlc_encoder_add_source_symbol_to_coding_window;
    enc->generic_encoder.reset_coding_window		= swif_rlc_encoder_reset_coding_window;
    enc->generic_encoder.build_repair_symbol		= swif_rlc_build_repair_symbol;
    return (swif_encoder_t *) enc;
}
