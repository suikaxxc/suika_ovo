#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

int toilet_state = 0;       //卫生间工作状态
int toilet_light_state = 0; //卫生间灯的工作状态

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
}

void toilet_entry(void *arg)
{
    (void)arg;

    init();
    WifiIotGpioValue rel = 0;
    while (1)
    {

        //读取人体红外传感器，
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_7, &rel);

        toilet_state = rel;
        //读取光敏电阻
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_9, &rel);

        //如果有人
        if (toilet_state)
        {

            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, (int)rel);//高电平
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, (int)rel);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, (int)rel);
            toilet_light_state = rel;
        }
        else
        {
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_VALUE0);//低电平
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_VALUE0);
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_VALUE0);
            toilet_light_state = 0;
        }

        sleep(1);
    }
}

void toilet_task(void)
{

    osThreadAttr_t attr;
    attr.name = "toilet_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew(toilet_entry, NULL, &attr) == NULL)
    {
        printf("[toilet_entry] Falied to create toilet_entry!\n");
    }
}
