#---------------------------------------------------------------------------
# Test of GF256 code
#---------------------------------------------------------------------------

import swif
from swif import GF256Elem

def test_simple():
    a = GF256Elem(1)
    b = GF256Elem(2)

    print(a+a, b+b, a+b, a-b, a-a)
    print(b.inverse())
    print(b*b.inverse(), b/(b*b))


def test_gf256():
    """check basic field identities: 
    * x.1 == x
    * x.0 == 0
    * x.y == y.x
    * x.inv(x) == 1
    * x.(y+z) == x.y + x.z
    * x.(y.z) = (x.y).z
    """
    n = 256
    def mul(a,b): return (GF256Elem(a)*GF256Elem(b)).value
    def inv(a): return GF256Elem(a).inverse().value
    def add(a,b): return (GF256Elem(a)+GF256Elem(b)).value

    # x.1 == x and x.0 == 0
    for x in range(n):
        x_times_1 = mul(x,1)
        x_times_0 = mul(x,0)
        assert x_times_1 == x
        assert x_times_0 == 0

    # x.y == y.x
    for x in range(n):
        for y in range(n):
            x_times_y = mul(x,y)
            y_times_x = mul(y,x)
            assert x_times_y == y_times_x

    # x.inv(x) == 1
    for x in range(1,n):
        x_inv = inv(x)
        x_times_inv = mul(x, x_inv)
        assert x_times_inv == 1

    # x.(y.z) = (x.y).z
    for x in range(n):
        for y in range(n):
            for z in range(n):
                v1 = mul(x, mul(y,z))
                v2 = mul(mul(x,y), z)
                assert v1 == v2

    # x.(y+z) == x.y + x.z
    for x in range(n):
        for y in range(n):
            for z in range(n):
                v1 = mul(x, add(y,z))
                v2 = add(mul(x,y), mul(x,z))
                assert v1 == v2

#---------------------------------------------------------------------------
                
test_simple()
test_gf256()

#---------------------------------------------------------------------------
