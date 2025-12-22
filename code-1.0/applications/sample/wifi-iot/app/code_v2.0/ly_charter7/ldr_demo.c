/* ldr_demo.c
   简单的光敏传感器（LDR）读取演示，将 ADC 原始值转换为百分比并提供查询接口。
*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "ldr_demo.h"

#define LDR_TASK_STACK_SIZE 4096
#define LIGHT_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_0

static unsigned short g_light_raw = 0;

int Get_LightPercent(void)
{
    unsigned int raw = g_light_raw;
    const unsigned int max = 4095; /* 假设 12-bit ADC */
    if (raw > max) raw = max;
    return (int)((raw * 100u) / max);
}

static void Ldr_Task(void *arg)
{
    (void)arg;

    GpioInit();

    while (1)
    {
        if (AdcRead(LIGHT_SENSOR_CHAN_NAME, &g_light_raw, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            int pct = Get_LightPercent();
            printf("light raw:%u pct:%d%%\n", g_light_raw, pct);
        }
        sleep(1);
    }
}

void Ldr_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "Ldr_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LDR_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)Ldr_Task, NULL, &attr) == NULL)
    {
        printf("[LdrDemo] Failed to create Ldr task!\n");
    }
}
