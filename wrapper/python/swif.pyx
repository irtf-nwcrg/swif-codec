#---------------------------------------------------------------------------
# C.A. - 2019
#---------------------------------------------------------------------------

cimport cswif
from cswif cimport *

#---------------------------------------------------------------------------

cdef check_swif_status(swif_status, swif_errno):
    if swif_status == SWIF_STATUS_ERROR:
        raise RuntimeError("SWIF Error", swif_errno) #XXX

cdef class RlcEncoder:
    cdef cswif.swif_encoder_t* encoder

    def __cinit__(self, symbol_size, max_coding_window_size):
        self.encoder = swif_encoder_create(
            SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC,
            0, symbol_size, max_coding_window_size)
        if self.encoder == NULL:
            raise RuntimeError("SWIF Error", "encoder_create returned NULL") #XXX

    cpdef release(self):
        if self.encoder is NULL:
            return
        status = swif_encoder_release(self.encoder)
        check_swif_status(status, self.encoder.swif_errno)
        self.encoder = NULL

    def __dealloc__(self):
        if self.encoder is not NULL:
            status = swif_encoder_release(self.encoder)
            self.encoder = NULL

#---------------------------------------------------------------------------

class RlcDecoder:
    def __init__(self, max_window_size, symbol_size):
        self.decoder = None

#---------------------------------------------------------------------------
