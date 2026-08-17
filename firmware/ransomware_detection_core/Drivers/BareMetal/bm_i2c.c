/**
 * @file    bm_i2c.c
 * @brief   TODO: one-line description of this file's purpose
 */

#include "bm.h"
#include <stddef.h>

typedef struct {
    volatile bool done;
    volatile bool error;
    const uint8_t *tx;
    uint8_t *rx;
    uint16_t len;
    uint16_t idx;
} bm_i2c_xfer_t;

static bm_i2c_xfer_t s_i2c1_xfer;
static bm_i2c_xfer_t s_i2c3_xfer;

/**
 * @brief   TODO: describe what i2c_wait_job() does
 * @param   job  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
static bool i2c_wait_job(bm_i2c_xfer_t *job, uint32_t timeout_ms)
{
    uint32_t deadline = bm_tick_get() + timeout_ms;
    while (!job->done) {
        if (bm_tick_get() >= deadline) {
            job->done = true;
            return false;
        }
    }
    return !job->error;
}

/**
 * @brief   TODO: describe what i2c_disable_irqs() does
 * @param   i2c  TODO: describe parameter
 */
static void i2c_disable_irqs(I2C_TypeDef *i2c)
{
    i2c->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE);
}

/**
 * @brief   TODO: describe what i2c_clear_flags() does
 * @param   i2c  TODO: describe parameter
 */
static void i2c_clear_flags(I2C_TypeDef *i2c)
{
    i2c->ICR = I2C_ICR_NACKCF | I2C_ICR_BERRCF | I2C_ICR_ARLOCF
             | I2C_ICR_STOPCF | I2C_ICR_OVRCF;
}

/**
 * @brief   TODO: describe what i2c_ev_handler() does
 * @param   i2c  TODO: describe parameter
 * @param   job  TODO: describe parameter
 */
static void i2c_ev_handler(I2C_TypeDef *i2c, bm_i2c_xfer_t *job)
{
    uint32_t isr = i2c->ISR;

    if (isr & I2C_ISR_TXIS) {
        if (job->tx && job->idx < job->len) {
            i2c->TXDR = job->tx[job->idx++];
        }
    }

    if (isr & I2C_ISR_RXNE) {
        if (job->rx && job->idx < job->len) {
            job->rx[job->idx++] = (uint8_t)i2c->RXDR;
        }
    }

    if (isr & I2C_ISR_NACKF) {
        i2c->ICR = I2C_ICR_NACKCF;
        job->error = true;
        job->done = true;
        i2c_disable_irqs(i2c);
    }

    if (isr & I2C_ISR_STOPF) {
        i2c->ICR = I2C_ICR_STOPCF;
        job->done = true;
        i2c_disable_irqs(i2c);
    }
}

/**
 * @brief   TODO: describe what i2c_er_handler() does
 * @param   i2c  TODO: describe parameter
 * @param   job  TODO: describe parameter
 */
static void i2c_er_handler(I2C_TypeDef *i2c, bm_i2c_xfer_t *job)
{
    if (i2c->ISR & (I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR)) {
        job->error = true;
        job->done = true;
        i2c_clear_flags(i2c);
        i2c_disable_irqs(i2c);
    }
}

/**
 * @brief   TODO: describe what i2c1_gpio_init() does
 */
static void i2c1_gpio_init(void)
{
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_I2C1SEL) | RCC_CCIPR_I2C1SEL_0;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    GPIOB->MODER = (GPIOB->MODER & ~((3U << 16) | (3U << 18))) | (3U << 16) | (3U << 18);
    GPIOB->OTYPER |= (GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);
    GPIOB->AFR[1] = (GPIOB->AFR[1] & ~0xFFU) | 0x44U;
}

/**
 * @brief   TODO: describe what i2c3_gpio_init() does
 */
static void i2c3_gpio_init(void)
{
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_I2C3SEL) | RCC_CCIPR_I2C3SEL_0;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C3EN;

    GPIOC->MODER = (GPIOC->MODER & ~(3U << 0)) | (3U << 0);
    GPIOC->OTYPER |= GPIO_OTYPER_OT0;
    GPIOC->AFR[0] = (GPIOC->AFR[0] & ~0xFU) | 0x4U;

    GPIOB->MODER = (GPIOB->MODER & ~(3U << 28)) | (3U << 28);
    GPIOB->OTYPER |= GPIO_OTYPER_OT14;
    GPIOB->AFR[1] = (GPIOB->AFR[1] & ~(0xFU << 24)) | (0x4U << 24);
}

/**
 * @brief   TODO: describe what bm_i2c1_init() does
 */
void bm_i2c1_init(void)
{
    const uint32_t prio_group = 3U;

    i2c1_gpio_init();
    I2C1->CR1 = 0;
    I2C1->TIMINGR = 0x00200D12U;
    I2C1->CR1 = I2C_CR1_PE;

    NVIC_SetPriority(I2C1_EV_IRQn, NVIC_EncodePriority(prio_group, 14U, 0));
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_SetPriority(I2C1_ER_IRQn, NVIC_EncodePriority(prio_group, 14U, 0));
    NVIC_EnableIRQ(I2C1_ER_IRQn);
}

/**
 * @brief   TODO: describe what bm_i2c3_init() does
 */
void bm_i2c3_init(void)
{
    const uint32_t prio_group = 3U;

    i2c3_gpio_init();
    I2C3->CR1 = 0;
    I2C3->TIMINGR = 0x00200D13U;
    I2C3->CR1 = I2C_CR1_PE;

    NVIC_SetPriority(I2C3_EV_IRQn, NVIC_EncodePriority(prio_group, 14U, 0));
    NVIC_EnableIRQ(I2C3_EV_IRQn);
    NVIC_SetPriority(I2C3_ER_IRQn, NVIC_EncodePriority(prio_group, 14U, 0));
    NVIC_EnableIRQ(I2C3_ER_IRQn);
}

/**
 * @brief   TODO: describe what bm_i2c1_ev_irq_handler() does
 */
void bm_i2c1_ev_irq_handler(void)
{
    i2c_ev_handler(I2C1, &s_i2c1_xfer);
}

/**
 * @brief   TODO: describe what bm_i2c1_er_irq_handler() does
 */
void bm_i2c1_er_irq_handler(void)
{
    i2c_er_handler(I2C1, &s_i2c1_xfer);
}

/**
 * @brief   TODO: describe what bm_i2c3_ev_irq_handler() does
 */
void bm_i2c3_ev_irq_handler(void)
{
    i2c_ev_handler(I2C3, &s_i2c3_xfer);
}

/**
 * @brief   TODO: describe what bm_i2c3_er_irq_handler() does
 */
void bm_i2c3_er_irq_handler(void)
{
    i2c_er_handler(I2C3, &s_i2c3_xfer);
}

/**
 * @brief   TODO: describe what bm_i2c_master_transmit() does
 * @param   i2c  TODO: describe parameter
 * @param   dev_addr_7bit  TODO: describe parameter
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_i2c_master_transmit(I2C_TypeDef *i2c, uint16_t dev_addr_7bit,
                            const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (!data || len == 0U) {
        return false;
    }

    bm_i2c_xfer_t *job = (i2c == I2C1) ? &s_i2c1_xfer : &s_i2c3_xfer;

    job->done = false;
    job->error = false;
    job->tx = data;
    job->rx = NULL;
    job->len = len;
    job->idx = 0U;

    i2c_clear_flags(i2c);
    i2c->CR2 = ((uint32_t)dev_addr_7bit << I2C_CR2_SADD_Pos)
             | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
             | I2C_CR2_AUTOEND
             | I2C_CR2_START;

    i2c->CR1 |= I2C_CR1_TXIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE;

    return i2c_wait_job(job, timeout_ms);
}

/**
 * @brief   TODO: describe what bm_i2c_master_receive() does
 * @param   i2c  TODO: describe parameter
 * @param   dev_addr_7bit  TODO: describe parameter
 * @param   data  TODO: describe parameter
 * @param   len  TODO: describe parameter
 * @param   timeout_ms  TODO: describe parameter
 * @retval  TODO: describe return value
 */
bool bm_i2c_master_receive(I2C_TypeDef *i2c, uint16_t dev_addr_7bit,
                           uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (!data || len == 0U) {
        return false;
    }

    bm_i2c_xfer_t *job = (i2c == I2C1) ? &s_i2c1_xfer : &s_i2c3_xfer;

    job->done = false;
    job->error = false;
    job->tx = NULL;
    job->rx = data;
    job->len = len;
    job->idx = 0U;

    i2c_clear_flags(i2c);
    i2c->CR2 = ((uint32_t)dev_addr_7bit << I2C_CR2_SADD_Pos)
             | ((uint32_t)len << I2C_CR2_NBYTES_Pos)
             | I2C_CR2_RD_WRN
             | I2C_CR2_AUTOEND
             | I2C_CR2_START;

    i2c->CR1 |= I2C_CR1_RXIE | I2C_CR1_STOPIE | I2C_CR1_NACKIE | I2C_CR1_ERRIE;

    return i2c_wait_job(job, timeout_ms);
}
