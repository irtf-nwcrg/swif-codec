
/**
 * @brief       For now this is an hackish adaptation on top of liblc,
 *              will be replaced by a proper implementation later.
 */


#include "coded-packet.h"

#define LIBLC_GF256 3
#define COEF_PER_HEADER ((COEF_HEADER_SIZE*BITS_PER_BYTE)>>LIBLC_GF256)

#include "swif_full_symbol.h"

/*---------------------------------------------------------------------------*/

struct s_swif_full_symbol_t {
    coded_packet_t* coded_packet;
};

/*---------------------------------------------------------------------------*/

struct s_swif_full_symbol_set_t {
    /* XXX:TODO */
};

/*---------------------------------------------------------------------------*/

/**
 * @brief Add a full_symbol to a packet set.
 * 
 * Gaussian elimination can occur.
 * Teturn the pivot associated to the new full_symbol 
 * or SWIF_FULL_SYMBOL_ID_NONE if dependent (e.g. redundant) packet
 */

uint32_t swif_full_symbol_set_add
(swif_full_symbol_set_t* set, swif_full_symbol_t* full_symbol)
{
    
}

/*---------------------------------------------------------------------------*/

/**
 * @brief Create a full_symbol from a raw packet (a set of bytes)
 *        and initialize it with content '0'
 */
swif_full_symbol_t *full_symbol_alloc
(uint32_t symbol_id_size, uint32_t symbol_size)
{
    assert(symbol_id_size <= COEF_HEADER_SIZE);
    assert(symbol_size <= CODED_PACKET_SIZE);
    swif_full_symbol_t *result
        = (swif_full_symbol_t *)calloc(1, sizeof(swif_full_symbol_t));
    if (result == NULL) {
        return NULL;
    }
  
    coded_packet_t *coded_packet
        = (coded_packet_t *)calloc(1, sizeof(coded_packet_t));
    if (coded_packet == NULL) {
        free(result);
        return NULL;
    }

    result->coded_packet = coded_packet;
    coded_packet_init(result->coded_packet, LIBLC_GF256);
    return result;
}


/**
 * @brief Create a full_symbol from a raw packet (a set of bytes)
 *        and initialize it with content '0'
 */
swif_full_symbol_t *full_symbol_create_from_source
(uint32_t symbol_id, uint8_t *symbol_data, uint32_t symbol_size)
{
    swif_full_symbol_t *result = full_symbol_alloc(1, symbol_size);
    if (result == NULL) {
        return NULL;
    }
    coded_packet_set_coef(result->coded_packet, symbol_id, 1);
    return result;
}

swif_full_symbol_t *full_symbol_create
(uint8_t* symbol_coef_table, uint32_t min_symbol_id, uint32_t nb_symbol_id,
 uint8_t* symbol_data, uint32_t symbol_size)
{
    swif_full_symbol_t *result = full_symbol_alloc(nb_symbol_id, symbol_size);
    if (result == NULL) {
        return NULL;
    }
    for (uint32_t i=0; i<nb_symbol_id; i++) {
        /* Note: assumption of GF256 in next line -> 1 coef 1 byte: */ 
        uint8_t coef = symbol_coef_table[i]; 
        coded_packet_set_coef(result->coded_packet, i+min_symbol_id, coef);
    }
    memcpy(coded_packet_data(result->coded_packet), symbol_data, symbol_size);
    return result;
}

/**
 * @brief Release a full_symbol
 */
void full_symbol_free(swif_full_symbol_t* full_symbol)
{
    assert(full_symbol != NULL);
    assert(full_symbol->coded_packet != NULL);
    free(full_symbol->coded_packet);
    full_symbol->coded_packet = NULL;
    free(full_symbol);
}

/**
 * @brief Create a new (unlinked) copy of a full_symbol
 */
swif_full_symbol_t *full_symbol_clone(swif_full_symbol_t* swif_full_symbol)
{
    /* XXX: TODO */
}

/**
 * @brief get the size of the data
 */
uint32_t full_symbol_get_size(swif_full_symbol_t *full_symbol)
{
    /* XXX: TODO */
}


/**
 * @brief get the minimum source index that appears in the symbol
 *        returns SWIF_FULL_SYMBOL_ID_NONE if there is none (e.g. symbol is 0)
 */
uint32_t full_symbol_get_min_symbol_id(swif_full_symbol_t *full_symbol)
{
    /* XXX: TODO */
}

/**
 * @brief get the maximum source index that appears in the symbol
 *        returns SWIF_FULL_SYMBOL_ID_NONE if there is none (e.g. symbol is 0)
 */
uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol)
{
    /* XXX: TODO */
}

/**
 * @brief get the coefficient corresponding to the specified symbol identifier
 */
uint8_t full_symbol_get_coef
(swif_full_symbol_t *full_symbol, uint32_t symbol_id)
{
    /* XXX: TODO */
}

/*---------------------------------------------------------------------------*/
