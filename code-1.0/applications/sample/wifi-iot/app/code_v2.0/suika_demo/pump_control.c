/**
 * @file pump_control.c
 * @brief Water pump control implementation for aquatic plant tank
 * 
 * Drain Pump Relay:
 *   - GPIO00 - LOW level turns relay ON (active-low relay)
 * 
 * Fill Pump Relay:
 *   - GPIO05 - LOW level turns relay ON (active-low relay)
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "pump_control.h"

// Drain pump relay pin (active-low trigger)
#define DRAIN_PUMP_RELAY_GPIO WIFI_IOT_GPIO_IDX_0
#define DRAIN_PUMP_RELAY_IO   WIFI_IOT_IO_NAME_GPIO_0

// Fill pump relay pin (active-low trigger)
#define FILL_PUMP_RELAY_GPIO WIFI_IOT_GPIO_IDX_5
#define FILL_PUMP_RELAY_IO   WIFI_IOT_IO_NAME_GPIO_5

static PumpStatus g_drain_pump_status = PUMP_OFF;
static PumpStatus g_fill_pump_status = PUMP_OFF;
static osMutexId_t g_pump_mutex = NULL;

void Pump_Init(void)
{
    // Create mutex for thread-safe pump operations
    g_pump_mutex = osMutexNew(NULL);
    if (g_pump_mutex == NULL) {
        printf("[Pump] Warning: failed to create pump mutex, fallback to unlocked mode\n");
    }

    // Initialize drain pump relay pin
    IoSetFunc(DRAIN_PUMP_RELAY_IO, WIFI_IOT_IO_FUNC_GPIO_0_GPIO);
    GpioSetDir(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    // Keep relay OFF on startup for active-low modules
    GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);

    // Initialize fill pump relay pin (GPIO05)
    // Note: GPIO05 is connected to onboard button on some Hi3861 OLED boards
    // We must disable the internal pull-up to avoid conflicts with button circuit
    IoSetFunc(FILL_PUMP_RELAY_IO, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    
    // Disable internal pull-up/pull-down on GPIO05 (may have external pull-up from button)
    IoSetPull(FILL_PUMP_RELAY_IO, WIFI_IOT_IO_PULL_NONE);
    
    GpioSetDir(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    // Keep relay OFF on startup for active-low modules
    GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
    
    printf("[Pump] GPIO05 pull disabled, configured as relay output for fill pump\n");

    g_drain_pump_status = PUMP_OFF;
    g_fill_pump_status = PUMP_OFF;

    printf("[Pump] Initialized (active-low relay control for both pumps)\n");
}

void Pump_SetState(PumpType pump, PumpStatus status)
{
    if (g_pump_mutex != NULL) {
        osMutexAcquire(g_pump_mutex, osWaitForever);
    }

    if (pump == PUMP_DRAIN)
    {
        if (status == PUMP_ON)
        {
            // Interlock: drain and fill pumps must never run simultaneously
            GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
            g_fill_pump_status = PUMP_OFF;

            // Relay ON (active-low)
            GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_drain_pump_status = PUMP_ON;
        }
        else
        {
            // Relay OFF (active-low)
            GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
            g_drain_pump_status = PUMP_OFF;
        }
    }
    else if (pump == PUMP_FILL)
    {
        if (status == PUMP_ON)
        {
            // Interlock: fill and drain pumps must never run simultaneously
            GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
            g_drain_pump_status = PUMP_OFF;

            // Relay ON (active-low)
            GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_fill_pump_status = PUMP_ON;
        }
        else
        {
            // Relay OFF (active-low)
            GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
            g_fill_pump_status = PUMP_OFF;
        }
    }

    if (g_pump_mutex != NULL) {
        osMutexRelease(g_pump_mutex);
    }
}

PumpStatus Pump_GetState(PumpType pump)
{
    PumpStatus status;
    if (g_pump_mutex != NULL) {
        osMutexAcquire(g_pump_mutex, osWaitForever);
    }

    if (pump == PUMP_DRAIN)
    {
        status = g_drain_pump_status;
    }
    else
    {
        status = g_fill_pump_status;
    }

    if (g_pump_mutex != NULL) {
        osMutexRelease(g_pump_mutex);
    }
    return status;
}

void Pump_StartDrain(void)
{
    Pump_SetState(PUMP_DRAIN, PUMP_ON);
}

void Pump_StopDrain(void)
{
    Pump_SetState(PUMP_DRAIN, PUMP_OFF);
}

void Pump_StartFill(void)
{
    Pump_SetState(PUMP_FILL, PUMP_ON);
}

void Pump_StopFill(void)
{
    Pump_SetState(PUMP_FILL, PUMP_OFF);
}

void Pump_StopAll(void)
{
    if (g_pump_mutex != NULL) {
        osMutexAcquire(g_pump_mutex, osWaitForever);
    }

    GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
    GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_drain_pump_status = PUMP_OFF;
    g_fill_pump_status = PUMP_OFF;

    if (g_pump_mutex != NULL) {
        osMutexRelease(g_pump_mutex);
    }
}
