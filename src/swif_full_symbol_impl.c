
/**
 * @brief       For now this is an hackish adaptation on top of liblc,
 *              will be replaced by a proper implementation later.
 */

#include <stdio.h>
#include <stdlib.h>

#include "swif_full_symbol.h"

/*---------------------------------------------------------------------------*/


struct s_swif_full_symbol_t {
    //coded_packet_t* coded_packet;
    //uint32_t symbol_size;
    uint8_t* coef;
    symbol_id_t first_id; // coef[0] is the coefficient of first symbol_id
    symbol_id_t last_id; // included

    symbol_id_t first_nonzero_id;
    symbol_id_t last_nonzero_id;    
    
    uint8_t* data;
    uint32_t data_size;
};

/*---------------------------------------------------------------------------*/

struct s_swif_full_symbol_set_t {
    
};

/*---------------------------------------------------------------------------*/


/**
 * @brief Create a full_symbol set, that will be used to do gaussian elimination
 */
swif_full_symbol_set_t *full_symbol_set_alloc()
{
    return NULL;
}

/**
 * @brief Free a full_symbol set
 */
void full_symbol_set_free(swif_full_symbol_set_t *set)
{
    //memset(&set->packet_set, 0, sizeof(packet_set_t));
    //free(set);
}


/**
 * @brief Add a full_symbol to a packet set.
 * 
 * Gaussian elimination can occur.
 * Return the pivot associated to the new full_symbol 
 * or SYMBOL_ID_NONE if dependent (e.g. redundant) packet
 * 
 * The full_symbol is not freed and also reference is not captured.
 */
uint32_t swif_full_symbol_set_add
(swif_full_symbol_set_t* set, swif_full_symbol_t* full_symbol)
{
    return SYMBOL_ID_NONE;
}

/*---------------------------------------------------------------------------*/

/**
 * @brief Create a full_symbol from a raw packet (a set of bytes)
 *        and initialize it with content '0'
 */
swif_full_symbol_t *full_symbol_alloc
(symbol_id_t first_symbol_id, symbol_id_t last_symbol_id, uint32_t symbol_size)
{
    return NULL;
}


/**
 * @brief Create a full_symbol from a raw packet (a set of bytes)
 *        and initialize it with content '0'
 */
swif_full_symbol_t *full_symbol_create_from_source
(uint32_t symbol_id, uint8_t *symbol_data, uint32_t symbol_size)
{
    return NULL;
}

swif_full_symbol_t *full_symbol_create
(uint8_t* symbol_coef_table, uint32_t min_symbol_id, uint32_t nb_symbol_id,
 uint8_t* symbol_data, uint32_t symbol_size)
{
    return NULL;
}

/**
 * @brief Release a full_symbol
 */
void full_symbol_free(swif_full_symbol_t* full_symbol)
{
    // TODO
}

/**
 * @brief Create a new (unlinked) copy of a full_symbol
 */
swif_full_symbol_t *full_symbol_clone(swif_full_symbol_t* full_symbol)
{
    return NULL;
}

/**
 * @brief get the size of the data
 */
uint32_t full_symbol_get_size(swif_full_symbol_t *full_symbol)
{ return 0; // TODO
}

/**
 * @brief get the minimum source index that appears in the symbol
 *        SYMBOL_ID_NONE if there is none (e.g. symbol is 0)
 */
uint32_t full_symbol_get_min_symbol_id(swif_full_symbol_t *full_symbol)
{
    return SYMBOL_ID_NONE;        
}

/**
 * @brief get the maximum source index that appears in the symbol
 *        returns SYMBOL_ID_NONE if there is none (e.g. symbol is 0)
 */
uint32_t full_symbol_get_max_symbol_id(swif_full_symbol_t *full_symbol)
{
    return SYMBOL_ID_NONE;
}

/**
 * @brief get the coefficient corresponding to the specified symbol identifier
 */
uint8_t full_symbol_get_coef
(swif_full_symbol_t *full_symbol, uint32_t symbol_id)
{
    return 0; //TODO
}

/**
 * @brief get the symbol 'data'. result_data should be a pointer to a
 *        a block of memory of full_symbol_get_size(full_symbol)
 */
void full_symbol_get_data
(swif_full_symbol_t *full_symbol, uint8_t *result_data)
{
    // TODO
}

void full_symbol_dump(swif_full_symbol_t *full_symbol, FILE *out)
{
    //fprintf(out, "{'size':%u, 'internal':", full_symbol->symbol_size);
    //coded_packet_pywrite(out, full_symbol->coded_packet);
    //fprintf(out, "}");
    // TODO
}

/*---------------------------------------------------------------------------*/

/**
 * @brief Take a symbol and add another symbol multiplied by a 
 *        coefficient, e.g. performs the equivalent of: p1 += coef * p2
 * @param[in,out] p1     First symbol (to which coef*p2 will be added)
 * @param[in]     coef2  Coefficient by which the second packet is multiplied
 * @param[in]     p2     Second symbol
 */
void full_symbol_add_scaled
(void *symbol1, uint8_t coef, void *symbol2, uint32_t symbol_size);


/**
 * @brief Take a symbol and add another symbol to it, e.g. performs the equivalent of: p3 = p1 + p2
 * @param[in] p1     First symbol (to which p2 will be added)
 * @param[in] p2     Second symbol
 * @param[in] p3     result XXX
 */
void full_symbol_add_base(
swif_full_symbol_t *symbol1, swif_full_symbol_t *symbol2, swif_full_symbol_t *symbol_result);
// assert (symbol1->data != NULL && symbol2->data != NULL .... )
// assert (symbol_result->data_size >= symbol2->data_size);
// assert (symbol2->first_nonzero_iid -> inclus dans l'entete symbol_result);
// assert (symbol2->last_nonzero_id -> inclus dans l'entete symbol_result);


swif_full_symbol_t* full_symbol_add
(swif_full_symbol_t *symbol1, swif_full_symbol_t *symbol2)
{
    return NULL;
}


static inline bool full_symbol_includes_id(swif_full_symbol_t* symbol,
                                           symbol_id_t id)
{ return false; }




/* Substration */
//void full_symbol_sub(void *symbol1, void *symbol2, uint32_t symbol_size, uint8_t* result);


/**
 * @brief Take a symbol and multiply it by another symbol, e.g. performs the equivalent of: result = p1 * p2
 * @param[in] p1     First symbol (to which coeff will be multiplied)
 * @param[in]     coeff      Coefficient by which the second packet is multiplied
 */
//void full_symbol_mul(void *symbol1, uint8_t coeff, uint32_t symbol_size, uint8_t* result);


/**
 * @brief Take a symbol and divide it by another symbol, e.g. performs the equivalent of: result = p1 / p2
 * @param[in] p1     First symbol (to which coeff will be divided)
 * @param[in]     coeff      Coefficient by which the second packet is divided
 */
//void full_symbol_div(void *symbol1, uint32_t symbol_size, uint8_t coeff, uint8_t* result);

/*---------------------------------------------------------------------------*/
