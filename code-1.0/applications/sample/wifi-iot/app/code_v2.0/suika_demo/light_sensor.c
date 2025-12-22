/**
 * @file light_sensor.c
 * @brief Light sensor (LDR) implementation for aquatic plant tank
 * Uses ADC0 (GPIO12) for light sensing
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

#include "light_sensor.h"

#define LIGHT_SENSOR_TASK_STACK_SIZE 2048
#define LIGHT_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_0  // GPIO12/ADC0

static unsigned short g_light_raw = 0;

int Get_LightIntensity(void)
{
    unsigned int raw = g_light_raw;
    const unsigned int max = 4095;
    if (raw > max) raw = max;
    // Higher ADC value = brighter (inverted from typical LDR)
    return (int)((raw * 100u) / max);
}

unsigned short Get_LightRaw(void)
{
    return g_light_raw;
}

static void LightSensor_Task(void *arg)
{
    (void)arg;
    GpioInit();

    while (1)
    {
        if (AdcRead(LIGHT_ADC_CHANNEL, &g_light_raw,
                    WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            int pct = Get_LightIntensity();
            printf("[Light] raw:%u pct:%d%%\n", g_light_raw, pct);
        }
        sleep(2);
    }
}

void LightSensor_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "LightSensor_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LIGHT_SENSOR_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)LightSensor_Task, NULL, &attr) == NULL)
    {
        printf("[LightSensor] Failed to create task!\n");
    }
}
