/**
 * @file water_level.c
 * @brief Water level sensor (YW01) implementation for aquatic plant tank
 * Uses ADC3 (GPIO07) for water level sensing
 * 
 * YW01 Sensor Specifications:
 * - Range: 0-90mm
 * - Output: 0-1.0V analog (corresponds to 0%-100% of range)
 * - Hi3861 ADC: 12-bit (0-4095), 1.8V reference
 * - 1.0V input corresponds to ~2275 ADC value (1.0V / 1.8V * 4095)
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

// YW01 sensor parameters
#define YW01_MAX_MM 90           // Maximum water level in mm
#define YW01_MAX_VOLTAGE_MV 1000 // Maximum output voltage (1.0V = 1000mV)
#define ADC_VREF_MV 1800         // Hi3861 ADC reference voltage (1.8V = 1800mV)
#define ADC_MAX_VALUE 4095       // 12-bit ADC max value

// Calculate ADC value corresponding to YW01 max output (1.0V)
// ADC_at_1V = (1000mV / 1800mV) * 4095 = ~2275
#define YW01_MAX_ADC_VALUE ((YW01_MAX_VOLTAGE_MV * ADC_MAX_VALUE) / ADC_VREF_MV)

static unsigned short g_water_level_raw = 0;

int Get_WaterLevelMM(void)
{
    unsigned int raw = g_water_level_raw;
    
    // Clamp to max expected ADC value for 1.0V input
    if (raw > YW01_MAX_ADC_VALUE) raw = YW01_MAX_ADC_VALUE;
    
    // Convert ADC value to mm: (raw / YW01_MAX_ADC_VALUE) * YW01_MAX_MM
    return (int)((raw * YW01_MAX_MM) / YW01_MAX_ADC_VALUE);
}

int Get_WaterLevelPercent(void)
{
    // Keep for backward compatibility - convert mm to percentage
    int mm = Get_WaterLevelMM();
    return (mm * 100) / YW01_MAX_MM;
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
