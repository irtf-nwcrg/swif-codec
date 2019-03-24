#---------------------------------------------------------------------------
# C.A. - 2019
#---------------------------------------------------------------------------

import swif

SYMBOL_SIZE = 1024
WINDOW_SIZE = 10

#encoder = swif.RlcEncoder(SYMBOL_SIZE, WINDOW_SIZE)
#encoder.release()

#---------------------------------------------------------------------------

symbol = swif.FullSymbol().from_source_symbol(6, b"azertyuiop")
print(symbol.is_zero())
print(symbol.get_min_symbol_id())
print(symbol.get_max_symbol_id())
print(symbol.get_coefs())

#---------------------------------------------------------------------------
