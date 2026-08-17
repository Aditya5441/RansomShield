/**
 * @file    diag.c
 * @brief   Lightweight runtime diagnostics: per-task stack high-water-mark
 *          tracking, AI/loop latency sampling, and a periodic text report
 *          sent to the ESP32 (via ESP_Send) and mirrored on LPUART for a
 *          local debug console.
 */

#include <stdio.h>
#include <string.h>

#include "diag.h"
#include "main.h"
#include "bm.h"
#include "esp32.h"

/** Cortex-M4 DWT/CoreDebug registers, used for a free-running
 *  microsecond timestamp independent of the RTOS tick. */
#define DIAG_DWT_CTRL      (*(volatile uint32_t *)0xE0001000UL)
#define DIAG_DWT_CYCCNT    (*(volatile uint32_t *)0xE0001004UL)
#define DIAG_DEMCR         (*(volatile uint32_t *)0xE000EDFCUL)
#define DIAG_DWT_CYCCNTENA (1UL << 0)
#define DIAG_DEMCR_TRCENA  (1UL << 24)

typedef struct {
    TaskHandle_t handle;
    char         name[configMAX_TASK_NAME_LEN];
    uint8_t      in_use;
} DIAG_TaskEntry_t;

typedef struct {
    uint32_t min_us;
    uint32_t max_us;
    uint32_t sum_us;
    uint32_t count;
    uint32_t over_budget_count;
} DIAG_LatencyStats_t;

static DIAG_TaskEntry_t    s_tasks[DIAG_MAX_TASKS];
static uint8_t             s_task_count   = 0U;
static uint8_t             s_dwt_started  = 0U;

static DIAG_LatencyStats_t s_ai_stats;
static DIAG_LatencyStats_t s_loop_stats;

extern uint32_t SystemCoreClock;

/**
 * @brief   Lazily enables the DWT cycle counter on first use.
 */
static void diag_dwt_start(void)
{
    if (!s_dwt_started) {
        DIAG_DEMCR |= DIAG_DEMCR_TRCENA;
        DIAG_DWT_CYCCNT = 0U;
        DIAG_DWT_CTRL  |= DIAG_DWT_CYCCNTENA;
        s_dwt_started = 1U;
    }
}

/**
 * @brief   Resets a latency stats block to its initial "no samples yet"
 *          state.
 * @param   stats  Stats block to reset.
 */
static void diag_stats_reset(DIAG_LatencyStats_t *stats)
{
    stats->min_us = 0xFFFFFFFFUL;
    stats->max_us = 0U;
    stats->sum_us = 0U;
    stats->count  = 0U;
    stats->over_budget_count = 0U;
}

/**
 * @brief   Folds one new sample into a latency stats block.
 * @param   stats        Stats block to update.
 * @param   elapsed_us   Newly measured latency, in microseconds.
 * @param   period_ms    Nominal period; a sample exceeding this budget
 *                        is counted separately.
 */
static void diag_stats_record(DIAG_LatencyStats_t *stats, uint32_t elapsed_us, uint32_t period_ms)
{
    if (elapsed_us < stats->min_us) {
        stats->min_us = elapsed_us;
    }
    if (elapsed_us > stats->max_us) {
        stats->max_us = elapsed_us;
    }
    stats->sum_us += elapsed_us;
    stats->count++;

    if (elapsed_us > (period_ms * 1000UL)) {
        stats->over_budget_count++;
    }
}

void DIAG_RegisterTask(TaskHandle_t handle, const char *name)
{
    if ((handle == NULL) || (s_task_count >= DIAG_MAX_TASKS)) {
        return;
    }

    s_tasks[s_task_count].handle  = handle;
    s_tasks[s_task_count].in_use  = 1U;
    strncpy(s_tasks[s_task_count].name, name, configMAX_TASK_NAME_LEN - 1U);
    s_tasks[s_task_count].name[configMAX_TASK_NAME_LEN - 1U] = '\0';
    s_task_count++;
}

uint32_t DIAG_MicrosNow(void)
{
    diag_dwt_start();
    /* Read the live core clock rather than a hardcoded value, so this
     * stays correct if SystemCoreClock ever changes (e.g. RCC rework). */
    return (uint32_t)(((uint64_t)DIAG_DWT_CYCCNT * 1000000ULL) / SystemCoreClock);
}

void DIAG_RecordAiLatency(uint32_t elapsed_us, uint32_t period_ms)
{
    if (s_ai_stats.count == 0U) {
        diag_stats_reset(&s_ai_stats);
    }
    diag_stats_record(&s_ai_stats, elapsed_us, period_ms);
}

void DIAG_RecordLoopLatency(uint32_t elapsed_us, uint32_t period_ms)
{
    if (s_loop_stats.count == 0U) {
        diag_stats_reset(&s_loop_stats);
    }
    diag_stats_record(&s_loop_stats, elapsed_us, period_ms);
}

/**
 * @brief   Sends one line of the report to the ESP32 link.
 * @note    ESP_Send() and the board's LPUART debug console are the same
 *          physical LPUART1 peripheral on this hardware — do NOT also
 *          call bm_lpuart_transmit() here, or every line gets sent
 *          twice back-to-back on the wire the ESP32 is framing its own
 *          protocol on.
 * @param   line  NUL-terminated line to send (a trailing "\r\n" is
 *                appended by the caller inside the format string).
 */
static void diag_send_line(const char *line)
{
    ESP_Send((uint8_t *)line, (uint16_t)strlen(line));
}

void DIAG_PrintReport(void)
{
    char line[96];

    diag_send_line("[DIAG] ---- report ----\r\n");

    if (s_ai_stats.count > 0U) {
        snprintf(line, sizeof(line),
                 "[DIAG] AI   latency us min=%lu max=%lu avg=%lu over_budget=%lu\r\n",
                 (unsigned long)s_ai_stats.min_us,
                 (unsigned long)s_ai_stats.max_us,
                 (unsigned long)(s_ai_stats.sum_us / s_ai_stats.count),
                 (unsigned long)s_ai_stats.over_budget_count);
        diag_send_line(line);
    }

    if (s_loop_stats.count > 0U) {
        snprintf(line, sizeof(line),
                 "[DIAG] Loop latency us min=%lu max=%lu avg=%lu over_budget=%lu\r\n",
                 (unsigned long)s_loop_stats.min_us,
                 (unsigned long)s_loop_stats.max_us,
                 (unsigned long)(s_loop_stats.sum_us / s_loop_stats.count),
                 (unsigned long)s_loop_stats.over_budget_count);
        diag_send_line(line);
    }

    for (uint8_t i = 0U; i < s_task_count; i++) {
        if (!s_tasks[i].in_use) {
            continue;
        }
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(s_tasks[i].handle);
        snprintf(line, sizeof(line),
                 "[DIAG] task=%-8s stack_hwm_words=%lu\r\n",
                 s_tasks[i].name, (unsigned long)hwm);
        diag_send_line(line);
    }
}
