/**
 * @file turbidity_sensor.c
 * @brief Turbidity sensor implementation for aquatic plant tank
 * Uses ADC1 for turbidity measurement, output in NTU
 */

#include <stdint.h>

#include "wifiiot_adc.h"
#include "wifiiot_errno.h"

#include "turbidity_sensor.h"

#define TURBIDITY_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_1
#define ADC_MAX_VALUE 4095
#define SAMPLE_COUNT 15
#define TURBIDITY_MAX_NTU 1000

static unsigned short g_sample_buffer[SAMPLE_COUNT];
static int g_sample_index = 0;
static int g_samples_collected = 0;
static unsigned short g_turbidity_raw = 0;
static int g_turbidity_ntu = 0;

static void BubbleSort(unsigned short arr[], int len)
{
    int i;
    int j;
    unsigned short temp;

    for (j = 0; j < len - 1; j++)
    {
        for (i = 0; i < len - j - 1; i++)
        {
            if (arr[i] > arr[i + 1])
            {
                temp = arr[i];
                arr[i] = arr[i + 1];
                arr[i + 1] = temp;
            }
        }
    }
}

static unsigned short GetMedian(void)
{
    if (g_samples_collected == 0)
    {
        return 0;
    }

    int count = (g_samples_collected < SAMPLE_COUNT) ? g_samples_collected : SAMPLE_COUNT;
    unsigned short temp_buffer[SAMPLE_COUNT];
    int i;

    for (i = 0; i < count; i++)
    {
        temp_buffer[i] = g_sample_buffer[i];
    }

    BubbleSort(temp_buffer, count);

    if (count % 2 != 0)
    {
        return temp_buffer[(count - 1) / 2];
    }

    return (temp_buffer[count / 2] + temp_buffer[count / 2 - 1]) / 2;
}

void Turbidity_CollectSample(void)
{
    unsigned short raw_value = 0;

    if (AdcRead(TURBIDITY_ADC_CHANNEL, &raw_value,
                WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) == WIFI_IOT_SUCCESS)
    {
        g_sample_buffer[g_sample_index] = raw_value;
        g_sample_index++;
        if (g_sample_index >= SAMPLE_COUNT)
        {
            g_sample_index = 0;
        }
        if (g_samples_collected < SAMPLE_COUNT)
        {
            g_samples_collected++;
        }
    }
}

void Turbidity_Update(void)
{
    Turbidity_CollectSample();
    g_turbidity_raw = GetMedian();

    int ntu = (g_turbidity_raw * TURBIDITY_MAX_NTU) / ADC_MAX_VALUE;
    if (ntu > TURBIDITY_MAX_NTU)
    {
        ntu = TURBIDITY_MAX_NTU;
    }
    g_turbidity_ntu = ntu;
}

int Get_TurbidityValue(void)
{
    return g_turbidity_ntu;
}

unsigned short Get_TurbidityRaw(void)
{
    return g_turbidity_raw;
}
