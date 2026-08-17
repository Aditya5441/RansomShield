/**
 * @file    bm_crc_pka.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"

/**
 * @brief   TODO: describe what bm_crc_init() does
 */
void bm_crc_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
    CRC->CR = CRC_CR_RESET;
    CRC->CR = 0U;
    CRC->POL = 0x04C11DB7U;
    CRC->INIT = 0xFFFFFFFFU;
}

/**
 * @brief   TODO: describe what bm_pka_init() does
 */
void bm_pka_init(void)
{
    RCC->AHB3ENR |= RCC_AHB3ENR_PKAEN;
    PKA->CR = PKA_CR_EN;
}
