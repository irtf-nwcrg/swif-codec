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

if __name__ == "__main__":
    try_full_symbol_wrapper()
    

#---------------------------------------------------------------------------
