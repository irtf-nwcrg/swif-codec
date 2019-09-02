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
    symbol1.dump()
    symbol2.dump()
    #print(symbol_result.dump())
    print(symbol_result._add_base(symbol1,symbol2))
    symbol_result.dump()
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
    symbol3 = symbol1.add(symbol2)        
    print(symbol3)
    assert symbol3.get_coefs() == (6, b'\x01\x00\x00\x01\x01')
    assert symbol3.get_data() == b'\x00\x1b\x00\x00yo'
    print(symbol3.get_coefs())
    print(symbol3.get_data())
#---------------------------------------------------------------------------

def try_full_symbol_scale():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    coef = 2
    print(symbol1._scale(coef))
    symbol1.dump()
    symbol1.get_coefs()
    assert list(bytearray(symbol1.get_coefs()[1]))== [2,0,0,2]
    assert symbol1.get_data() == bytearray([2*x for x in b"azeu"])



#---------------------------------------------------------------------------

def try_full_symbol_scale_inverted():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    coef = 5
    print(symbol1._scale_inv(coef))

#---------------------------------------------------------------------------

def try_alloc_add_set():
    set1 = swif.FullSymbolSet().alloc_set()
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        6, bytes([1,0,0,1,0,0]), b"azeu")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        7, bytes([0,0,0,0]), b"")
    symbol3 = swif.FullSymbol().from_coefs_and_symbol(
        3, bytes([1,0,1,0]), b"aaeuyo")
    symbol4 = swif.FullSymbol().from_coefs_and_symbol(
        7, bytes([9,0,1,0]), b"aaeuyo")
    set1.set_add(symbol1)
    set1.set_add(symbol2)
    set1.set_add(symbol3)
    set1.set_add(symbol4)
    set1.dump()
    print(set1)
#---------------------------------------------------------------------------
def try_alloc_add_set_expl2():
    set2 = swif.FullSymbolSet().alloc_set()
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        9, bytes([1,0,0,0]), b"dddd")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        10, bytes([1,0,0,0,3,5]), b"aaaa")
    symbol3 = swif.FullSymbol().from_coefs_and_symbol(
        100, bytes([0,0,0,8,1,7]), b"bbbb")
    symbol4 = swif.FullSymbol().from_coefs_and_symbol(
        100, bytes([0,1,0,0,4,2]), b"cccc")
    
    set2.set_add(symbol1)
    set2.set_add(symbol2)
    set2.set_add(symbol3)
    set2.set_add(symbol4)
    set2.dump()
    print(set2.get_pivot(104))
    print(set2.get_pivot(103))
#---------------------------------------------------------------------------
def try_remove_each_pivot():

    set2 = swif.FullSymbolSet().alloc_set()
    P1 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,3,0,0,0,0]), b"azeu")
    P2 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([0,5,0,0,0,0]), b"aaeuyo")
    Q1 = P1.add(P2)  
    Q2 = P1.add(P2._scale(2))
    set2.add_with_elimination(Q1)
    set2.add_with_elimination(Q2)
    set2.dump()
    
    #new_symbol.dump()
#---------------------------------------------------------------------------
def try_elimination():
    import random
    random.seed(1)
    set2 = swif.FullSymbolSet().alloc_set()
    N = 10
    packet_list = []
    for i in range(N):
        header = [0]  * (N+2)
        header[i+1] = 1
        P = swif.FullSymbol().from_coefs_and_symbol(
            123, bytes(header), bytes([((i+j)**2)&0xff for j in range(10)]))
        packet_list.append(P)
        P.dump()

    symbol_list = []
    for i in range(N):
        Q = swif.FullSymbol().from_coefs_and_symbol(1, bytes([0]), b"")
        #Q = packet_list[0].clone()
        for j in range(N):
            P = packet_list[j].clone()
            Q = Q.add(P._scale(random.randint(0, 255)))
        Q.dump()
        symbol_list.append(Q)

    for i in range(N):
        set2.add_with_elimination(symbol_list[i])
    set2.dump()
#---------------------------------------------------------------------------
def test_full_symbol_get_coef():
    symbol = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([4,3,2,1,10]), b"azerty")
    print(symbol.get_coef(5))
#---------------------------------------------------------------------------
def test_add_as_pivot():
    set1 = swif.FullSymbolSet().alloc_set()
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,1,1,1]), b"azerty")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([0,0,1,2]), b"querty")
    new_symbol = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,1,2,0]), b"test")
    set1.set_add(symbol1)
    set1.set_add(symbol2)
    set1.dump()
    set1.add_as_pivot(new_symbol)
    set1.dump()
#---------------------------------------------------------------------------
def test_add_as_pivot2():
    set1 = swif.FullSymbolSet().alloc_set()
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,0,0]), b"azerty")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([0,1,0]), b"querty")
    #new_symbol = swif.FullSymbol().from_coefs_and_symbol(
    #    1, bytes([1,1,2,0]), b"test")
    v1 = symbol1.clone()
    v2 = symbol1.add(symbol2)

    #set1.set_add(v1)
    #set1.set_add(v2)
    #set1.dump()
    set1.add_with_elimination(v1)
    set1.add_with_elimination(v2)

    set1.dump()
#---------------------------------------------------------------------------

def test_add_with_elimination():
    set1 = swif.FullSymbolSet().alloc_set()
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,1,1,1]), b"azerty")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([0,0,1,2]), b"querty")
    new_symbol = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,1,2,0]), b"test")
    #set1.set_add(symbol1)
    #set1.set_add(symbol2)
    set1.dump()
    set1.add_with_elimination(new_symbol)
    set1.dump()
#---------------------------------------------------------------------------

def test_full_symbol_set_add_previous_id():
    set1 = swif.FullSymbolSet().alloc_set()
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        10, bytes([1,1,1,1]), b"azerty")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([0,0,1,2]), b"querty")
    symbol3 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,1,2,0]), b"test")
    set1.set_add(symbol1)
    set1.set_add(symbol2)
    set1.set_add(symbol3)
    set1.dump()
#---------------------------------------------------------------------------

def test_all():
    try_full_symbol_wrapper()
    test_full_symbol_wrapper()
    try_full_symbol_add_base()
    try_full_symbol_add()
    try_full_symbol_scale()
    try_full_symbol_scale_inverted()
    try_alloc_add_set()
    try_alloc_add_set_expl2()
    try_remove_each_pivot() 
    test_full_symbol_get_coef()
    test_add_as_pivot()
    test_add_as_pivot2()
    test_add_with_elimination()
    try_elimination()
    test_full_symbol_set_add_previous_id

if __name__ == "__main__":
    #test_all()
    #try_elimination()
    test_full_symbol_set_add_previous_id()

#---------------------------------------------------------------------------
