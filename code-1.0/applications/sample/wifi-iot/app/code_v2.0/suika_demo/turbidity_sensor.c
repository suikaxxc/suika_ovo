/**
 * @file turbidity_sensor.c
 * @brief Turbidity sensor implementation for aquatic plant tank
 * Uses ADC1 analog input for turbidity measurement, output in NTU
 */

#include <stdint.h>

#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "turbidity_sensor.h"

#define TURBIDITY_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_1
#define ADC_MAX_VALUE 4095
#define ADC_VREF_V 1.8f
#define TURBIDITY_FORMULA_VOLTAGE_OFFSET 0.5f
#define TURBIDITY_FORMULA_VOLTAGE_RANGE 4.0f
#define TURBIDITY_MIN_NTU 0
#define TURBIDITY_MAX_NTU 1000

static unsigned short g_turbidity_raw = 0;
static int g_turbidity_ntu = 0;

void Turbidity_CollectSample(void)
{
    unsigned short raw_value = 0;
    if (AdcRead(TURBIDITY_ADC_CHANNEL, &raw_value,
                WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
    {
        g_turbidity_raw = raw_value;
    }
}

void Turbidity_Update(void)
{
    Turbidity_CollectSample();

    // Convert ADC raw to voltage.
    float voltage = (float)g_turbidity_raw * ADC_VREF_V / (float)ADC_MAX_VALUE;

    // User-provided linear conversion:
    // NTU = 1000 * (1 - (Voltage - 0.5) / 4.0)
    float ntu = 1000.0f * (1.0f - (voltage - TURBIDITY_FORMULA_VOLTAGE_OFFSET) / TURBIDITY_FORMULA_VOLTAGE_RANGE);

    if (ntu < TURBIDITY_MIN_NTU)
    {
        ntu = TURBIDITY_MIN_NTU;
    }
    if (ntu > TURBIDITY_MAX_NTU)
    {
        ntu = TURBIDITY_MAX_NTU;
    }

    g_turbidity_ntu = (int)(ntu + 0.5f);
}

int Get_TurbidityValue(void)
{
    return g_turbidity_ntu;
}

unsigned short Get_TurbidityRaw(void)
{
    return g_turbidity_raw;
}
