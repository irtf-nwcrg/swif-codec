#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

import sys
import swif

SYMBOL_SIZE = 1024
WINDOW_SIZE = 10

#encoder = swif.RlcEncoder(SYMBOL_SIZE, WINDOW_SIZE)
#encoder.release()

#---------------------------------------------------------------------------

def try_full_symbol_wrapper():
    #symbol = swif.FullSymbol().from_source_symbol(6, b"azertyuiop")
    symbol = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([0,0,1,1,0,0]), b"azertyuiop")
    print(symbol.is_zero())
    print(symbol.get_info())
    print(symbol.get_min_symbol_id())
    print(symbol.get_max_symbol_id())
    print(symbol.get_coefs())
    print(symbol.get_size())
    print(symbol.count_coefs())
    print(symbol.get_data())
    print(symbol.clone().get_info())    
    symbol.dump();print(); sys.stdout.flush()


#---------------------------------------------------------------------------

def test_full_symbol_wrapper():
    symbol_id = 10
    content = b"AZERTY"
    symbol = swif.FullSymbol().from_source_symbol(symbol_id, content)
    assert symbol.get_size() == len(content)
    assert symbol.get_min_symbol_id() == symbol_id
    assert symbol.get_max_symbol_id() == symbol_id
    assert symbol.get_data() == content
    assert symbol.get_coefs() == (symbol_id, bytes([1]))
    assert symbol.count_coefs() == 1

#---------------------------------------------------------------------------

def try_full_symbol_add_base():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        8, bytes([0,0,1,0]), b"aaeuyo")
    symbol_result = swif.FullSymbol().from_coefs_and_symbol(
        5, bytes([0,0,0,0,0,1,1,1]), b"azeyui")
    print(symbol1.dump())
    print(symbol2.dump())
    #print(symbol_result.dump())
    print(symbol_result._add_base(symbol1,symbol2))
    print(symbol_result.dump())
    #assert symbol_result.first_nonzero_id == 10
    #assert symbol_result.last_nonzero_id == 12
    assert symbol_result.get_coefs() == (6, b'\x01\x00\x00\x01\x01')
    #assert symbol_result->first_nonzero_id == 6
    #assert symbol_result->last_nonzero_id == 10
    assert symbol_result.get_data() == b'\x00\x1b\x00\x00yo'

#---------------------------------------------------------------------------

def try_full_symbol_add():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        8, bytes([0,0,1,0]), b"aaeuyo")
    print(symbol1._add(symbol2))
    #assert symbol1.get_coefs() == (6, b'\x01\x00\x00\x01\x01')
    #assert symbol1.get_data() == b'\x00\x1b\x00\x00yo'
    print(symbol1.get_coefs())
    print(symbol1.get_data())
#---------------------------------------------------------------------------

def try_full_symbol_scale():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    coef = 2
    print(symbol1._scale(coef))
    print(symbol1.dump())
    assert symbol1.get_coefs() == (0, b'')
    assert symbol1.get_data() == b'\xa3\x8e\xaf\x9f'



#---------------------------------------------------------------------------

def try_full_symbol_scale_inverted():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    coef = 5
    print(symbol1._scale_inv(coef))

#---------------------------------------------------------------------------

if __name__ == "__main__":
    try_full_symbol_wrapper()
    test_full_symbol_wrapper()
    #try_full_symbol_add_base()
    #//////try_full_symbol_add()
    #try_full_symbol_scale()
    #try_full_symbol_scale_inverted()
    #try_misc()

#---------------------------------------------------------------------------
