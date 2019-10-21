
import swif
from swif import GF256Elem

def test_simple():
    a = GF256Elem(1)
    b = GF256Elem(2)

    print(a+a, b+b, a+b, a-b, a-a)
    print(b.inverse())
    print(b*b.inverse(), b/(b*b))

test_simple()

