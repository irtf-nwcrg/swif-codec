/**
 * SWiF Codec: an open-source sliding window FEC codec in C
 * https://github.com/irtf-nwcrg/swif-codec
 */

#ifndef __SWIF_SYMBOL_H__
#define __SWIF_SYMBOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
    
/*---------------------------------------------------------------------------*/

/**
 * @brief Take a symbol and add another symbol multiplied by a 
 *        coefficient, e.g. performs the equivalent of: p1 += coef * p2
 * @param[in,out] p1     First symbol (to which coef*p2 will be added)
 * @param[in]     coef2  Coefficient by which the second packet is multiplied
 * @param[in]     p2     Second symbol
 */
void symbol_add_scaled
(void *symbol1, uint8_t coef, void *symbol2, uint32_t symbol_size);


/**
 * @brief Take a symbol and add another symbol to it, e.g. performs the equivalent of: result = p1 + p2
 * @param[in,out] p1     First symbol (to which p2 will be added)
 * @param[in]     p2     Second symbol
 */
void symbol_add(void *symbol1, void *symbol2, uint32_t symbol_size, uint8_t* result);


/* Substration */
void symbol_sub(void *symbol1, void *symbol2, uint32_t symbol_size, uint8_t* result);


/**
 * @brief Take a symbol and multiply it by another symbol, e.g. performs the equivalent of: result = p1 * p2
 * @param[in] p1     First symbol (to which coeff will be multiplied)
 * @param[in]     coeff      Coefficient by which the second packet is multiplied
 */
void symbol_mul(void *symbol1, uint8_t coeff, uint32_t symbol_size, uint8_t* result);


/**
 * @brief Take a symbol and divide it by another symbol, e.g. performs the equivalent of: result = p1 / p2
 * @param[in] p1     First symbol (to which coeff will be divided)
 * @param[in]     coeff      Coefficient by which the second packet is divided
 */
void symbol_div(void *symbol1, uint32_t symbol_size, uint8_t coeff, uint8_t* result);


/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __SWIF_SYMBOL_H__ */
