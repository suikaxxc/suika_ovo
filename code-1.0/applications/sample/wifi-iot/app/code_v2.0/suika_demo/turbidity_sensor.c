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
// Hi3861 ADC reference voltage (max measurable analog level)
#define ADC_VREF_VOLTS 1.8f
// Sensor module supply voltage (user wiring is 5V side)
#define TURBIDITY_SENSOR_SUPPLY_VOLTS 5.0f
// AO->ADC divider ratio:
//   adc_voltage = sensor_ao_voltage / ratio
// default 2.0 means e.g. 10k:10k divider from 0~5V to 0~2.5V
// IMPORTANT: Hi3861 ADC max is 1.8V, so hardware should ensure adc_voltage <= 1.8V.
#define TURBIDITY_VOLTAGE_DIVIDER_RATIO 2.0f

// Piecewise calibration points (sensor AO voltage on 5V supply side)
// Higher voltage => clearer water (lower NTU).
#define TURBIDITY_VOLTAGE_CLEAR_HIGH 3.5f
#define TURBIDITY_VOLTAGE_MID        3.0f
#define TURBIDITY_VOLTAGE_TURBID     2.5f

// Corresponding NTU anchors
#define TURBIDITY_NTU_CLEAR_HIGH 200.0f
#define TURBIDITY_NTU_MID        600.0f
#define TURBIDITY_NTU_TURBID     1000.0f

#define TURBIDITY_MIN_NTU 0
#define TURBIDITY_MAX_NTU 1000

static unsigned short g_turbidity_raw = 0;
static int g_turbidity_ntu = 0;

void Turbidity_CollectSample(void)
{
    // Multi-sample averaging to reduce jitter and accidental spikes.
    uint32_t sum = 0;
    int valid_count = 0;
    int i;
    for (i = 0; i < 8; i++) {
        unsigned short raw_value = 0;
        if (AdcRead(TURBIDITY_ADC_CHANNEL, &raw_value,
                    WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
        {
            sum += raw_value;
            valid_count++;
        }
    }

    if (valid_count > 0) {
        g_turbidity_raw = (unsigned short)(sum / (uint32_t)valid_count);
    }
}

static float ClampFloat(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static float Lerp(float x, float x0, float y0, float x1, float y1)
{
    if (x1 <= x0) return y0;
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

void Turbidity_Update(void)
{
    Turbidity_CollectSample();

    // Convert ADC raw to ADC pin voltage (0~1.8V).
    float adc_voltage = (float)g_turbidity_raw * ADC_VREF_VOLTS / (float)ADC_MAX_VALUE;

    // Reconstruct sensor AO voltage (5V side) using divider ratio.
    float sensor_voltage = adc_voltage * TURBIDITY_VOLTAGE_DIVIDER_RATIO;
    sensor_voltage = ClampFloat(sensor_voltage, 0.0f, TURBIDITY_SENSOR_SUPPLY_VOLTS);

    float ntu;
    if (sensor_voltage >= TURBIDITY_VOLTAGE_CLEAR_HIGH) {
        // Very clear region: continue linearly down toward 0 NTU at near-full voltage.
        ntu = Lerp(sensor_voltage, TURBIDITY_VOLTAGE_CLEAR_HIGH, TURBIDITY_NTU_CLEAR_HIGH,
                   TURBIDITY_SENSOR_SUPPLY_VOLTS, 0.0f);
    } else if (sensor_voltage >= TURBIDITY_VOLTAGE_MID) {
        ntu = Lerp(sensor_voltage, TURBIDITY_VOLTAGE_MID, TURBIDITY_NTU_MID,
                   TURBIDITY_VOLTAGE_CLEAR_HIGH, TURBIDITY_NTU_CLEAR_HIGH);
    } else if (sensor_voltage >= TURBIDITY_VOLTAGE_TURBID) {
        ntu = Lerp(sensor_voltage, TURBIDITY_VOLTAGE_TURBID, TURBIDITY_NTU_TURBID,
                   TURBIDITY_VOLTAGE_MID, TURBIDITY_NTU_MID);
    } else {
        // Very turbid region below 2.5V, clamp near top range.
        ntu = TURBIDITY_MAX_NTU;
    }

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
