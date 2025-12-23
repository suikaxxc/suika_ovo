/**
 * @file pump_control.c
 * @brief Water pump control implementation for aquatic plant tank
 * 
 * Drain Pump (L9110S #1):
 *   - IA (GPIO00) - Forward control
 *   - IB (GPIO01) - Reverse control
 * 
 * Fill Pump (L9110S #2):
 *   - IA (GPIO05) - Forward control (originally used for buttons)
 *   - IB (GPIO06) - Reverse control
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "pump_control.h"

// Drain pump pins (L9110S #1)
#define DRAIN_PUMP_IA_GPIO WIFI_IOT_GPIO_IDX_0
#define DRAIN_PUMP_IA_IO   WIFI_IOT_IO_NAME_GPIO_0
#define DRAIN_PUMP_IB_GPIO WIFI_IOT_GPIO_IDX_1
#define DRAIN_PUMP_IB_IO   WIFI_IOT_IO_NAME_GPIO_1

// Fill pump pins (L9110S #2 - GPIO05 repurposed from buttons)
#define FILL_PUMP_IA_GPIO WIFI_IOT_GPIO_IDX_5
#define FILL_PUMP_IA_IO   WIFI_IOT_IO_NAME_GPIO_5
#define FILL_PUMP_IB_GPIO WIFI_IOT_GPIO_IDX_6
#define FILL_PUMP_IB_IO   WIFI_IOT_IO_NAME_GPIO_6

static PumpStatus g_drain_pump_status = PUMP_OFF;
static PumpStatus g_fill_pump_status = PUMP_OFF;

void Pump_Init(void)
{
    // Initialize drain pump pins (L9110S #1)
    IoSetFunc(DRAIN_PUMP_IA_IO, WIFI_IOT_IO_FUNC_GPIO_0_GPIO);
    IoSetFunc(DRAIN_PUMP_IB_IO, WIFI_IOT_IO_FUNC_GPIO_1_GPIO);
    GpioSetDir(DRAIN_PUMP_IA_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(DRAIN_PUMP_IB_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(DRAIN_PUMP_IA_GPIO, WIFI_IOT_GPIO_VALUE0);
    GpioSetOutputVal(DRAIN_PUMP_IB_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Initialize fill pump pins (L9110S #2 - GPIO05 + GPIO06)
    IoSetFunc(FILL_PUMP_IA_IO, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    IoSetFunc(FILL_PUMP_IB_IO, WIFI_IOT_IO_FUNC_GPIO_6_GPIO);
    GpioSetDir(FILL_PUMP_IA_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(FILL_PUMP_IB_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(FILL_PUMP_IA_GPIO, WIFI_IOT_GPIO_VALUE0);
    GpioSetOutputVal(FILL_PUMP_IB_GPIO, WIFI_IOT_GPIO_VALUE0);

    g_drain_pump_status = PUMP_OFF;
    g_fill_pump_status = PUMP_OFF;

    printf("[Pump] Initialized (L9110S dual-pin control for both pumps)\n");
}

void Pump_SetState(PumpType pump, PumpStatus status)
{
    if (pump == PUMP_DRAIN)
    {
        if (status == PUMP_ON)
        {
            // Forward rotation for drain
            GpioSetOutputVal(DRAIN_PUMP_IA_GPIO, WIFI_IOT_GPIO_VALUE1);
            GpioSetOutputVal(DRAIN_PUMP_IB_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_drain_pump_status = PUMP_ON;
        }
        else
        {
            // Stop drain pump
            GpioSetOutputVal(DRAIN_PUMP_IA_GPIO, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(DRAIN_PUMP_IB_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_drain_pump_status = PUMP_OFF;
        }
    }
    else if (pump == PUMP_FILL)
    {
        if (status == PUMP_ON)
        {
            // Forward rotation for fill (L9110S #2)
            GpioSetOutputVal(FILL_PUMP_IA_GPIO, WIFI_IOT_GPIO_VALUE1);
            GpioSetOutputVal(FILL_PUMP_IB_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_fill_pump_status = PUMP_ON;
        }
        else
        {
            // Stop fill pump
            GpioSetOutputVal(FILL_PUMP_IA_GPIO, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(FILL_PUMP_IB_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_fill_pump_status = PUMP_OFF;
        }
    }
}

PumpStatus Pump_GetState(PumpType pump)
{
    if (pump == PUMP_DRAIN)
    {
        return g_drain_pump_status;
    }
    return g_fill_pump_status;
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
    Pump_StopDrain();
    Pump_StopFill();
}
