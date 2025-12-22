#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "hi_pwm.h"

#include "mq2_demo.h"

void init(void)
{
    GpioInit();

    // 蜂鸣器引脚 设置为 PWM功能
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);
}

static void Beep_Work(void)
{
    uint16_t freqDivisor = 34052;
    // 占空比，频率
    PwmStart(WIFI_IOT_PWM_PORT_PWM0, freqDivisor / 2, freqDivisor);
    usleep(2000000);
    PwmStop(WIFI_IOT_PWM_PORT_PWM0);
    usleep(1000000);
}

void BeeperDemo(void)
{
    init();
    while (1)
    {
        if (Get_Mq2Value() > 900)
        {
            // printf("mq2_demo = %d\n", Get_Mq2Value());
            Beep_Work();
        }
        sleep(1);
    }
}

void Beep_Task(void)
{
    osThreadAttr_t attr;

    attr.name = "BeeperDemo";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)BeeperDemo, NULL, &attr) == NULL)
    {
        printf("[BeeperDemo] Falied to create BeeperMusicTask!\n");
    }
}