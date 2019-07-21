#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

from collections import namedtuple
import swif

SYMBOL_SIZE = 16
MAX_CODING_WINDOW_SIZE = 4

Packet = namedtuple("Packet", "first_id nb_id data key")

#---------------------------------------------------------------------------

def make_source_symbol(symbol_id):
    content = [0 if (i!=symbol_id) else 1 for i in range(SYMBOL_SIZE)]
    return bytes(content)

def try_encoder():
    packet_list = []
    encoder = swif.RlcEncoder(SYMBOL_SIZE, MAX_CODING_WINDOW_SIZE)
    for i in range(MAX_CODING_WINDOW_SIZE):
        symbol = make_source_symbol(i)
        encoder.add_source_symbol_to_coding_window(symbol, i)
        packet = Packet(first_id=i, nb_id=1, key=None, data = symbol)
        print(packet)
        packet_list.append(packet)

    (encoder.build_repair_symbol())
    
    encoder.generate_coding_coefs(key=2,add_param=1)

    packet_list.append()
    
    
#---------------------------------------------------------------------------

if __name__ == "__main__":
    try_encoder()

#---------------------------------------------------------------------------
