#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#define ADC_TASK_STACK_SIZE 4096
#define GAS_SENSOR_CHAN_NAME WIFI_IOT_ADC_CHANNEL_5

unsigned short data = 0;

unsigned short Get_Mq2Value(void)
{
    return data;
}

static void ADC_Task(void *arg)
{
    (void)arg;

    GpioInit();

    while (1)
    {
        if (AdcRead(GAS_SENSOR_CHAN_NAME, &data, WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            printf("gas:%d ppm\n", data);
        }
        sleep(1);
    }
}

void Mq2_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "ADC_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ADC_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    if (osThreadNew((osThreadFunc_t)ADC_Task, NULL, &attr) == NULL)
    {
        printf("[EnvironmentDemo] Falied to create EnvironmentTask!\n");
    }
}