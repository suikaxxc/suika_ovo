/* yw01_demo.c
   视棠因YW01液位传感器采集演示，将ADC原始值转换为百分比并提供查询接口。
   假设YW01信号转化板输出为0~3.3V模拟信号，接到ADC通道。
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

#include "yw01_demo.h"

#define YW01_TASK_STACK_SIZE 4096
#define YW01_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_3 // 改为ADC3(GPIO07)，避免与I2C(GPIO13)冲突

static unsigned short g_yw01_raw = 0;


// 假设YW01信号板输出0~3.3V，ADC 0~4095，满量程对应实际液位高度（如100mm），可根据实际调整
#define YW01_MAX_MM 100
int Get_YW01Mm(void)
{
    unsigned int raw = g_yw01_raw;
    const unsigned int max = 4095;
    if (raw > max) raw = max;
    // 线性映射到毫米
    return (int)((raw * YW01_MAX_MM) / max);
}

static void YW01_Task(void *arg)
{
    (void)arg;
    GpioInit();
    while (1)
    {
        if (AdcRead(YW01_SENSOR_CHAN_NAME, &g_yw01_raw, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            int mm = Get_YW01Mm();
            printf("yw01 raw:%u level:%dmm\n", g_yw01_raw, mm);
        }
        sleep(1);
    }
}

void YW01_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "YW01_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = YW01_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)YW01_Task, NULL, &attr) == NULL)
    {
        printf("[YW01Demo] Failed to create YW01 task!\n");
    }
}
