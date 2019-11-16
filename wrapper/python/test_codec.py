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
    #source_id += 1
    packet_list.append(packet)
    decoder.decoder_generate_coding_coefs(key=0, add_param=0)
    buff = bytes([1,0,3,1])
    decoder.decode_with_new_repair_symbol(buff,source_id)

def test_with_2_packets():
    symbol1 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([1,0,0]), b"azerty")
    symbol2 = swif.FullSymbol().from_coefs_and_symbol(
        1, bytes([0,1,0]), b"querty")
    print("dump 2 ")
    symbol2.dump()
    src1 = symbol1.clone()
    src1._scale(0x27)
    src2 = symbol2.clone()
    src2._scale(0x2a)
    repair = src1.add(src2)

    decoder = swif.RlcDecoder(SYMBOL_SIZE, MAX_CODING_WINDOW_SIZE)
    decoder.decoder_reset_coding_window()
    decoder.decode_with_new_source_symbol(symbol1.get_data(), symbol1.get_min_symbol_id())
    #decoder.swif_decoder_decode_with_new_source_symbol(symbol2)
    decoder.add_source_symbol_to_coding_window(1)
    decoder.add_source_symbol_to_coding_window(2)
    decoder.decoder_generate_coding_coefs(key=0, add_param=0)
    decoder.decode_with_new_repair_symbol(repair.get_data(),repair.get_min_symbol_id())    
#---------------------------------------------------------------------------

if __name__ == "__main__":
    #try_encoder()
    test_with_2_packets()

#---------------------------------------------------------------------------
