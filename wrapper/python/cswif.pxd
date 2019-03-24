#---------------------------------------------------------------------------
# C.A. - 2019
#---------------------------------------------------------------------------
# Definitions of the "C" structure, functions etc.
# of SWIF includes.
#---------------------------------------------------------------------------

from libc.stdint cimport uint8_t, uint32_t, int64_t, bool

cdef extern from "../../src/swif_api.h":
    ctypedef enum swif_status_t:
        SWIF_STATUS_OK = 0
        SWIF_STATUS_FAILURE
        SWIF_STATUS_ERROR

    ctypedef enum swif_errno_t:
        SWIF_ERRNO_NULL = 0
        SWIF_ERRNO_UNSUPPORTED_CODEPOINT

    ctypedef enum swif_codepoint_t:
        SWIF_CODEPOINT_NULL = 0
        SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC

    ctypedef uint32_t esi_t

    ctypedef struct swif_encoder_t:
        swif_codepoint_t codepoint;
        swif_errno_t     swif_errno;

    swif_encoder_t* swif_encoder_create (
        swif_codepoint_t codepoint,
        uint32_t        verbosity,
        uint32_t        symbol_size,
        uint32_t        max_coding_window_size);

    swif_status_t swif_encoder_release (swif_encoder_t* enc);

#---------------------------------------------------------------------------

cdef extern from "../../src/swif_rlc_cb.h":
    ctypedef struct swif_encoder_rlc_cb_t:
        swif_codepoint_t  codepoint
        swif_errno_t      swif_errno
        uint32_t          verbosity
        uint32_t          max_coding_window_size
        uint32_t          symbol_size
        uint8_t	          *cc_tab
        uint32_t          cw_nb_in_list

#---------------------------------------------------------------------------

cdef extern from "../../src/swif_full_symbol.h":
     cdef struct s_swif_full_symbol_t:
        pass
     ctypedef s_swif_full_symbol_t swif_full_symbol_t

     swif_full_symbol_t *full_symbol_create_from_source(
         uint32_t full_symbol_id, uint8_t *full_symbol_data,
         uint32_t full_symbol_size);
     
     swif_full_symbol_t *full_symbol_create(
         uint8_t *symbol_coef_table, uint32_t min_symbol_id,
         uint32_t nb_symbol_id,
         uint8_t *symbol_data, uint32_t symbol_size);

     uint32_t full_symbol_get_size(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_get_min_symbol_id(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol);

     uint32_t full_symbol_count_coef(swif_full_symbol_t *full_symbol);

     uint8_t  full_symbol_get_coef(swif_full_symbol_t *full_symbol,
                                   uint32_t symbol_id);

     bint full_symbol_is_zero(swif_full_symbol_t *full_symbol);

     void full_symbol_free(swif_full_symbol_t* swif_full_symbol);
     
     #swif_full_symbol_t *full_symbol_clone(
     #    swif_full_symbol_t* swif_full_symbol);

#---------------------------------------------------------------------------
