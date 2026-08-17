/**
 * @file    diag.h
 * @brief   Lightweight runtime diagnostics: per-task stack high-water-mark
 *          tracking, AI/loop latency sampling, and a periodic text report
 *          sent over LPUART. Used by app_freertos.c to keep an eye on the
 *          Sensor/DSP/AI/Comm/Diag task set without a debugger attached.
 */

#ifndef DIAG_H
#define DIAG_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Maximum number of tasks the diagnostics module can track. */
#define DIAG_MAX_TASKS 8U

/**
 * @brief   Registers a task so its name and stack high-water-mark are
 *          included in the periodic diagnostics report.
 * @param   handle  Handle of the task to track (may be NULL; the call
 *                   is then a no-op).
 * @param   name    Short human-readable label for the report (copied
 *                   internally, does not need to stay alive).
 */
void DIAG_RegisterTask(TaskHandle_t handle, const char *name);

/**
 * @brief   Returns a free-running microsecond timestamp, suitable for
 *          measuring short latencies (AI inference time, loop time).
 *          Backed by the Cortex-M4 DWT cycle counter; wraps roughly
 *          every ~26.8 minutes at 168 MHz, which is fine for the
 *          short deltas it's used to measure here.
 * @retval  Current timestamp in microseconds.
 */
uint32_t DIAG_MicrosNow(void);

/**
 * @brief   Records one AI-inference latency sample and updates the
 *          rolling min/max/average used in the report.
 * @param   elapsed_us   Measured inference duration, in microseconds.
 * @param   period_ms    The task's nominal period, in milliseconds
 *                        (used to flag samples that exceed budget).
 */
void DIAG_RecordAiLatency(uint32_t elapsed_us, uint32_t period_ms);

/**
 * @brief   Records one main-loop latency sample and updates the
 *          rolling min/max/average used in the report.
 * @param   elapsed_us   Measured loop duration, in microseconds.
 * @param   period_ms    The task's nominal period, in milliseconds
 *                        (used to flag samples that exceed budget).
 */
void DIAG_RecordLoopLatency(uint32_t elapsed_us, uint32_t period_ms);

/**
 * @brief   Prints a one-shot diagnostics report over LPUART: AI/loop
 *          latency stats and each registered task's stack
 *          high-water-mark (in words).
 */
void DIAG_PrintReport(void);

#ifdef __cplusplus
}
#endif

#endif /* DIAG_H */
