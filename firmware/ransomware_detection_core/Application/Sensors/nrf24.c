/**
 * @file    nrf24.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "nrf24.h"
#include "bm.h"

#define NRF_CS_PIN     (1U << 6)

#define NRF_CSN_LOW()  bm_gpio_write(GPIOB, NRF_CS_PIN, false)
#define NRF_CSN_HIGH() bm_gpio_write(GPIOB, NRF_CS_PIN, true)

static void NRF24_WriteReg(uint8_t reg, uint8_t value);
static uint8_t NRF24_ReadReg(uint8_t reg);

#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define NOP        0xFF

/**
 * @brief   TODO: describe what NRF24_Init() does
 */
void NRF24_Init(void)
{
    NRF24_WriteReg(0x00, 0x0B);
    NRF24_WriteReg(0x01, 0x3F);
    NRF24_WriteReg(0x02, 0x03);
    NRF24_WriteReg(0x04, 0x04);
    NRF24_WriteReg(0x05, 76);
    NRF24_WriteReg(0x06, 0x0F);
}

/**
 * @brief   TODO: describe what NRF24_ReadReg() does
 * @param   reg  TODO: describe parameter
 * @retval  TODO: describe return value
 */
uint8_t NRF24_ReadReg(uint8_t reg)
{
    uint8_t tx[2] = {R_REGISTER | reg, NOP};
    uint8_t rx[2];

    NRF_CSN_LOW();
    bm_spi_transmit_receive(SPI1, tx, rx, 2, 100);
    NRF_CSN_HIGH();

    return rx[1];
}

/**
 * @brief   TODO: describe what NRF24_WriteReg() does
 * @param   reg  TODO: describe parameter
 * @param   value  TODO: describe parameter
 */
void NRF24_WriteReg(uint8_t reg, uint8_t value)
{
    uint8_t tx[2] = {W_REGISTER | reg, value};

    NRF_CSN_LOW();
    bm_spi_transmit(SPI1, tx, 2, 100);
    NRF_CSN_HIGH();
}

/**
 * @brief   TODO: describe what NRF24_WriteBuf() does
 * @param   reg  TODO: describe parameter
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 */
void NRF24_WriteBuf(uint8_t reg, uint8_t *data, uint8_t len)
{
    NRF_CSN_LOW();
    uint8_t cmd = W_REGISTER | reg;
    bm_spi_transmit(SPI1, &cmd, 1, 100);
    bm_spi_transmit(SPI1, data, len, 100);
    NRF_CSN_HIGH();
}

/**
 * @brief   TODO: describe what NRF24_ReadBuf() does
 * @param   reg  TODO: describe parameter
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 */
void NRF24_ReadBuf(uint8_t reg, uint8_t *data, uint8_t len)
{
    NRF_CSN_LOW();
    uint8_t cmd = R_REGISTER | reg;
    bm_spi_transmit(SPI1, &cmd, 1, 100);
    bm_spi_receive(SPI1, data, len, 100);
    NRF_CSN_HIGH();
}

/**
 * @brief   TODO: describe what NRF24_GetRSSI() does
 * @retval  TODO: describe return value
 */
int8_t NRF24_GetRSSI(void)
{
    uint8_t rpd = NRF24_ReadReg(0x09);
    return (rpd & 0x01) ? -64 : -100;
}
