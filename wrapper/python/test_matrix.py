#---------------------------------------------------------------------------
# Test of Gaussian Elimination
#---------------------------------------------------------------------------

import numpy as np
import swif

#---------------------------------------------------------------------------

np.random.seed(1)

spec_list = [
    (2, 1, []),
    (3, 1, []),
    (4, 5, [7]),
    (4, 5, [7]),
    (5, 4, [7]),
    (5, 6, [7]),
    (6, 4, [7])
]

def make_random_symbol(start, nb_coef, zero_list):
    coefs = list(np.random.randint(1, 256, size=nb_coef))
    if nb_coef == 1:
        coefs[0] = 1
    for zero_pos in zero_list:
        coefs[zero_pos-start] = 0
    return swif.FullSymbol((start, coefs, b""))

symbol_list = [ make_random_symbol(start, nb_coef, zero_list)
                for (start, nb_coef, zero_list) in spec_list ]

print(symbol_list)
M = swif.to_matrix(symbol_list)
print(M)

#---------------------------------------------------------------------------

symbols = swif.FullSymbolSet()
for symbol in symbol_list:
    symbols.add_with_elimination(symbol)

print(symbols.get_matrix())
    
#---------------------------------------------------------------------------
