#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

unsigned short data = 0; // 保存读取到的MQ2值

static void mq2_task_run(void *arg)
{
    (void)arg;
    GpioInit();
    while (1)
    {
        // AdcRead函数用于读取指定通道的ADC值。
        // 参数GAS_SENSOR_CHAN_NAME，表示ADC通道索引，用于指定要读取数据的ADC通道。
        // 参数data，表示指向存储读取数据的地址的指针，用于接收ADC转换后的数据。
        // 参数WIFI_IOT_ADC_EQU_MODEL_4，表示方程模型，用于指定数据转换时使用的数学模型。
        // 参数WIFI_IOT_ADC_CUR_BAIS_DEFAULT，表示模拟电源控制模式，用于设置ADC的模拟电源工作状态。
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            printf("gas:%d ppm\n", data);
        }

        sleep(1);
    }
}

static void mq2_task(void)
{
    osThreadAttr_t attr;
    attr.name = "MQ2Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;
    if (osThreadNew(mq2_task_run, NULL, &attr) == NULL)
    {
        printf("[EnvironmentDemo] Falied to create MQ2Task!\n");
    }
}

SYS_RUN(mq2_task);