#---------------------------------------------------------------------------
# C.A. - 2019
#---------------------------------------------------------------------------

cimport stdio

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
            raise RuntimeError("SWIF Error", "encoder_create returned NULL")#XXX

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

cdef class FullSymbol:
    cdef swif_full_symbol_t* symbol

    def __init__(self):
        self.symbol = NULL

    cpdef from_source_symbol(self, symbol_id, content):
        self.release()
        self.symbol = full_symbol_create_from_source(
            symbol_id, content, len(content))
        return self

    cpdef from_coefs_and_symbol(self, first_symbol_id,
                                symbol_id_table, content):
        self.release()
        self.symbol = full_symbol_create(
            symbol_id_table, first_symbol_id, len(symbol_id_table),
            content, len(content))
        return self

    cpdef from_other(self, FullSymbol other):
        self.release()
        self.symbol = full_symbol_clone(other.symbol)
        return self

    cpdef is_zero(self):
        assert self.symbol is not NULL    
        return full_symbol_is_zero(self.symbol)
    
    cpdef get_size(self):
        assert self.symbol is not NULL
        return full_symbol_get_size(self.symbol)
    
    cpdef get_min_symbol_id(self):
        assert self.symbol is not NULL
        return full_symbol_get_min_symbol_id(self.symbol)

    cpdef get_max_symbol_id(self):
        assert self.symbol is not NULL
        return full_symbol_get_max_symbol_id(self.symbol)

    cpdef count_coefs(self):
        assert self.symbol is not NULL    
        return full_symbol_count_coef(self.symbol)

    cpdef get_coef(self, symbol_id):
        assert self.symbol is not NULL
        return full_symbol_get_coef(self.symbol, symbol_id)

    def get_coefs(self):
        if self.is_zero():
            return (0, b"")
        min_id = self.get_min_symbol_id()
        nb_coefs = self.count_coefs()
        content = bytes([self.get_coef(min_id+i) for i in range(nb_coefs)])
        return min_id, content

    cpdef get_data(self):
        assert self.symbol is not NULL    
        symbol_size = self.get_size()
        result = bytes(symbol_size)
        full_symbol_get_data(self.symbol, result)
        return result

    cpdef clone(self):
        assert self.symbol is not NULL    
        return FullSymbol().from_other(self)

    cpdef release(self):
        if self.symbol is NULL:
            return
        full_symbol_free(self.symbol)
        self.symbol = NULL

    def __dealloc__(self):
        if self.symbol is not NULL:
            full_symbol_free(self.symbol)
            self.symbol = NULL

    cpdef get_info(self):
        assert self.symbol is not NULL
        #if not self.is_zero():
        return self.get_coefs()+(self.get_data(),)
        #else:
        #    return (0, b"", self.get_data())

    cpdef dump(self):
        assert self.symbol is not NULL
        return full_symbol_dump(self.symbol, stdio.stdout)

#---------------------------------------------------------------------------
