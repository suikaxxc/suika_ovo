/**
 * @file tds_sensor.c
 * @brief TDS water quality sensor implementation for aquatic plant tank
 * Uses ADC5 (GPIO11) for TDS measurement
 * 
 * Based on DFRobot Gravity TDS Sensor (SEN0244):
 * - Output voltage range: 0 ~ 2.3V
 * - Measurement range: 0 ~ 1000ppm
 * 
 * Hi3861 ADC: 12-bit (0-4095), 1.8V reference
 * Note: TDS sensor max output (2.3V) exceeds ADC reference (1.8V)
 *       So we can only measure up to ~78% of the sensor range
 *       ADC max (1.8V) corresponds to ~780ppm
 * 
 * Uses median filtering for stable readings (following GravityTDSExample.ino)
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

#include "tds_sensor.h"
#include "ds18b20.h"

#define TDS_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_5  // GPIO11/ADC5

// Hi3861 ADC parameters
#define ADC_VREF_MV 1800         // Hi3861 ADC reference voltage (1.8V = 1800mV)
#define ADC_MAX_VALUE 4095       // 12-bit ADC max value

// Median filter parameters (similar to GravityTDSExample.ino)
#define SAMPLE_COUNT 30
static unsigned short g_sample_buffer[SAMPLE_COUNT];
static int g_sample_index = 0;
static int g_samples_collected = 0;

static unsigned short g_tds_raw = 0;
static int g_tds_ppm = 0;

int Get_TDSValue(void)
{
    return g_tds_ppm;
}

unsigned short Get_TDSRaw(void)
{
    return g_tds_raw;
}

/**
 * @brief Bubble sort for median calculation
 */
static void BubbleSort(unsigned short arr[], int len)
{
    int i, j;
    unsigned short temp;
    for (j = 0; j < len - 1; j++) {
        for (i = 0; i < len - j - 1; i++) {
            if (arr[i] > arr[i + 1]) {
                temp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = temp;
            }
        }
    }
}

/**
 * @brief Get median value from sample buffer (following GravityTDSExample.ino)
 */
static unsigned short GetMedian(void)
{
    if (g_samples_collected == 0) return 0;
    
    int count = (g_samples_collected < SAMPLE_COUNT) ? g_samples_collected : SAMPLE_COUNT;
    
    // Copy buffer for sorting
    unsigned short temp_buffer[SAMPLE_COUNT];
    int i;
    for (i = 0; i < count; i++) {
        temp_buffer[i] = g_sample_buffer[i];
    }
    
    // Sort to find median
    BubbleSort(temp_buffer, count);
    
    // Return median
    if ((count & 1) > 0) {
        return temp_buffer[(count - 1) / 2];
    } else {
        return (temp_buffer[count / 2] + temp_buffer[count / 2 - 1]) / 2;
    }
}

/**
 * @brief Calculate TDS value from ADC reading with temperature compensation
 *        Following the formula from GravityTDSExample.ino
 */
static int CalculateTDS(unsigned short adcValue, float temperature)
{
    // Convert ADC value to voltage (Hi3861: 1.8V reference, 12-bit)
    float voltage = (float)adcValue * ADC_VREF_MV / 1000.0f / ADC_MAX_VALUE;

    // Temperature compensation coefficient (standard: 25°C)
    // Formula from GravityTDSExample.ino: 1.0 + 0.02 * (temperature - 25.0)
    float compensationCoeff = 1.0f + 0.02f * (temperature - 25.0f);
    float compensatedVoltage = voltage / compensationCoeff;

    // Convert voltage to TDS using the DFRobot formula
    // tdsValue = (133.42*V^3 - 255.86*V^2 + 857.39*V) * 0.5
    float v = compensatedVoltage;
    float tds = (133.42f * v * v * v - 255.86f * v * v + 857.39f * v) * 0.5f;

    if (tds < 0) tds = 0;
    if (tds > 1000) tds = 1000;

    return (int)tds;
}

/**
 * @brief Collect ADC sample for median filtering
 *        Should be called frequently (e.g., every 40ms like the reference)
 */
void TDS_CollectSample(void)
{
    unsigned short raw_value = 0;
    if (AdcRead(TDS_ADC_CHANNEL, &raw_value,
                WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
    {
        g_sample_buffer[g_sample_index] = raw_value;
        g_sample_index++;
        if (g_sample_index >= SAMPLE_COUNT) {
            g_sample_index = 0;
        }
        if (g_samples_collected < SAMPLE_COUNT) {
            g_samples_collected++;
        }
    }
}

void TDS_Update(void)
{
    // Collect a new sample
    TDS_CollectSample();
    
    // Get median value from samples
    g_tds_raw = GetMedian();
    
    // Calculate TDS with temperature compensation
    float temp = Get_WaterTemperature();
    g_tds_ppm = CalculateTDS(g_tds_raw, temp);
}

void TDS_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
