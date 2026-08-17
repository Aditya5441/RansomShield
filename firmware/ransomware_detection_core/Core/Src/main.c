/**
 * @file    main.c
 * @brief   Application entry point. Boots the bare-metal platform, the
 *          on-device AI (X-CUBE-AI) runtime, and then hands off to the
 *          FreeRTOS multi-task architecture defined in app_freertos.c
 *          (Sensor / DSP / AI / Comm / Diag tasks).
 */

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOSConfig.h"
#include "app_x-cube-ai.h"
#include "bm.h"
#include "diag.h"
#include "session.h"

/**
 * @brief   Entry point. Initializes the bare-metal platform and the
 *          on-device AI runtime, runs the ECDH handshake with the
 *          ESP32 gateway to establish a session key (see session.h),
 *          then starts the FreeRTOS kernel with the task set created
 *          in MX_AppFreeRTOS_Init() (see app_freertos.c: Boot / Sensor
 *          / DSP / AI / Comm / Diag).
 * @retval  Does not return under normal operation.
 */
int main(void)
{
    bm_system_init();
    ai_init();

    /* Blocking, ~2s worst case (HANDSHAKE_TIMEOUT_MS in session.c).
     * If the ESP32 isn't listening yet or isn't wired up, this fails
     * and the Comm task will simply skip sending each cycle
     * (ENC_Encrypt() returns 0 with no session key) rather than the
     * old behavior of silently encrypting under a key the receiver
     * could never reproduce. Retry logic / periodic re-handshake is
     * not implemented yet - a single failed handshake here currently
     * means no session for the rest of this boot. */
    SESSION_Handshake();

    osKernelInitialize();

    /* Creates the Sensor/DSP/AI/Comm/Diag tasks (see app_freertos.c).
     * Must run after osKernelInitialize() and before osKernelStart(),
     * since it calls the raw FreeRTOS xTaskCreate() API directly. */
    MX_AppFreeRTOS_Init();

    osKernelStart();

    /* osKernelStart() does not return once the scheduler is running. */
    while (1) {
    }
}

/**
 * @brief   TODO: describe what Error_Handler() does
 */
void Error_Handler(void)
{
  __disable_irq();
    while (1) {
  }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief   TODO: describe what assert_failed() does
 * @param   file  TODO: describe parameter
 * @param   line  TODO: describe parameter
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
