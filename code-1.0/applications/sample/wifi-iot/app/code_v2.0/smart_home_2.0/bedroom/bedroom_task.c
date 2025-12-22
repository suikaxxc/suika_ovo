#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "mqtt_utils.h"

void init_lights(void)
{
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);
}

void bedroom_thread(void *arg)
{
    arg = arg;

    int light_pin[3] = {WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_NAME_GPIO_12};

    init_lights();
    while (1)
    {

        for (int i = 0; i < 3; i++)
            GpioSetOutputVal(light_pin[i], lights_stat[i]); //绿

        sleep(1);
    }
}

void bedroom_task(void)
{
    osThreadAttr_t attr;
    attr.name = "bedroom_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(bedroom_thread, NULL, &attr) == NULL)
    {
        printf("[bedroom_task] Falied to create bedroom_task!\n");
    }
}