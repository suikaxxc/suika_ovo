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

// LDR characteristics (calibrated for typical photoresistor GL5528)
// Reference point: 10 lux = 10k ohms resistance
// Gamma coefficient typically 0.7-0.8 for common LDRs
#define LDR_REFERENCE_LUX 10.0f       // Reference illuminance
#define LDR_REFERENCE_RESISTANCE 10000.0f  // Resistance at reference lux
#define LDR_GAMMA 0.7f                // LDR gamma coefficient
#define PULL_UP_RESISTANCE 10000.0f   // Pull-up resistor value in ohms

static unsigned short g_light_raw = 0;
static int g_light_lux = 0;

/**
 * @brief Convert ADC value to lux using LDR formula
 * 
 * LDR resistance-illuminance relationship:
 *   R_ldr = R_ref * (Lux_ref / Lux)^gamma
 * Rearranging to solve for lux:
 *   Lux = Lux_ref * (R_ref / R_ldr)^(1/gamma)
 * 
 * For GL5528 LDR: R = ~10k ohms at 10 lux, gamma ~= 0.7
 */
static int AdcToLux(unsigned short adcValue)
{
    if (adcValue == 0) return 0;
    if (adcValue >= ADC_MAX_VALUE) return 2000;  // Clamp to max reasonable indoor
    
    // Calculate voltage from ADC (Hi3861 ADC reference is 1.8V)
    float voltage = (float)adcValue * ADC_VREF_MV / ADC_MAX_VALUE;
    
    // Calculate LDR resistance using voltage divider formula
    // Circuit: VCC --[Pull-up R]-- ADC_PIN --[LDR]-- GND
    // V_adc = Vcc * R_ldr / (R_pullup + R_ldr)
    // Rearranging: R_ldr = R_pullup * V_adc / (Vcc - V_adc)
    
    float vcc = (float)ADC_VREF_MV;  // Use ADC reference voltage
    float voltageDiff = vcc - voltage;
    
    // Avoid division by zero
    if (voltageDiff < 1.0f) voltageDiff = 1.0f;
    
    float ldrResistance = PULL_UP_RESISTANCE * voltage / voltageDiff;
    
    // Avoid invalid resistance values
    if (ldrResistance < 100.0f) ldrResistance = 100.0f;      // Minimum ~bright light
    if (ldrResistance > 100000.0f) ldrResistance = 100000.0f; // Maximum ~darkness
    
    // Calculate lux using LDR formula:
    // Lux = Lux_ref * (R_ref / R_ldr)^(1/gamma)
    // 
    // Using power function approximation since we don't have math.h:
    // For (R_ref/R_ldr)^(1/gamma) where gamma=0.7, exponent = 1/0.7 ≈ 1.43
    // 
    // Simplified calculation with linear interpolation for typical range:
    // At R = 50k ohms -> ~1 lux (very dark)
    // At R = 10k ohms -> ~10 lux (dim)
    // At R = 1k ohms  -> ~200 lux (indoor light)
    // At R = 500 ohms -> ~500 lux (bright indoor)
    // At R = 200 ohms -> ~1000 lux (very bright)
    
    float ratio = LDR_REFERENCE_RESISTANCE / ldrResistance;
    
    // Calculate lux using approximation of power function
    // lux ≈ 10 * ratio^1.43
    // Approximate: ratio^1.43 ≈ ratio * sqrt(ratio) for practical range
    float sqrtRatio;
    if (ratio <= 0.01f) {
        sqrtRatio = 0.1f;
    } else if (ratio >= 100.0f) {
        sqrtRatio = 10.0f;
    } else {
        // Simple Newton-Raphson approximation for sqrt
        sqrtRatio = ratio;
        for (int i = 0; i < 5; i++) {
            sqrtRatio = (sqrtRatio + ratio / sqrtRatio) / 2.0f;
        }
    }
    
    float lux = LDR_REFERENCE_LUX * ratio * sqrtRatio;
    
    // Clamp to reasonable indoor range (target 100-800 lux typically)
    if (lux < 1.0f) lux = 1.0f;
    if (lux > 2000.0f) lux = 2000.0f;
    
    return (int)lux;
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
