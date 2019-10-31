
import swif

def test_create():
    p1 = swif.Symbol()
    assert p1.size == 0

    p1.alloc(100)
    assert p1.size == 100
    data = p1.get_data()
    assert len(data) == 100
    assert data == 100*b"\x00"

    p1.alloc(50)
    data = p1.get_data()
    assert len(data) == 50
    assert data == 50*b"\x00"

    p1.dealloc()
    assert p1.size == 0

    p1 = swif.Symbol(b"ABCD")
    p1.set_data(b"ABCD")

def test_add():
    p1 = swif.Symbol(b"ABCD73")
    p2 = p1.add(swif.Symbol(b"ABEF12"))
    p3 = p2.add(swif.Symbol(b"ABEF12"))    
    assert (p1.get_data() == p3.get_data())
    assert (p2.get_data() != p1.get_data())

def test_sub():
    p1 = swif.Symbol(b"ABCD73")
    p2 = p1.sub(swif.Symbol(b"ABEF12"))
    p3 = p1.add(swif.Symbol(b"ABEF12"))    
    assert (p2.get_data() == p3.get_data()) # because GF(2^k)

def test_mul():
    p0 = swif.Symbol(bytes([i for i in range(256)]))
    p1 = p0.copy()
    for i in range(255):
        p1 = 5*p1
    assert p1.get_data() == p0.get_data()

def test_div():
    p0 = swif.Symbol(bytes([i for i in range(0,256)]))
    for i in range(2,255+1):
        p1 = p0/i
        p2 = p1*i
        assert p1.get_data() != p2.get_data()
        assert p2.get_data() == p0.get_data()

test_div()
    
if 0:
    print(swif.Symbol(b"0123456789") + swif.Symbol(b"9876543210"))

    test_create()

    p = swif.Symbol(b"\x01\x01")
    for i in range(256):
        p = 4*p
        data = p.get_data()
        assert data[0] == data[1]
        print(p.get_data()[0], sep=" ", end=" ")
