#---------------------------------------------------------------------------
# Example of Gaussian Elimination
#---------------------------------------------------------------------------

import numpy as np
import swif

#---------------------------------------------------------------------------

def make_random_symbol(start, nb_coef, zero_list, possible_coefs=None):
    if possible_coefs is None:
        possible_coefs = list(range(1, 256))
    coefs = [possible_coefs[i]
             for i in np.random.randint(0, len(possible_coefs), size=nb_coef)]
    if nb_coef == 1:
        coefs[0] = 1
    for zero_pos in zero_list:
        coefs[zero_pos-start] = 0
    return swif.FullSymbol((start, coefs, b""))

#---------------------------------------------------------------------------
# Representation of systems and matrices
#--------------------------------------------------

def to_latex_matrix(matrix, repr_coef_function=None):
    if repr_coef_function is None:
        repr_coef_function = str
            
    if len(matrix) == 0:
        return r"\left( \begin{array}{} \end{array} \right)"

    nb_col = len(matrix[0])
    r = r"\left( \begin{array}{"+nb_col*"c"+"}"

    for row in matrix:
        r += " & ".join([repr_coef_function(c) for c in row]) + r"\\"

    r += r"\end{array} \right)"
    return r

def basic_to_str_matrix(matrix):
    result = []
    zero = r"{\color{gray}{\scriptsize{0}}}"
    for row in matrix:
        (nz_array,) = np.nonzero(row)
        if len(nz_array) == 0:
            # redundant packet
            str_row = len(row) * [zero] 
        elif len(nz_array) == 1: # and row[nz_array[0]] == 1:
            # decoded packet
            str_row = len(row) * [zero]
            str_row[nz_array[0]] = r"\textbf{1}"
        else:
            str_row = [r"\textbf{"+str(x)+"}" for x in row]
            for i in range(0,nz_array[0]):
                str_row[i] = zero
            for i in range(nz_array[-1]+1, len(row)):
                str_row[i] = zero                
        result.append(str_row)
    return result


def to_str_matrix(matrix):
    result = []
    #zero = "."
    zero = r"{\color{gainsboro}{\scriptsize{0}}}"    

    def color_str(x, is_in_range):
        if x == 0 and not is_in_range:
            return r"\textbf{"+str(x)+"}"
        else:
            return r"{\color{red}{\textbf{"+str(x)+"}}}"

    for row in matrix:
        (nz_array,) = np.nonzero(row)
        if len(nz_array) == 0:
            # redundant packet
            str_row = len(row) * [zero] 
        elif len(nz_array) == 1: # and row[nz_array[0]] == 1:
            # decoded packet
            str_row = len(row) * [zero]
            str_row[nz_array[0]] = color_str(row[nz_array[0]], True)
        else:
            str_row = [r"{"+color_str(x, nz_array[0]<=i<=nz_array[-1])+"}"
                       for i,x in enumerate(row)]
        result.append(str_row)

    for j,column in enumerate(matrix.T):
        (nz_array,) = np.nonzero(column)
        if (len(nz_array) == 0
            or (len(nz_array) == 1
                and len(np.nonzero(matrix[nz_array[0]])[0]) <= 1)):
            for i,c in enumerate(column):
                if c == 0:
                    result[i][j] = zero
        
    return result

def to_latex_system(matrix, repr_coef_func=None, with_eq=False):
    if repr_coef_func is None:
        repr_coef_func = str
    str_row_list = []
    for ri,row in enumerate(matrix):
        (nz_array,) = np.nonzero(row)
        str_row = [
            ("%s "%repr_coef_func(row[i]) if row[i] != 1 else "")
            +"\:" + " P_{%d}"%(i+1)
            for i in nz_array ]
        str_row = " &+ ".join(str_row)        
        str_row_list.append(str_row)

    if with_eq:
        max_coef = max(str_row.count("&") for str_row in str_row_list)
        str_row_list = [( str_row
                          + (max_coef - str_row.count("&")) * "& "
                          + " & = Q_{%d}" % (i+1))
                       for i,str_row in enumerate(str_row_list) ]

    r  = r"\begin{cases} \begin{aligned} "
    r += (r"\\"+"\n").join(str_row_list+[""])
    r += r"\end{aligned} \end{cases}"
    return r

#---------------------------------------------------------------------------

np.random.seed(1)

spec_list = [
    (2, 1, []),
    (3, 1, []),
    (6, 1, []),    
    (4, 5, [7]),
    (4, 5, [7]),
    (5, 5, [7]),
    (6, 5, [7]),
    #(6, 6, [7])
]

def repr_gf16(c):
    return r"{\color{blue}{"+str(swif.as_gf16_int(swif.GF256Elem(c)))+"}}"

def map_gf256_to_gf16(c):
    return swif.as_gf16_int(swif.GF256Elem(c))

#possible_coefs = [ swif.make_gf4(i)._value for i in range(1,4) ]
possible_coefs = [ swif.make_gf16(i)._value for i in range(1,16) ]
repr_coef = repr_gf16

symbol_list = [ make_random_symbol(start, nb_coef, zero_list, possible_coefs)
                for (start, nb_coef, zero_list) in spec_list ]

M = swif.to_matrix(symbol_list)

#--------------------------------------------------

symbols = swif.FullSymbolSet()
for symbol in symbol_list:
    symbols.add_with_elimination(symbol)

#--------------------------------------------------
    
from IPython.display import display, Math, Latex

str_system = to_latex_system(M, repr_coef, True)
display(Math(str_system))

map_to_gf16_vect = np.vectorize(map_gf256_to_gf16)
M_gf16 = map_to_gf16_vect(M)
str_matrix = basic_to_str_matrix(M_gf16)
latex_matrix = to_latex_matrix(str_matrix)
display(Math(latex_matrix))

# Note: if you take coefs subfield GF(16) of GF(256), the RREF will stay in
# this subfield.
rref_M = symbols.get_matrix() 
rref_M_gf16 = map_to_gf16_vect(rref_M)
latex_rref_matrix = to_latex_matrix(to_str_matrix(rref_M_gf16))
display(Math(latex_rref_matrix))


rev_M = M[:,::-1].copy()
ncol = len(M[0])
rref_M_lt = swif.compute_rref(rev_M)[::-1,::-1]
rref_M_lt_gf16 = map_to_gf16_vect(rref_M_lt)
latex_rref_lt_matrix = to_latex_matrix(to_str_matrix(rref_M_lt_gf16))
display(Math(latex_rref_lt_matrix))


if __name__ == "__main__":
    print("in jupyter you can do:")
    print('exec(open("test_matrix.py").read())')

#---------------------------------------------------------------------------

