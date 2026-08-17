/**
 * @file    w5500.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "w5500.h"
#include "bm.h"

#define W5500_CS_PIN   (1U << 8)

#define W5500_CS_LOW()  bm_gpio_write(GPIOB, W5500_CS_PIN, false)
#define W5500_CS_HIGH() bm_gpio_write(GPIOB, W5500_CS_PIN, true)

/**
 * @brief   TODO: describe what W5500_Init() does
 */
void W5500_Init(void)
{
}

/**
 * @brief   TODO: describe what W5500_Read() does
 * @param   addr  TODO: describe parameter
 * @retval  TODO: describe return value
 */
uint8_t W5500_Read(uint16_t addr)
{
    uint8_t tx[3];
    uint8_t rx = 0;

    tx[0] = (addr >> 8) & 0xFF;
    tx[1] = addr & 0xFF;
    tx[2] = 0x00;

    W5500_CS_LOW();
    bm_spi_transmit(SPI2, tx, 3, 100);
    bm_spi_receive(SPI2, &rx, 1, 100);
    W5500_CS_HIGH();

    return rx;
}

/**
 * @brief   TODO: describe what W5500_Write() does
 * @param   addr  TODO: describe parameter
 * @param   data  TODO: describe parameter
 */
void W5500_Write(uint16_t addr, uint8_t data)
{
    uint8_t tx[4];

    tx[0] = (addr >> 8) & 0xFF;
    tx[1] = addr & 0xFF;
    tx[2] = 0x04;
    tx[3] = data;

    W5500_CS_LOW();
    bm_spi_transmit(SPI2, tx, 4, 100);
    W5500_CS_HIGH();
}

/**
 * @brief   TODO: describe what W5500_GetTraffic() does
 * @retval  TODO: describe return value
 */
float W5500_GetTraffic(void)
{
    uint8_t rx = W5500_Read(0x0026);
    return rx * 0.1f;
}
