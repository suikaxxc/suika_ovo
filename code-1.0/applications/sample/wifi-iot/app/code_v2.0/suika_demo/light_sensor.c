/**
 * @file light_sensor.c
 * @brief Light sensor (LDR/BH1750) implementation for aquatic plant tank
 * Uses ADC0 (GPIO12) for light sensing
 * Returns light intensity in lux
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

// ADC reference voltage (mV) and resolution
#define ADC_VREF_MV 1800
#define ADC_MAX_VALUE 4095

// LDR characteristics (approximate for typical photoresistor)
// These values may need calibration for specific LDR
#define LDR_RESISTANCE_DARK 50000   // Dark resistance in ohms
#define LDR_RESISTANCE_BRIGHT 500   // Bright light resistance in ohms
#define PULL_UP_RESISTANCE 10000    // Pull-up resistor value in ohms

static unsigned short g_light_raw = 0;
static int g_light_lux = 0;

/**
 * @brief Convert ADC value to lux using approximate LDR formula
 * Based on typical photoresistor characteristics
 */
static int AdcToLux(unsigned short adcValue)
{
    if (adcValue == 0) return 0;
    if (adcValue >= ADC_MAX_VALUE) return 10000;  // Clamp to max
    
    // Calculate voltage from ADC
    float voltage = (float)adcValue * ADC_VREF_MV / ADC_MAX_VALUE;
    
    // Calculate LDR resistance using voltage divider formula
    // Assuming LDR is connected between ADC pin and GND with pull-up to VCC
    // V_adc = Vcc * R_ldr / (R_pullup + R_ldr)
    // Rearranging: R_ldr = R_pullup * V_adc / (Vcc - V_adc)
    
    float vcc = 3300.0f;  // 3.3V supply
    if (voltage >= vcc) return 10000;
    
    float ldrResistance = PULL_UP_RESISTANCE * voltage / (vcc - voltage);
    
    // Convert resistance to lux using approximate logarithmic relationship
    // log(lux) = (log(R_dark) - log(R_ldr)) / gamma
    // Typical gamma for LDR is around 0.7
    if (ldrResistance <= 0) return 10000;
    
    float gamma = 0.7f;
    float logRatio = (float)(LDR_RESISTANCE_DARK) / ldrResistance;
    if (logRatio <= 0) return 0;
    
    // Simplified lux calculation
    // lux = 10^((log10(R_dark/R_ldr)) / gamma)
    // Using approximation: higher ADC = more light = higher lux
    int lux = (int)(logRatio * 100.0f / gamma);
    
    // Clamp to reasonable range (0-10000 lux for indoor/outdoor)
    if (lux < 0) lux = 0;
    if (lux > 10000) lux = 10000;
    
    return lux;
}

int Get_LightIntensity(void)
{
    return g_light_lux;
}

unsigned short Get_LightRaw(void)
{
    return g_light_raw;
}

void LightSensor_Update(void)
{
    if (AdcRead(LIGHT_ADC_CHANNEL, &g_light_raw,
            WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
    {
        g_light_lux = AdcToLux(g_light_raw);
    }
}

void LightSensor_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
