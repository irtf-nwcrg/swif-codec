#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

from collections import namedtuple
import swif

SYMBOL_SIZE = 16
MAX_CODING_WINDOW_SIZE = 2
NB_WINDOW = 2

Packet = namedtuple("Packet", "first_id nb_id data key")

#---------------------------------------------------------------------------

def make_source_symbol(symbol_id):
    content = [0 if (i!=symbol_id) else 1 for i in range(SYMBOL_SIZE)]
    return bytes(content)

def try_encoder():
    packet_list = []
    encoder = swif.RlcEncoder(SYMBOL_SIZE, MAX_CODING_WINDOW_SIZE)
    source_id = 0
    for window_idx in range(MAX_CODING_WINDOW_SIZE*NB_WINDOW):
        for i in range(MAX_CODING_WINDOW_SIZE):
            symbol = make_source_symbol(source_id)
            encoder.add_source_symbol_to_coding_window(symbol, source_id)
            packet = Packet(first_id=source_id, nb_id=1, key=None, data=symbol)
            source_id += 1
            packet_list.append(packet)
        encoder.generate_coding_coefs(key=window_idx, add_param=0)
        repair_packet = encoder.build_repair_symbol()
        packet_list.append(repair_packet)

    for packet in packet_list:
        print(packet)

def try_decoder():
    packet_list = []
    decoder = swif.RlcDecoder(SYMBOL_SIZE, MAX_CODING_WINDOW_SIZE)
    source_id = 1
    symbol = make_source_symbol(source_id)
    decoder.add_source_symbol_to_coding_window(source_id)
    packet = Packet(first_id=source_id, nb_id=1, key=None, data=symbol)
    source_id += 1
    packet_list.append(packet)
    decoder.decoder_generate_coding_coefs(key=0, add_param=0)
    
#---------------------------------------------------------------------------

if __name__ == "__main__":
    #try_encoder()
    try_decoder()

#---------------------------------------------------------------------------
