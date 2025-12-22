#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

int isHuman = 0;
int isCall = 0;

static void OnButtonPressed(char *arg)
{
    (void)arg;
    isCall = 60;
    printf("isHuman %disCall\r\n", isCall);
}

void init(void)
{
    GpioInit();
    //设置红色,蓝 色,绿色 LED IO为输出状态
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_OUT);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);

    //光敏电阻
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_IN);

    //人体红外感应
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_PULL_UP);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO); // button
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                        OnButtonPressed, NULL);
}

void access_control_entry(void *arg)
{
    (void)arg;

    init();
    WifiIotGpioValue rel = 0;
    while (1)
    {

        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_7, &rel);

        isHuman = rel;
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_9, &rel);
        if (isHuman)//光明电阻  ，感应灯晚上有人则亮，无人则不亮
        {

            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, (int)rel);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, (int)rel);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, (int)rel);
        }
        else
        {
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_VALUE0);
        }

        sleep(1);
        isCall--;
    }
}

void access_control_task(void)
{

    osThreadAttr_t attr;
    attr.name = "access_control_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(access_control_entry, NULL, &attr) == NULL)
    {
        printf("[access_control_task] Falied to create access_control_task!\n");
    }
}
