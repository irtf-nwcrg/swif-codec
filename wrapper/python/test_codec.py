#---------------------------------------------------------------------------
# swif-codec - 2019
#---------------------------------------------------------------------------

import swif

SYMBOL_SIZE = 16
MAX_CODING_WINDOW_SIZE = 4

#---------------------------------------------------------------------------

def make_source_symbol(symbol_id):
    content = [0 if (i!=symbol_id) else 1 for i in range(SYMBOL_SIZE)]
    return bytes(content)

def try_encoder():
    encoder = swif.RlcEncoder(SYMBOL_SIZE, MAX_CODING_WINDOW_SIZE)
    for i in range(MAX_CODING_WINDOW_SIZE):
        symbol = make_source_symbol(i)
        encoder.add_source_symbol_to_coding_window(symbol, i)
        print(symbol)
    encoder.generate_coding_coefs(key=2,add_param=0)
    print(encoder.build_repair_symbol())
    
#---------------------------------------------------------------------------

if __name__ == "__main__":
    try_encoder()

#---------------------------------------------------------------------------
