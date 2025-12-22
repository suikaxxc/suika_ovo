#include <stdio.h>
#include "ohos_init.h"
#include <unistd.h>
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "ohos_types.h"
void leddemo(void)
{
    //1.初始化GPIO设备
    GpioInit(); 

    //2.set gpio-09 func:GPIO 
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);

    //3.set gpio-09 dir:OUT
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);

    //4.set gpio-09 Value:0  led on
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE0);
    printf("led 点亮\n");

    usleep(4000000);
    //4.set gpio-09 Value:1  led off
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_VALUE1);
    printf("led 熄灭\n"); 
}
SYS_RUN(leddemo);
