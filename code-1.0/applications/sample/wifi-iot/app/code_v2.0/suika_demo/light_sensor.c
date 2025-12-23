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

void LightSensor_Update(void)
{
    AdcRead(LIGHT_ADC_CHANNEL, &g_light_raw,
            WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
}

void LightSensor_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
