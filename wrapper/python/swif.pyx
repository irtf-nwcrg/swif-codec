#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

cimport libc.stdio as stdio
from libc.stdlib cimport malloc, free

cimport cswif
from cswif cimport *
from libc.stdint cimport uint8_t, uint32_t, int64_t, bool
from libc.stdlib cimport malloc, free
from libc.string cimport memcpy, memset
from cpython.object cimport Py_EQ, Py_NE

import numpy as np
import warnings

#---------------------------------------------------------------------------

cdef check_swif_status(swif_status, swif_errno):
    if swif_status == SWIF_STATUS_ERROR:
        raise RuntimeError("SWIF Error", swif_errno) #XXX

cpdef uint8_t* memclone(uint8_t* data, int data_size):
    cdef uint8_t* result = <uint8_t*> malloc(data_size)
    memcpy(result, data, data_size)
    return result

VERBOSITY = 2

cdef class RlcEncoder:
    cdef cswif.swif_encoder_t *encoder
    cdef symbol_size # XXX: replicated for convenience

    def __cinit__(self, symbol_size, max_coding_window_size):
        self.encoder = swif_encoder_create(
            SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC,
            VERBOSITY, symbol_size, max_coding_window_size)
        self.symbol_size = symbol_size
        if self.encoder is NULL:
            raise RuntimeError("SWIF Error", "encoder_create returned NULL")#XXX
	#if (swif_encoder_set_callback_functions(ses, source_symbol_removed_from_coding_window_callback, NULL) != SWIF_STATUS_OK) {

    def add_source_symbol_to_coding_window(self, uint8_t *symbol, symbol_id):
        assert self.encoder is not NULL
        cdef uint8_t* cloned_symbol = memclone(symbol, self.symbol_size)
        status = swif_encoder_add_source_symbol_to_coding_window(
            self.encoder, cloned_symbol, symbol_id)
        check_swif_status(status, self.encoder.swif_errno)        

    def build_repair_symbol(self):
        assert self.encoder is not NULL
        result = bytes(self.symbol_size)
        cdef uint8_t *data = result
        status = swif_build_repair_symbol(self.encoder, data) # XXX: ref bytes?
        check_swif_status(status, self.encoder.swif_errno)
        return result

    def generate_coding_coefs(self, key, add_param):
        status = swif_encoder_generate_coding_coefs(
            self.encoder, key, add_param)
        check_swif_status(status, self.encoder.swif_errno)        

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

cdef class RlcDecoder:
    cdef cswif.swif_decoder_t *decoder
    cdef symbol_size # XXX: replicated for convenience

    def __cinit__(self, symbol_size, max_coding_window_size):
        self.decoder = swif_decoder_create(
            SWIF_CODEPOINT_RLC_GF_256_FULL_DENSITY_CODEC,
            0, symbol_size, max_coding_window_size, 2*max_coding_window_size)
        self.symbol_size = symbol_size
        if self.decoder is NULL:
            raise RuntimeError("SWIF Error", "decoder_create returned NULL")#XXX

    # XXX add code here
    def decode_with_new_source_symbol(self, uint8_t* new_symbol_buf, esi_t new_size_esi):
        assert self.symbol_size == len(new_symbol_buf)
        swif_rlc_decoder_decode_with_new_source_symbol(self.decoder, new_symbol_buf, new_size_esi)

    def decode_with_new_repair_symbol(self, uint8_t* new_symbol_buf, esi_t new_size_esi):
        assert self.symbol_size == len(new_symbol_buf)
        swif_rlc_decoder_decode_with_new_repair_symbol(self.decoder, new_symbol_buf, new_size_esi)

    def decoder_generate_coding_coefs(self, uint32_t key, uint32_t add_param): 
        swif_rlc_decoder_generate_coding_coefs(self.decoder, key, add_param)
    
    def add_source_symbol_to_coding_window(self, esi_t symbol_id):
        assert self.decoder is not NULL
        status = swif_decoder_add_source_symbol_to_coding_window(
            self.decoder, symbol_id)
        check_swif_status(status, self.decoder.swif_errno)
    def decoder_reset_coding_window(self):
        swif_rlc_decoder_reset_coding_window(self.decoder)
    def decode_with_new_source_symbol(self, uint8_t* new_symbol_buf, esi_t new_size_esi):
        swif_rlc_decoder_decode_with_new_source_symbol(self.decoder, new_symbol_buf, new_size_esi)


    # XXX end of modifications 

#---------------------------------------------------------------------------

cdef class GF256Elem:
    cdef public uint8_t _value

    def __cinit__(self, value=0):
        self._value = self.int_to_polyint(value)

    cdef int_to_polyint(self, i):
        return i

    cdef polyint_to_int(self, uint8_t i):
        return i

    cpdef make(self, polyint):
        return GF256Elem(polyint)
        
    def __add__(self, other):
        return self.make(gf256_add(self._value, other._value))

    def __sub__(self, other):
        return self.make(gf256_sub(self._value, other._value))

    def __mul__(self, other):
        return self.make(gf256_mul(self._value, other._value))

    def __truediv__(self, other):
        return self.make(gf256_div(self._value, other._value))

    def inverse(self):
        return self.make(gf256_inv(self._value))

    def __repr__(self):
        return "GF256Elem("+repr(self.as_int())+")"

    def __pow__(self, v1, v2):
        if v2 is not None:
            raise ValueError("cannot compute pow modulo", v2)

        if isinstance(v1, GF256Elem):
            # XXX: what is the semantics of this?
            # technically it is polynomial power to another polynomial
            raise ValueError("not implemented GF256Elem ** GF256Elem", v1)
        
        if v1 < 0:
            return (self**(-v1)).inverse()
        result = self.make(1)
        current = self
        while v1>0:
            if v1 & 1 != 0:
                result = result * current
            current = current*current
            v1 = v1 >> 1
        return result

    def __richcmp__(self, other, int op):
        if op != Py_EQ and op != Py_NE:
            raise ValueError("Impossible comparison operation", op)
        if isinstance(other, GF256Elem):
            eq = (self._value == other._value)
        else:
            eq = (self.as_int() == other)
        if op == Py_EQ:
            return eq
        else:
            assert op == Py_NE
            return not eq

    def __hash__(self):
        return hash(self.value)

    def as_int(self):
        return self.polyint_to_int(self._value)

    cdef as_gf256(self):
        return GF256Elem.make(self, self._value)


cpdef GF256Elem gf256_one = GF256Elem(1)
cpdef GF256Elem gf256_zero = GF256Elem(0)
assert gf256_one._value == 1
assert gf256_zero._value == 0

cpdef gf256_roots_of_unity(p):
    result = []
    cdef int i
    for i in range(256):
        x = GF256Elem(i) 
        if x**p == 1:
            result.append(x)
    return result

# any element x of a subfield GF(2^q) will satisfy x**(q-1) == 1
# in our GF256, it appears that only subfield elem do so - XXX: prove?
subfield_GF16 = [0] + [x.as_int() for x in gf256_roots_of_unity(15)]
index_in_subfield_GF16 = { x:i for i,x in enumerate(subfield_GF16) }
subfield_GF4 = [0] + [x.as_int() for x in gf256_roots_of_unity(3)]
index_in_subfield_GF4 = { x:i for i,x in enumerate(subfield_GF4) }

assert subfield_GF16[1] == 1
assert subfield_GF4[1] == 1

cpdef as_gf16_int(GF256Elem elem):
    return index_in_subfield_GF16.get(elem._value)

cpdef as_gf4_int(GF256Elem elem):
    return index_in_subfield_GF4.get(elem._value)

cpdef as_gf2_int(GF256Elem elem):
    assert elem._value == 0 or elem._value == 1
    return elem._value

cpdef make_gf16(int i):
    assert 0 <=i and i<16
    return GF256Elem(subfield_GF16[i])

cpdef make_gf4(int i):
    assert 0 <=i and i<4
    return GF256Elem(subfield_GF4[i])

cpdef make_gf2(int i):
    assert 0 <=i and i<2
    return GF256Elem(i)

#---------------------------------------------------------------------------

cdef class Symbol:
    """
    `Symbol' is a wrapper around the low level api functions:
      symbol_add_scaled, symbol_add, symbol_sub, symbol_mul, symbol_div
    The `Symbol' is a sequence of bytes, stored in `data' and of length `size'.
    GF(256) operations are performed.
    """
    cdef public uint8_t *data
    cdef public uint32_t size

    def __cinit__(self, uint8_t *init_data=NULL):
        self.data = NULL
        self.size = 0
        if init_data is not NULL:
            self.set_data(init_data)

    def get_data(self):
        if self.size == 0:
            return b""
        result = bytes(self.size)
        cdef uint8_t *data = result        
        memcpy(data, self.data, self.size)
        return result

    cpdef alloc(self, new_size):
        if self.data is not NULL:
            self.dealloc()
        assert new_size >= 0
        self.data = <uint8_t*>malloc(new_size)
        memset(self.data, 0, new_size)
        if self.data is NULL and new_size > 0:
            self.size = 0
            raise RuntimeError("cannot malloc", new_size)
        self.size = new_size

    cpdef dealloc(self):
        if self.data is NULL:
            return
        free(self.data)
        self.data = NULL
        self.size = 0

    def set_data(self, data):
        self.dealloc()
        self.alloc(len(data))
        assert len(data) == self.size        
        cdef uint8_t *u8_data = data
        memcpy(self.data, u8_data, len(data))

    def __dealloc__(self):
        if self.data is not NULL:
            free(self.data)
            self.data = NULL
            self.size = 0

    def add(self, other):
        assert self.size == other.size
        result = Symbol()
        result.alloc(self.size)
        cdef uint8_t *other_data = other.data
        symbol_add(self.data, other_data, self.size, result.data)
        return result

    def mul(self, coef):
        result = Symbol()
        result.alloc(self.size)
        symbol_mul(self.data, coef, self.size, result.data)
        return result

    def div(self, coef):
        result = Symbol()
        result.alloc(self.size)
        symbol_div(self.data, self.size, coef, result.data)
        return result

    def sub(self, other):
        assert self.size == other.size
        result = Symbol()
        result.alloc(self.size)
        cdef uint8_t *other_data = other.data
        symbol_sub(self.data, other_data, self.size, result.data)
        return result

    def __add__(self, other):
        return self.add(other)

    def __sub__(self, other):
        return self.sub(other)

    def __mul__(v1, v2):
        if isinstance(v1, Symbol):
            return v1.mul(v2)
        else:
            return v2.mul(v1)

    def __truediv__(self, coef):
        return self.div(coef)

    def __repr__(self):
        return "Symbol("+repr([x for x in self.get_data()])+")"

    def copy(self):
        return Symbol(self.get_data())

#---------------------------------------------------------------------------

cdef class FullSymbol:
    cdef swif_full_symbol_t *symbol

    def __init__(self, info=None):
        self.symbol = NULL
        if info is not None:
            self.init_from_info(info)
        else:
            self.to_zero()

    def init_from_info(self, info):
        if isinstance(info, tuple):
            assert len(info) == 2 or len(info) == 3
            if len(info) == 2:
                first_id = 0
                header, data = info
            else:
                first_id, header, data = info
            header_bytes = bytes(header)
            data_bytes = bytes(data)
            self.symbol = full_symbol_create(
                header_bytes, first_id, len(header_bytes),
                data_bytes, len(data_bytes))

    cpdef to_zero(self):
        self.release()
        self.symbol = full_symbol_alloc(SYMBOL_ID_NONE, SYMBOL_ID_NONE, 0)

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

    cdef from_other_c(self, swif_full_symbol_t *other_symbol):
        self.release()
        self.symbol = full_symbol_clone(other_symbol)
    
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
        full_symbol_get_data(self.symbol, result) # XXX: passing `result' safe?
        return result

    cpdef clone(self):
        assert self.symbol is not NULL    
        return FullSymbol().from_other(self)

    cpdef copy(self):
        return self.clone()

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
        return self.get_coefs()+(self.get_data(),)

    cpdef dump(self):
        assert self.symbol is not NULL
        return full_symbol_dump(self.symbol, stdio.stdout) 

    cpdef _add_base(self, FullSymbol other1, FullSymbol other2):
        return full_symbol_add_base(other1.symbol, other2.symbol, self.symbol)

    cdef replace_symbol(self, swif_full_symbol_t *symbol):
        self.release()
        self.symbol = symbol
    
    cpdef add(self, FullSymbol other):
        result_symbol = full_symbol_add(self.symbol, other.symbol)
        result = FullSymbol()
        #assert result.symbol is NULL
        result.replace_symbol(result_symbol)
        #result.symbol = result_symbol
        return result

    cpdef _scale(self, coef):
        full_symbol_scale(self.symbol, coef)
        return self # XXX

    cpdef _scale_inv(self, coef):
        full_symbol_scale(self.symbol, gf256_inv(coef))
        return self

    def __repr__(self):
        offset, header, content = self.get_info()
        h = list(b"\x00"*offset+header)
        c = list(content)
        return repr((h,c))
        #return repr(self.get_info())

    def __add__(self, other):
        return self.add(other)

    def __sub__(self, other):
        return self.sub(other)

    def __mul__(v1, v2):
        if isinstance(v1, GF256Elem):
            v1 = v1.value
        if isinstance(v2, GF256Elem):
            v2 = v2.value
        if isinstance(v1, FullSymbol):
            return v1.clone()._scale(v2)
        else:
            return v2.clone()._scale(v1)

    def __truediv__(self, coef):
        return self.div(coef)


#---------------------------------------------------------------------------

cdef class FullSymbolSet:

    cdef swif_full_symbol_set_t* symbol_set

    def __init__(self):
        self.symbol_set = NULL
        self._allocate()

    def set_notify(self): # XXX
        self.symbol_set.notify_decoded_func = notify_decoded_print
        
    def alloc_set(self):
        warnings.warn("obsolete method")
        return self

    cpdef release_set(self):
        if self.symbol_set is NULL:
            return
        full_symbol_set_free(self.symbol_set)
        self.symbol_set = NULL

    def __dealloc__(self):
        if self.symbol_set is not NULL:
            full_symbol_set_free(self.symbol_set)
            self.symbol_set = NULL

    cpdef _allocate(self):
        assert self.symbol_set is NULL
        self.symbol_set = full_symbol_set_alloc()


    cpdef set_add(self, FullSymbol other):
        return full_symbol_set_add(self.symbol_set, other.symbol)

    cpdef dump(self):
        assert self.symbol_set is not NULL
        return full_symbol_set_dump(self.symbol_set, stdio.stdout) 

    def get_pivot(self, symbol_id): # XXX: not working well
        assert self.symbol_set is not NULL
        cdef swif_full_symbol_t *full_symbol = full_symbol_set_get_pivot(
            self.symbol_set, symbol_id)
        cdef FullSymbol result = FullSymbol()
        if full_symbol is not NULL:
            result.from_other_c(full_symbol)
        else:
            result = None
        return result

    def remove_each_pivot(self,  FullSymbol new_symbol):
        assert self.symbol_set is not NULL
        assert new_symbol.symbol is not NULL
        full_symbol_set_remove_each_pivot(self.symbol_set, new_symbol.symbol)
        return self

    def add_as_pivot(self,  FullSymbol new_symbol):
        assert self.symbol_set is not NULL
        assert new_symbol.symbol is not NULL
        full_symbol_set_add_as_pivot(self.symbol_set, new_symbol.symbol)
        return self

    def add_with_elimination(self,  FullSymbol new_symbol):
        assert self.symbol_set is not NULL
        assert new_symbol.symbol is not NULL
        res=full_symbol_add_with_elimination(self.symbol_set, new_symbol.symbol)
        return res

    def get_min_id(self): # min, included
        return self.symbol_set.first_symbol_id

    def get_max_id(self): # max, included
        return self.get_min_id()+self.symbol_set.size-1

    def get(self, pivot_id):
        if pivot_id < self.get_min_id() or pivot_id > self.get_max_id():
            return None
        idx = pivot_id - self.get_min_id()
        cdef swif_full_symbol_t *fs = NULL
        if self.symbol_set.full_symbol_tab[idx] is not NULL:
            fs = self.symbol_set.full_symbol_tab[idx]
            result = FullSymbol()
            result.from_other_c(fs)
            return result
        else: return None

    def get_matrix(self, nb_col=None):
        symbol_list = []
        for symbol_id in  range(self.get_min_id(), self.get_max_id()+1):
            symbol = self.get(symbol_id)
            if symbol is not None:
                symbol_list.append(symbol)
        return to_matrix(symbol_list, nb_col)


def to_matrix(symbol_list, nb_col=None):
    if len(symbol_list) == 0:
        return np.array([[]], type=np.int)
    max_id = max([symbol.get_max_symbol_id() for symbol in symbol_list])
    if nb_col is not None:
        max_id = nb_col-1
    def to_int_list(coefs):
        min_id, coef_bytes = coefs
        result = min_id*[0] + list(coef_bytes)
        result = result + (max_id+1 - len(result))* [0]
        return result
    result = [to_int_list(symbol.get_coefs()) for symbol in symbol_list]
    return np.array(result, dtype=np.int)

def compute_rref(matrix):
    if len(matrix) == 0:
        return matrix.copy()
    symbol_set = FullSymbolSet()
    for row in matrix:
        symbol = FullSymbol((0, list(row), b""))
        symbol_set.add_with_elimination(symbol)
    return symbol_set.get_matrix(len(matrix[0]))

#---------------------------------------------------------------------------

cdef public void notify_decoded_print(swif_full_symbol_set_t *set, symbol_id_t symbol_id, void *context):
    print("notify decoded:",symbol_id)

#---------------------------------------------------------------------------
