/**
 * @file    si4432.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "si4432.h"
#include "bm.h"

#define SI_CS_PIN    (1U << 7)

#define SI_CS_LOW()  bm_gpio_write(GPIOB, SI_CS_PIN, false)
#define SI_CS_HIGH() bm_gpio_write(GPIOB, SI_CS_PIN, true)

static void SI4432_WriteReg(uint8_t reg, uint8_t value);
static uint8_t SI4432_ReadReg(uint8_t reg);

/**
 * @brief   TODO: describe what SI4432_Init() does
 */
void SI4432_Init(void)
{
    SI4432_WriteReg(0x07, 0x01);
    SI4432_WriteReg(0x09, 0x7F);
    SI4432_WriteReg(0x6D, 0x1F);
}

/**
 * @brief   TODO: describe what SI4432_ReadReg() does
 * @param   reg  TODO: describe parameter
 * @retval  TODO: describe return value
 */
uint8_t SI4432_ReadReg(uint8_t reg)
{
    uint8_t tx[2] = {reg & 0x7F, 0};
    uint8_t rx[2];

    SI_CS_LOW();
    bm_spi_transmit_receive(SPI1, tx, rx, 2, 100);
    SI_CS_HIGH();

    return rx[1];
}

/**
 * @brief   TODO: describe what SI4432_WriteReg() does
 * @param   reg  TODO: describe parameter
 * @param   val  TODO: describe parameter
 */
void SI4432_WriteReg(uint8_t reg, uint8_t val)
{
    uint8_t tx[2] = {reg | 0x80, val};

    SI_CS_LOW();
    bm_spi_transmit(SPI1, tx, 2, 100);
    SI_CS_HIGH();
}

/**
 * @brief   TODO: describe what SI4432_GetRSSI() does
 * @retval  TODO: describe return value
 */
int8_t SI4432_GetRSSI(void)
{
    uint8_t raw = SI4432_ReadReg(0x26);
    return (raw / 2) - 120;
}
