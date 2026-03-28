/**
 * @file temp_control.c
 * @brief Temperature control implementation (heater and PWM fan) for aquatic plant tank
 * 
 * Heater: GPIO10 (digital control) - Active LOW (LOW=ON, HIGH=OFF)
 * Fan: GPIO04/PWM1 - Using PWM for variable speed control - Active LOW
 * 
 * Note: Since GPIO14 is used for I2C0_SCL in OLED display,
 * we use GPIO04/PWM1 for fan control instead
 * 
 * PWM Configuration (compared with STM32 reference):
 *   - STM32: TIM2, 72MHz/720/100 = 1kHz, Active-HIGH (TIM_OCPolarity_High)
 *   - Hi3861: PWM1, 160MHz/40000 = 4kHz, Active-LOW (inverted duty)
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
    // Initialize heater GPIO (start with heater OFF = HIGH for active-low)
    IoSetFunc(HEATER_IO, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(HEATER_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(HEATER_GPIO, WIFI_IOT_GPIO_VALUE1);  // HIGH = OFF (active-low)
    g_heater_state = 0;

    // Initialize fan PWM (start with fan OFF)
    IoSetFunc(FAN_IO, WIFI_IOT_IO_FUNC_GPIO_4_PWM1_OUT);
    PwmInit(FAN_PWM_PORT);
    // Set to 100% duty (all HIGH) to keep fan OFF (active-low)
    PwmStart(FAN_PWM_PORT, FAN_PWM_FREQ, FAN_PWM_FREQ);
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

    g_fan_speed = speedPercent;

    // Hi3861 PWM API: duty cycle = duty/freq, frequency = 160MHz/freq
    // With freq=40000: PWM frequency = 160MHz/40000 = 4kHz
    // 
    // For active-low logic (like STM32 reference with inverted polarity):
    // - 0% speed = 100% duty cycle (always HIGH = OFF)
    // - 100% speed = ~0% duty cycle (always LOW = full ON)
    // 
    // Note: duty must be in range [1, 65535], so we use 1 as minimum instead of 0
    // This means 100% speed is actually 99.9975% duty (duty=1/freq=40000)
    
    if (speedPercent == 0)
    {
        // Full stop: 100% duty (all HIGH = off)
        PwmStart(FAN_PWM_PORT, FAN_PWM_FREQ, FAN_PWM_FREQ);
    }
    else if (speedPercent >= 100)
    {
        // Full speed: minimum duty (almost all LOW = full on)
        // Use duty=1 instead of 0 to stay within valid range [1, 65535]
        PwmStart(FAN_PWM_PORT, 1, FAN_PWM_FREQ);
    }
    else
    {
        // Variable speed: invert duty for active-low
        // speedPercent 1-99 maps to duty (freq-1) down to 2
        int invertedPercent = 100 - speedPercent;
        uint16_t duty = (uint16_t)((FAN_PWM_FREQ * invertedPercent) / 100);
        if (duty < 1) duty = 1;  // Ensure duty is at least 1
        PwmStart(FAN_PWM_PORT, duty, FAN_PWM_FREQ);
    }
}

int Fan_GetSpeed(void)
{
    return g_fan_speed;
}

void Fan_Stop(void)
{
    Fan_SetSpeed(0);  // This will set 100% duty (all HIGH = OFF)
}
