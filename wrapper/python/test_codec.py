#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

#import sys
import swif

SYMBOL_SIZE = 128
MAX_CODING_WINDOW_SIZE = 16

#---------------------------------------------------------------------------

def try_encoder():
    encoder = swif.RlcEncoder(SYMBOL_SIZE, MAX_CODING_WINDOW_SIZE)
    symbol = SYMBOL_SIZE * bytes([1])
    encoder.add_source_symbol_to_coding_window(symbol, 1)
    encoder.generate_coding_coefs(key=1,add_param=0)
    print(encoder.build_repair_symbol())

#---------------------------------------------------------------------------

if __name__ == "__main__":
    try_encoder()

#---------------------------------------------------------------------------
