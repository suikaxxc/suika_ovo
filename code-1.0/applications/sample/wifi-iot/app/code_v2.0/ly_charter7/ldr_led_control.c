#include <stdio.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "ldr_demo.h"
#include "ldr_led_control.h"

#define LDR_LED_TASK_STACK_SIZE 2048
#define LDR_LED_THRESHOLD 30
/* 修改为 GPIO8，避免与蜂鸣器（PWM0/GPIO9）冲突 */
#define LDR_LED_GPIO WIFI_IOT_GPIO_IDX_8
#define LDR_LED_IO_NAME WIFI_IOT_IO_NAME_GPIO_8
#define LDR_LED_FUNC WIFI_IOT_IO_FUNC_GPIO_8_GPIO

static volatile int g_ldr_led_on = 0;

int LdrLed_GetState(void)
{
    return g_ldr_led_on;
}

static void LdrLed_SetOutput(int on)
{
    g_ldr_led_on = on ? 1 : 0;
    /* 
       修改为低电平点亮（Active Low），适配部分硬件连接方式
       on=1 -> Output 0 (LED ON)
       on=0 -> Output 1 (LED OFF)
    */
    GpioSetOutputVal(LDR_LED_GPIO, on ? WIFI_IOT_GPIO_VALUE0 : WIFI_IOT_GPIO_VALUE1);
}

static void LdrLed_Task(void *arg)
{
    (void)arg;
    GpioInit();

    IoSetFunc(LDR_LED_IO_NAME, LDR_LED_FUNC);
    GpioSetDir(LDR_LED_GPIO, WIFI_IOT_GPIO_DIR_OUT);
    LdrLed_SetOutput(0);

    while (1)
    {
        int lightPercent = Get_LightPercent();
        int turnOn = (lightPercent < LDR_LED_THRESHOLD);
        printf("[LdrLed] light=%d%% -> %s\n", lightPercent, turnOn ? "ON" : "OFF");
        LdrLed_SetOutput(turnOn);
        sleep(1);
    }
}

void LdrLed_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "LdrLed_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LDR_LED_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)LdrLed_Task, NULL, &attr) == NULL)
    {
        printf("[LdrLedDemo] Failed to create LdrLed task!\n");
    }
}
