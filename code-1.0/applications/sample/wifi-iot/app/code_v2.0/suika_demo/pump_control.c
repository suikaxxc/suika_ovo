/**
 * @file pump_control.c
 * @brief Water pump control implementation for aquatic plant tank
 * 
 * Drain Pump Relay:
 *   - GPIO00 HIGH: relay ON, drain pump ON
 *   - GPIO00 LOW: relay OFF, drain pump OFF
 *
 * Fill Pump Relay:
 *   - GPIO05 HIGH: relay ON, fill pump ON
 *   - GPIO05 LOW: relay OFF, fill pump OFF
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "pump_control.h"

// Drain pump relay pin (active-high)
#define DRAIN_PUMP_RELAY_GPIO WIFI_IOT_GPIO_IDX_0
#define DRAIN_PUMP_RELAY_IO   WIFI_IOT_IO_NAME_GPIO_0

// Fill pump relay pin (active-high)
#define FILL_PUMP_RELAY_GPIO WIFI_IOT_GPIO_IDX_5
#define FILL_PUMP_RELAY_IO   WIFI_IOT_IO_NAME_GPIO_5

static PumpStatus g_drain_pump_status = PUMP_OFF;
static PumpStatus g_fill_pump_status = PUMP_OFF;

void Pump_Init(void)
{
    // Initialize drain pump relay pin
    IoSetFunc(DRAIN_PUMP_RELAY_IO, WIFI_IOT_IO_FUNC_GPIO_0_GPIO);
    GpioSetDir(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE0);

    // Initialize fill pump relay pin
    // Note: GPIO05 is connected to onboard button on some Hi3861 OLED boards
    // Keep pull disabled to avoid conflicts with button circuit
    IoSetFunc(FILL_PUMP_RELAY_IO, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    
    // Disable internal pull-up/pull-down on GPIO05 (may have external pull-up from button)
    IoSetPull(FILL_PUMP_RELAY_IO, WIFI_IOT_IO_PULL_NONE);
    
    GpioSetDir(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE0);
    
    printf("[Pump] GPIO05 pull disabled, configured as output for fill relay\n");

    g_drain_pump_status = PUMP_OFF;
    g_fill_pump_status = PUMP_OFF;

    printf("[Pump] Initialized (active-high relays: drain GPIO00, fill GPIO05)\n");
}

void Pump_SetState(PumpType pump, PumpStatus status)
{
    if (pump == PUMP_DRAIN)
    {
        if (status == PUMP_ON)
        {
            // Relay ON
            GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
            g_drain_pump_status = PUMP_ON;
        }
        else
        {
            // Relay OFF
            GpioSetOutputVal(DRAIN_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE0);
            g_drain_pump_status = PUMP_OFF;
        }
    }
    else if (pump == PUMP_FILL)
    {
        if (status == PUMP_ON)
        {
            // Relay ON
            GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE1);
            g_fill_pump_status = PUMP_ON;
        }
        else
        {
            // Relay OFF
            GpioSetOutputVal(FILL_PUMP_RELAY_GPIO, WIFI_IOT_GPIO_VALUE0);
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
