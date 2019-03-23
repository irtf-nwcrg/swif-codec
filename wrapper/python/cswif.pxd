#---------------------------------------------------------------------------
# C.A. - 2019
#---------------------------------------------------------------------------

from libc.stdint cimport uint8_t, uint32_t, int64_t

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
