#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

//GPIO INIT: FUNC AND DIR
void init(void)
{
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);
}
//TrafficLightTask:control led_y led_r led_g on/off
//ON:1   OFF:0
static void StartTrafficLightTask(void)
{
    init();
    while (1)
    {
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_VALUE1);
        usleep(3000 * 1000);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_VALUE0);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_VALUE1);
        usleep(1000 * 1000);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_VALUE0);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_VALUE1);
        usleep(3000 * 1000);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_VALUE0);
    }
}

APP_FEATURE_INIT(StartTrafficLightTask);


