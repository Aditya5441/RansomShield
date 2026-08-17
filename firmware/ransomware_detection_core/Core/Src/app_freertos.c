/**
 * @file    app_freertos.c
 * @brief   Multi-task FreeRTOS architecture (Sensor/DSP/AI/Comm/Diag tasks).
 *
 * MX_AppFreeRTOS_Init() is called from main() (after osKernelInitialize()
 * and before osKernelStart()) to create the Boot/Sensor/DSP/AI/Comm/Diag
 * tasks below. This is the active task architecture for the board.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "bm.h"
#include "sensors.h"
#include "dsp.h"
#include "comm_encryption.h"
#include "esp32.h"
#include "ai.h"
#include "ai_inputs.h"
#include "app_x-cube-ai.h"
#include "diag.h"

volatile unsigned long ulIdleCycleCount = 0UL;

static TaskHandle_t xAiTaskHandle  = NULL;

static volatile uint8_t ucAlertActive = 0;

static volatile float   fLatestAiScore = 0.0f;
#define SENSOR_STACK_WORDS   256U
#define DSP_STACK_WORDS      512U
#define AI_STACK_WORDS       512U
#define COMM_STACK_WORDS     256U
#define BOOT_STACK_WORDS     128U

#define SENSOR_PERIOD_MS     100UL
#define DSP_PERIOD_MS        100UL
#define AI_PERIOD_MS         200UL
#define COMM_PERIOD_MS       200UL
#define DIAG_PERIOD_MS       5000UL
#define DIAG_STACK_WORDS     256U

static SensorData_t   xLatestSensors[2];
static DSP_Features_t xLatestFeatures[2];

static volatile uint8_t ucSensorReadIdx  = 0;
static volatile uint8_t ucSensorWriteIdx = 1;
static volatile uint8_t ucDspReadIdx     = 0;
static volatile uint8_t ucDspWriteIdx    = 1;

/**
 * @brief   TODO: describe what vBootBannerTask() does
 * @param   pvParameters  TODO: describe parameter
 */
static void vBootBannerTask( void *pvParameters )
{
    (void)pvParameters;
    bm_lpuart_transmit( (const uint8_t *)"[BOOT] Ransomware-Detection FW ready\r\n", 38, 100 );
    vTaskDelete( NULL );
}

/**
 * @brief   TODO: describe what vSensorTask() does
 * @param   pvParameters  TODO: describe parameter
 */
static void vSensorTask( void *pvParameters )
{
    (void)pvParameters;
    const TickType_t xPeriod     = pdMS_TO_TICKS( SENSOR_PERIOD_MS );
    TickType_t       xLastWake   = xTaskGetTickCount();
    SensorData_t     local;

    for( ;; )
    {
        Sensors_ReadAll( &local );
        xLatestSensors[ucSensorWriteIdx] = local;
        uint8_t ucNextReadIdx = ucSensorWriteIdx;
        ucSensorWriteIdx = ucSensorReadIdx;
        ucSensorReadIdx  = ucNextReadIdx;
        vTaskDelayUntil( &xLastWake, xPeriod );
    }
}

/**
 * @brief   TODO: describe what vDspTask() does
 * @param   pvParameters  TODO: describe parameter
 */
static void vDspTask( void *pvParameters )
{
    (void)pvParameters;
    const TickType_t xPeriod     = pdMS_TO_TICKS( DSP_PERIOD_MS );
    TickType_t       xLastWake   = xTaskGetTickCount();
    float            dsp_in[DSP_SIZE];
    DSP_Features_t   features;

    DSP_Init();

    for( ;; )
    {
        SensorData_t snap = xLatestSensors[ucSensorReadIdx];

        const float ch[13] = {
            snap.usb_voltage, snap.usb_current, snap.usb_power, snap.temp,
            snap.accel_external[0], snap.accel_external[1], snap.accel_external[2],
            snap.accel_internal[0], snap.accel_internal[1], snap.accel_internal[2],
            (float)snap.rf_nrf_dbm, (float)snap.rf_si_dbm,
            snap.net_traffic
        };

        for( int i = 0; i < DSP_SIZE; i++ ) {
            dsp_in[i] = ch[i % 13];
        }
        DSP_RunPipeline( dsp_in, &features );
        xLatestFeatures[ucDspWriteIdx] = features;
        uint8_t ucNextDspReadIdx = ucDspWriteIdx;
        ucDspWriteIdx = ucDspReadIdx;
        ucDspReadIdx  = ucNextDspReadIdx;
        vTaskDelayUntil( &xLastWake, xPeriod );
    }
}
/**
 * @brief   TODO: describe what vAiTask() does
 * @param   pvParameters  TODO: describe parameter
 */
static void vAiTask( void *pvParameters )
{
    (void)pvParameters;
    const TickType_t  xPeriod    = pdMS_TO_TICKS( AI_PERIOD_MS );
    TickType_t        xLastWake  = xTaskGetTickCount();
    const UBaseType_t uxBasePrio = uxTaskPriorityGet( NULL );

    for( ;; )
    {
        SensorData_t snap = xLatestSensors[ucSensorReadIdx];

        uint32_t t0    = DIAG_MicrosNow();
        float    score = ai_predict_sensors( &snap );
        uint32_t t1    = DIAG_MicrosNow();
        DIAG_RecordAiLatency( t1 - t0, AI_PERIOD_MS );

        fLatestAiScore = score;

        if( score > 0.7f )
        {
            ucAlertActive = 1U;
            vTaskPrioritySet( NULL, uxBasePrio + 2U );
            bm_lpuart_transmit( (const uint8_t *)"[ALERT] Anomaly Detected!\r\n", 27, 10 );
            vTaskPrioritySet( NULL, uxBasePrio );
            ucAlertActive = 0U;
        }
        vTaskDelayUntil( &xLastWake, xPeriod );
    }
}

/**
 * @brief   TODO: describe what vCommTask() does
 * @param   pvParameters  TODO: describe parameter
 */
static void vCommTask( void *pvParameters )
{
    (void)pvParameters;
    const TickType_t xPeriod     = pdMS_TO_TICKS( COMM_PERIOD_MS );
    TickType_t       xLastWake   = xTaskGetTickCount();
    SystemData_t     sys;
    SecurePacket_t   packet;

    for( ;; )
    {
        SensorData_t   snapS = xLatestSensors[ucSensorReadIdx];
        DSP_Features_t snapF = xLatestFeatures[ucDspReadIdx];
        float          score = fLatestAiScore;

        sys.voltage     = snapS.usb_voltage;
        sys.current     = snapS.usb_current;
        sys.power       = snapS.usb_power;
        sys.temperature = snapS.temp;
        sys.network     = snapS.net_traffic;
        sys.rf          = (float)snapS.rf_nrf_dbm;
        sys.ai_output   = score;

        for( int i = 0; i < 3; i++ )
        {
            sys.accel_internal[i] = snapS.accel_internal[i];
            sys.accel_external[i] = snapS.accel_external[i];
        }

        if (ENC_Encrypt( &sys, &snapF, &packet ))
        {
            ESP_Send( (uint8_t *)&packet, (uint16_t)sizeof(packet) );
        }
        /* else: no ECDH session key yet - nothing to send this cycle.
         * SESSION_Handshake() should have run at boot before this task
         * started; if this branch is hit repeatedly, the handshake
         * with the ESP32 failed or hasn't completed. */

        vTaskDelayUntil( &xLastWake, xPeriod );
    }
}

/**
 * @brief   Periodically prints the AI-latency and per-task stack
 *          high-water-mark diagnostics report over LPUART. Runs at the
 *          lowest priority so it never perturbs the timing of the tasks
 *          it's measuring.
 * @param   pvParameters  Unused, required by the FreeRTOS task signature.
 */
static void vDiagTask( void *pvParameters )
{
    (void)pvParameters;
    const TickType_t xPeriod   = pdMS_TO_TICKS( DIAG_PERIOD_MS );
    TickType_t       xLastWake = xTaskGetTickCount();

    for( ;; )
    {
        DIAG_PrintReport();
        vTaskDelayUntil( &xLastWake, xPeriod );
    }
}

/**
 * @brief   TODO: describe what vApplicationIdleHook() does
 */
void vApplicationIdleHook( void )
{
    ulIdleCycleCount++;
}

/**
 * @brief   TODO: describe what MX_AppFreeRTOS_Init() does
 */
void MX_AppFreeRTOS_Init( void )
{
    BaseType_t   xRet;
    TaskHandle_t xSensorTaskHandle = NULL;
    TaskHandle_t xDspTaskHandle    = NULL;
    TaskHandle_t xCommTaskHandle   = NULL;

    Sensors_Init();

    xRet = xTaskCreate( vBootBannerTask, "Boot",   BOOT_STACK_WORDS,   NULL, 1, NULL );
    configASSERT( xRet == pdPASS );

    xRet = xTaskCreate( vSensorTask,     "Sensor", SENSOR_STACK_WORDS, NULL, 1, &xSensorTaskHandle );
    configASSERT( xRet == pdPASS );

    xRet = xTaskCreate( vDspTask,        "DSP",    DSP_STACK_WORDS,    NULL, 1, &xDspTaskHandle );
    configASSERT( xRet == pdPASS );

    xRet = xTaskCreate( vAiTask,         "AI",     AI_STACK_WORDS,     NULL, 2, &xAiTaskHandle );
    configASSERT( xRet == pdPASS );

    xRet = xTaskCreate( vCommTask,       "Comm",   COMM_STACK_WORDS,   NULL, 3, &xCommTaskHandle );
    configASSERT( xRet == pdPASS );

    xRet = xTaskCreate( vDiagTask,       "Diag",   DIAG_STACK_WORDS,   NULL, 0, NULL );
    configASSERT( xRet == pdPASS );

    DIAG_RegisterTask( xSensorTaskHandle, "Sensor" );
    DIAG_RegisterTask( xDspTaskHandle,    "DSP" );
    DIAG_RegisterTask( xAiTaskHandle,     "AI" );
    DIAG_RegisterTask( xCommTaskHandle,   "Comm" );
}
