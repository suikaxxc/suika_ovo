/**
 * @file led_control.c
 * @brief LED plant light control implementation for aquatic plant tank
 * Uses GPIO03 for LED control (avoiding conflict with other pins)
 */

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_errno.h"

#include "led_control.h"

// LED control pin
#define LED_GPIO WIFI_IOT_GPIO_IDX_3
#define LED_IO   WIFI_IOT_IO_NAME_GPIO_3

static int g_led_state = 0;

void LED_Init(void)
{
    IoSetFunc(LED_IO, WIFI_IOT_IO_FUNC_GPIO_3_GPIO);
    GpioSetDir(LED_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(LED_GPIO, WIFI_IOT_GPIO_VALUE0);
    g_led_state = 0;

    printf("[LED] Initialized\r\n");
}

void LED_On(void)
{
    GpioSetOutputVal(LED_GPIO, WIFI_IOT_GPIO_VALUE1);
    g_led_state = 1;
}

void LED_Off(void)
{
    GpioSetOutputVal(LED_GPIO, WIFI_IOT_GPIO_VALUE0);
    g_led_state = 0;
}

int LED_GetState(void)
{
    return g_led_state;
}

void LED_Toggle(void)
{
    if (g_led_state)
    {
        LED_Off();
    }
    else
    {
        LED_On();
    }
}
