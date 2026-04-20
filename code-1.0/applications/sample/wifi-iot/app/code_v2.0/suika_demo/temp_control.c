/**
 * @file temp_control.c
 * @brief Temperature control implementation (heater and fan) for aquatic plant tank
 * 
 * Heater: GPIO10 (digital control) - Active LOW (LOW=ON, HIGH=OFF)
 * Fan: GPIO04 (digital output) - Active LOW
 * 
 * Note: Since GPIO14 is used for I2C0_SCL in OLED display,
 * we use GPIO04 for fan control instead
 * 
 * IMPORTANT: Both heater and fan use active-low logic:
 *   - LOW level (GPIO_VALUE0) = Device ON/Running
 *   - HIGH level (GPIO_VALUE1) = Device OFF/Stopped
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "temp_control.h"

// Heater control pin
#define HEATER_GPIO WIFI_IOT_GPIO_IDX_10
#define HEATER_IO   WIFI_IOT_IO_NAME_GPIO_10

// Fan control pin (using GPIO04 to avoid I2C conflict)
#define FAN_GPIO WIFI_IOT_GPIO_IDX_4
#define FAN_IO   WIFI_IOT_IO_NAME_GPIO_4

static int g_heater_state = 0;
static int g_fan_speed = 0;

void TempControl_Init(void)
{
    // Initialize heater GPIO (start with heater OFF = HIGH for active-low)
    IoSetFunc(HEATER_IO, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(HEATER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE1);  // HIGH = OFF (active-low)
    g_heater_state = 0;

    // Initialize fan GPIO (start with fan OFF = HIGH for active-low)
    IoSetFunc(FAN_IO, WIFI_IOT_IO_FUNC_GPIO_4_GPIO);
    GpioSetDir(FAN_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(FAN_GPIO, WIFI_IOT_GPIO_VALUE1);  // HIGH = OFF (active-low)
    g_fan_speed = 0;

    printf("[TempControl] Initialized (active-low logic: LOW=ON, HIGH=OFF)\n");
}

void Heater_On(void)
{
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE0);  // LOW = ON (active-low)
    g_heater_state = 1;
}

void Heater_Off(void)
{
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE1);  // HIGH = OFF (active-low)
    g_heater_state = 0;
}

int Heater_GetState(void)
{
    return g_heater_state;
}

void Fan_SetSpeed(int speedPercent)
{
    if (speedPercent < 0) speedPercent = 0;
    if (speedPercent > 100) speedPercent = 100;

    if (speedPercent == 0)
    {
        // OFF: HIGH level for active-low hardware
        GpioSetOutputVal(FAN_GPIO, WIFI_IOT_GPIO_VALUE1);
        g_fan_speed = 0;
    }
    else
    {
        // ON: LOW level for active-low hardware
        // Hardware does not support PWM speed control, treat any non-zero as ON.
        GpioSetOutputVal(FAN_GPIO, WIFI_IOT_GPIO_VALUE0);
        g_fan_speed = 1;
    }
}

int Fan_GetSpeed(void)
{
    return g_fan_speed;
}

void Fan_Stop(void)
{
    Fan_SetSpeed(0);
}
