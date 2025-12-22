#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "hi_gpio.h"

static void OnButtonPressed(char *arg)
{
    (void)arg;
    printf("Key  S3  pressed\r\n");
}

static void StartS3Task(void)
{
    GpioInit();

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO); // button
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_PULL_UP);

    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                        OnButtonPressed, NULL);
    while (1)
    {
        printf("waiting key press!!!!!!\n");
        sleep(1);
    }
}

static void StartS3Demo(void)
{
    osThreadAttr_t attr;

    attr.name = "S3Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)StartS3Task, NULL, &attr) == NULL)
    {
        printf("[StartS3Task] Falied to create StartS3Task!\n");
    }
    printf("\r\n[StartS3Task] Succ to create StartS3Task!\n");
}

APP_FEATURE_INIT(StartS3Demo);
