/**
 * @file water_level.c
 * @brief Water level sensor (YW001) implementation for aquatic plant tank
 * Uses ADC3 (GPIO07) for water level sensing
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

#include "water_level.h"

#define WATER_LEVEL_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_3  // GPIO07/ADC3

static unsigned short g_water_level_raw = 0;

int Get_WaterLevelPercent(void)
{
    unsigned int raw = g_water_level_raw;
    const unsigned int max = 4095;
    if (raw > max) raw = max;
    return (int)((raw * 100u) / max);
}

unsigned short Get_WaterLevelRaw(void)
{
    return g_water_level_raw;
}

void WaterLevel_Update(void)
{
    AdcRead(WATER_LEVEL_ADC_CHANNEL, &g_water_level_raw,
            WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
}

void WaterLevel_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
