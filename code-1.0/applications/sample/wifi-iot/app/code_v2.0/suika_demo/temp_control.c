/**
 * @file temp_control.c
 * @brief Temperature control implementation (heater and PWM fan) for aquatic plant tank
 * 
 * Heater: GPIO10 (digital control)
 * Fan: PWM5 (GPIO14) - Using PWM for variable speed control
 * 
 * Note: Since GPIO14 is used for I2C0_SCL in OLED display,
 * we use GPIO04/PWM1 for fan control instead
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_errno.h"

#include "temp_control.h"

// Heater control pin
#define HEATER_GPIO WIFI_IOT_GPIO_IDX_10
#define HEATER_IO   WIFI_IOT_IO_NAME_GPIO_10

// Fan PWM control pin (using GPIO04/PWM1 to avoid I2C conflict)
#define FAN_GPIO WIFI_IOT_GPIO_IDX_4
#define FAN_IO   WIFI_IOT_IO_NAME_GPIO_4
#define FAN_PWM_PORT WIFI_IOT_PWM_PORT_PWM1

// PWM frequency for fan (40kHz for DC brushless fan quiet operation)
#define FAN_PWM_FREQ 40000

static int g_heater_state = 0;
static int g_fan_speed = 0;

void TempControl_Init(void)
{
    // Initialize heater GPIO
    IoSetFunc(HEATER_IO, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(HEATER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE0);
    g_heater_state = 0;

    // Initialize fan PWM
    IoSetFunc(FAN_IO, WIFI_IOT_IO_FUNC_GPIO_4_PWM1_OUT);
    PwmInit(FAN_PWM_PORT);
    g_fan_speed = 0;

    printf("[TempControl] Initialized\n");
}

void Heater_On(void)
{
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_heater_state = 1;
}

void Heater_Off(void)
{
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE0);
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
        PwmStop(FAN_PWM_PORT);
        g_fan_speed = 0;
    }
    else
    {
        // Calculate duty cycle (duty/freq ratio)
        uint16_t duty = (uint16_t)((FAN_PWM_FREQ * speedPercent) / 100);
        PwmStart(FAN_PWM_PORT, duty, FAN_PWM_FREQ);
        g_fan_speed = speedPercent;
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
