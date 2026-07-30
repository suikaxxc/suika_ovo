/**
 * @file temp_control.c
 * @brief Temperature control implementation (heater and PWM fan) for aquatic plant tank
 * 
 * Heater: GPIO10 (digital control) - Active LOW (LOW=ON, HIGH=OFF)
 * Fan: GPIO04/PWM1 - Using PWM for variable speed control
 * 
 * Note: Since GPIO14 is used for I2C0_SCL in OLED display,
 * we use GPIO04/PWM1 for fan control instead
 * 
 * PWM Configuration (compared with STM32 reference):
 *   - STM32: TIM2, 72MHz/720/100 = 1kHz, Active-HIGH (TIM_OCPolarity_High)
 *   - Hi3861: PWM1, 160MHz/40000 = 4kHz
 * 
 * IMPORTANT:
 *   - Heater uses active-low logic: LOW=ON, HIGH=OFF
 *   - Fan PWM default uses active-high logic (for common 5V fan drive boards):
 *       duty越大转速越高，0% duty为停转
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
// Fan PWM active level:
// 0: active-high (default, common 5V fan transistor driver)
// 1: active-low  (legacy wiring compatibility)
#define FAN_PWM_ACTIVE_LOW 0

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
    // Start with fan OFF
#if FAN_PWM_ACTIVE_LOW
    PwmStart(FAN_PWM_PORT, FAN_PWM_FREQ, FAN_PWM_FREQ); // all HIGH -> OFF
#else
    PwmStart(FAN_PWM_PORT, 1, FAN_PWM_FREQ);            // ~0% duty -> OFF
#endif
    g_fan_speed = 0;

    printf("[TempControl] Initialized (heater active-low, fan active-%s)\n",
           FAN_PWM_ACTIVE_LOW ? "low" : "high");
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

    // Hi3861 PWM API: duty ratio = duty/freq, frequency = 160MHz/freq
    // Note: duty must be >=1, use 1 as "near 0%" duty.
    if (speedPercent == 0) {
#if FAN_PWM_ACTIVE_LOW
        PwmStart(FAN_PWM_PORT, FAN_PWM_FREQ, FAN_PWM_FREQ); // OFF
#else
        PwmStart(FAN_PWM_PORT, 1, FAN_PWM_FREQ);            // OFF
#endif
        return;
    }

    if (speedPercent >= 100) {
#if FAN_PWM_ACTIVE_LOW
        PwmStart(FAN_PWM_PORT, 1, FAN_PWM_FREQ);            // full ON
#else
        PwmStart(FAN_PWM_PORT, FAN_PWM_FREQ, FAN_PWM_FREQ); // full ON
#endif
        return;
    }

#if FAN_PWM_ACTIVE_LOW
    uint16_t duty = (uint16_t)((FAN_PWM_FREQ * (100 - speedPercent)) / 100);
#else
    uint16_t duty = (uint16_t)((FAN_PWM_FREQ * speedPercent) / 100);
#endif
    if (duty < 1) duty = 1;
    if (duty > FAN_PWM_FREQ) duty = FAN_PWM_FREQ;
    PwmStart(FAN_PWM_PORT, duty, FAN_PWM_FREQ);
}

int Fan_GetSpeed(void)
{
    return g_fan_speed;
}

void Fan_Stop(void)
{
    Fan_SetSpeed(0);
}
