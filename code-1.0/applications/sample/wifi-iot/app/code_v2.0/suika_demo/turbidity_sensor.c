/**
 * @file turbidity_sensor.c
 * @brief AZDM01 turbidity sensor implementation for aquatic plant tank
 * Uses ADC1 (GPIO01) for turbidity measurement
 *
 * Sensor output: analog voltage (typically 0.5V ~ 4.5V)
 * Formula: NTU = -125 * Vout + 625
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

#include "turbidity_sensor.h"

#define TURBIDITY_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_1  // GPIO01/ADC1

// Hi3861 ADC parameters
#define ADC_VREF_V 1.8f
#define ADC_MAX_VALUE 4095.0f

// AZDM01 sensor parameters
#define AZDM01_MIN_VOUT 0.5f
#define AZDM01_MAX_VOUT 4.5f

// Voltage divider reconstruction ratio for mapping ADC pin voltage back to sensor Vout.
// ratio = Vout / Vadc = (R1 + R2) / R2 (R1: upper resistor, R2: lower resistor to GND)
// This value MUST match the actual resistor divider on hardware.
// Current configuration assumes 4.5V -> 1.8V at ADC pin, so ratio = 4.5 / 1.8 = 2.5.
// One matching example is R1=15kΩ and R2=10kΩ -> (15k+10k)/10k = 2.5.
#define TURBIDITY_DIVIDER_RATIO 2.5f

// Simple averaging for stable ADC reading
#define TURBIDITY_SAMPLE_COUNT 8

static unsigned short g_turbidity_raw = 0;
static float g_turbidity_vout = AZDM01_MAX_VOUT;
static int g_turbidity_ntu = 0;

static int CalculateNTU(float vout)
{
    // Formula from requirement: NTU = -125 * Vout + 625
    float ntu = -125.0f * vout + 625.0f;

    if (ntu < 0.0f) ntu = 0.0f;
    if (ntu > 1000.0f) ntu = 1000.0f;

    return (int)(ntu + 0.5f);
}

int Get_TurbidityNTU(void)
{
    return g_turbidity_ntu;
}

float Get_TurbidityVoltage(void)
{
    return g_turbidity_vout;
}

unsigned short Get_TurbidityRaw(void)
{
    return g_turbidity_raw;
}

void Turbidity_Update(void)
{
    unsigned int sum = 0;
    int validSamples = 0;
    int i;

    for (i = 0; i < TURBIDITY_SAMPLE_COUNT; i++) {
        unsigned short raw = 0;
        if (AdcRead(TURBIDITY_ADC_CHANNEL, &raw,
                    WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS) {
            sum += raw;
            validSamples++;
        }
    }

    if (validSamples == 0) {
        return;
    }

    g_turbidity_raw = (unsigned short)(sum / (unsigned int)validSamples);

    // Convert ADC reading to ADC pin voltage, then reconstruct sensor output voltage
    float adcVoltage = ((float)g_turbidity_raw / ADC_MAX_VALUE) * ADC_VREF_V;
    float vout = adcVoltage * TURBIDITY_DIVIDER_RATIO;

    // Clamp to specified sensor range
    if (vout < AZDM01_MIN_VOUT) vout = AZDM01_MIN_VOUT;
    if (vout > AZDM01_MAX_VOUT) vout = AZDM01_MAX_VOUT;

    g_turbidity_vout = vout;
    g_turbidity_ntu = CalculateNTU(vout);
}

void Turbidity_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
