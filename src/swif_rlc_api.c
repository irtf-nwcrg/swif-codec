#define DEBUG 1

#include "swif_includes.h"
#include "swif_rlc_api.h"
#include "swif_coding_coefficients.h"

/*******************************************************************************
 * Encoder functions
 */


/**
 * Release an encoder and its associated ressources.
 **/
swif_status_t   swif_rlc_encoder_release (swif_encoder_t*        enc)
{
	assert(enc);
    swif_encoder_rlc_cb_t *rlc_enc = (swif_encoder_rlc_cb_t *) enc;
    if (rlc_enc->cc_tab)
        free(rlc_enc->cc_tab);
    if (rlc_enc->ew_tab)
        free(rlc_enc->ew_tab);
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
                                void**           new_buf)
{
    swif_encoder_rlc_cb_t* enc = (swif_encoder_rlc_cb_t*) generic_encoder;
    uint32_t	i;

    if ((*new_buf = calloc(1, enc->symbol_size)) == NULL) {
        fprintf(stderr, "swif_rlc_build_repair_symbol failed! No memory\n");
        return SWIF_STATUS_ERROR;
    }

    DEBUG_PRINT("\nbuild-repair: \n");
    for (i = enc->ew_left; i < enc->ew_ss_nb; i++) {
        uint32_t idx = i % enc->max_coding_window_size;
        DEBUG_PRINT(" +%u.P[%u->%u]", enc->cc_tab[idx], i, idx);
        symbol_add_scaled(*new_buf, enc->cc_tab[idx], enc->ew_tab[idx], enc->symbol_size);
    }
    DEBUG_PRINT("\n");
    return SWIF_STATUS_OK;
}


/*******************************************************************************
 * Decoder functions
 */


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
        swif_decoder_rlc_cb_t *rlc_dec = (swif_decoder_rlc_cb_t *) dec;
        swif_full_symbol_t *full_symbol = full_symbol_create_from_source(
                 new_symbol_esi, new_symbol_buf, rlc_dec->symbol_size);
        full_symbol_set_add(rlc_dec->symbol_set, full_symbol);
        fprintf(stderr, "[XXX] not checking if too many stored symbols\n");
	return SWIF_STATUS_OK;
}


/**
 * Submit a received repair symbol and try to progress in the decoding.
 * For each decoded source symbol (if any), the application is informed
 * through the dedicated callback functions.
 */
swif_status_t   swif_rlc_decoder_decode_with_new_repair_symbol (
                                swif_decoder_t* dec,
                                void* const     new_symbol_buf,
                                esi_t           new_symbol_esi)
{
        swif_decoder_rlc_cb_t *rlc_dec = (swif_decoder_rlc_cb_t *) dec;
        //XXX;
        swif_full_symbol_t *full_symbol = NULL;
        full_symbol = full_symbol_create_from_source(new_symbol_esi, new_symbol_buf, rlc_dec->symbol_size);
        full_symbol_set_add(rlc_dec->symbol_set, full_symbol);
        fprintf(stderr, "[XXX] not checking if too many stored symbols\n");
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

swif_status_t   swif_rlc_decoder_reset_coding_window (swif_decoder_t*  dec)
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
        fprintf(stderr, "swif_rlc_encoder_add_source_symbol_to_coding_window() failed! new_src_symbol_esi (%u) is not the expected value (%u)\n",
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
    } else if (enc->ew_ss_nb == 0) {
	assert(enc->ew_left == enc->ew_right);
        enc->ew_tab[enc->ew_right] = new_src_symbol_buf;
        enc->ew_esi_right = new_src_symbol_esi;
        enc->ew_ss_nb++;
    } else {
        enc->ew_right = (enc->ew_right + 1) % enc->max_coding_window_size;
        enc->ew_tab[enc->ew_right] = new_src_symbol_buf;
        enc->ew_esi_right = new_src_symbol_esi;
        enc->ew_ss_nb++;
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
                                swif_encoder_t* generic_encoder,
                                esi_t*          first,
                                esi_t*          last,
                                uint32_t*       nss)
{
    swif_encoder_rlc_cb_t* enc = (swif_encoder_rlc_cb_t*) generic_encoder;

    *first = enc->ew_esi_right - enc->ew_ss_nb + 1;
    *last = enc->ew_esi_right;
    *nss = enc->ew_ss_nb;
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
    swif_encoder_rlc_cb_t *rlc_enc = (swif_encoder_rlc_cb_t *) enc;
    assert(nb_coefs_in_tab <= rlc_enc->max_coding_window_size);
    memcpy(rlc_enc->cc_tab, coding_coefs_tab, nb_coefs_in_tab*sizeof(uint8_t));
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
        /* XXX: check why uint32_t key */
        DEBUG_PRINT("generate coding coefs: ");
        swif_encoder_rlc_cb_t *rlc_enc = (swif_encoder_rlc_cb_t *) enc;
        
        if (rlc_enc->cc_tab == NULL) {
                /* XXX: need to use allocation functions */
                rlc_enc->cc_tab = (uint8_t*) malloc(
                    rlc_enc->max_coding_window_size*sizeof(uint8_t) );
                if (rlc_enc->cc_tab == NULL) {
                        fprintf(stderr, "Error, swif_rlc_encoder_generate_"
                                "coding_coefs: malloc failed\n");
                        return SWIF_STATUS_ERROR;
                }
        }

        assert(rlc_enc->ew_ss_nb <= rlc_enc->max_coding_window_size);
        swif_rlc_generate_coding_coefficients (
            (uint16_t)key, rlc_enc->cc_tab,
            rlc_enc->ew_ss_nb, /* upper bound: enc->max_window_size */
            15 /* density dt [0-15] XXX dt=1*/,
            8 /*=m - GF(2^^m) */);
        DEBUG_DUMP(rlc_enc->cc_tab, rlc_enc->ew_ss_nb);
	return SWIF_STATUS_OK;
}


swif_status_t   swif_rlc_decoder_generate_coding_coefs (
                                swif_decoder_t* dec,
                                uint32_t        key,
                                uint32_t        add_param)
{
        // repair keys
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


/*******************************************************************************
 * Session creation functions (last position to avoid compilation errors for
 * codec specific functions and avoid the need to add prototypes in header).
 */

/**
 * Create and initialize an encoder, providing only key parameters.
 **/
swif_encoder_t* swif_rlc_encoder_create (swif_codepoint_t codepoint,
                                         uint32_t        verbosity,
                                         uint32_t        symbol_size,
                                         uint32_t        max_coding_window_size)
{
    swif_encoder_rlc_cb_t*	enc;

    /* initialize the encoder */
    assert(codepoint == SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC);
    if ((enc = calloc(1,sizeof(swif_encoder_rlc_cb_t))) == NULL){
        fprintf(stderr, "swif_encoder_create() failed! No memory \n");
        return NULL;
    }
    enc->generic_encoder.codepoint = codepoint;
    enc->symbol_size = symbol_size;
    enc->max_coding_window_size = max_coding_window_size;
    if ((enc->cc_tab = calloc(max_coding_window_size, sizeof(uint8_t))) == NULL){
        fprintf(stderr, "swif_encoder_create cc_tab failed! No memory \n");
        return NULL;
    }

    if ((enc->ew_tab = calloc(max_coding_window_size, sizeof(uintptr_t))) == NULL){
        fprintf(stderr, "swif_encoder_create ew_tab failed! No memory \n");
        return NULL;
    }
    enc->ew_right = enc->ew_left = 0;
    enc->ew_esi_right = INVALID_ESI;
    enc->ew_ss_nb = 0;

    enc->source_symbol_removed_from_coding_window_callback = NULL;
    enc->context_4_callback = NULL;

    enc->generic_encoder.set_callback_functions		= swif_rlc_encoder_set_callback_functions;
    enc->generic_encoder.set_parameters			= swif_rlc_encoder_set_parameters;
    enc->generic_encoder.get_parameters			= swif_rlc_encoder_get_parameters;
    enc->generic_encoder.build_repair_symbol		= swif_rlc_build_repair_symbol;
    enc->generic_encoder.reset_coding_window		= swif_rlc_encoder_reset_coding_window;
    enc->generic_encoder.add_source_symbol_to_coding_window		= swif_rlc_encoder_add_source_symbol_to_coding_window;
    enc->generic_encoder.remove_source_symbol_from_coding_window	= swif_rlc_encoder_remove_source_symbol_from_coding_window;
    enc->generic_encoder.get_coding_window_information	= swif_rlc_encoder_get_coding_window_information;
    enc->generic_encoder.set_coding_coefs_tab		= swif_rlc_encoder_set_coding_coefs_tab;
    enc->generic_encoder.generate_coding_coefs		= swif_rlc_encoder_generate_coding_coefs;
    enc->generic_encoder.get_coding_coefs_tab		= swif_rlc_encoder_get_coding_coefs_tab;
    return (swif_encoder_t *) enc;
}


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
    swif_decoder_rlc_cb_t*	dec;

    /* initialize the decoder */
    assert(codepoint == SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC);
    if ((dec = calloc(1,sizeof(swif_decoder_rlc_cb_t))) == NULL) {
        fprintf(stderr, "swif_decoder_create() failed! No memory \n");
        return NULL;
    }
    dec->generic_decoder.codepoint = codepoint;
    dec->symbol_size = symbol_size;
    dec->max_coding_window_size = max_coding_window_size;
    dec->max_linear_system_size = max_linear_system_size;
    dec->symbol_set = full_symbol_set_alloc();
#if 0
    dec->ew_right = dec->ew_left = 0;
    dec->ew_esi_right = INVALID_ESI;
    dec->ew_ss_nb = 0;
#endif

    dec->source_symbol_removed_from_linear_system_callback = NULL;
    dec->decodable_source_symbol_callback = NULL;
    dec->decoded_source_symbol_callback = NULL;
    dec->context_4_callback = NULL;

    dec->generic_decoder.set_callback_functions		= swif_rlc_decoder_set_callback_functions;
    dec->generic_decoder.set_parameters			= swif_rlc_decoder_set_parameters;
    dec->generic_decoder.get_parameters			= swif_rlc_decoder_get_parameters;
    dec->generic_decoder.decode_with_new_source_symbol	= swif_rlc_decoder_decode_with_new_source_symbol;
    dec->generic_decoder.decode_with_new_repair_symbol	= swif_rlc_decoder_decode_with_new_repair_symbol;
    dec->generic_decoder.reset_coding_window		= swif_rlc_decoder_reset_coding_window;
    dec->generic_decoder.add_source_symbol_to_coding_window		= swif_rlc_decoder_add_source_symbol_to_coding_window;
    dec->generic_decoder.remove_source_symbol_from_coding_window	= swif_rlc_decoder_remove_source_symbol_from_coding_window;
    dec->generic_decoder.set_coding_coefs_tab		= swif_rlc_decoder_set_coding_coefs_tab;
    dec->generic_decoder.generate_coding_coefs		= swif_rlc_decoder_generate_coding_coefs;
    return (swif_decoder_t *) dec;
}

