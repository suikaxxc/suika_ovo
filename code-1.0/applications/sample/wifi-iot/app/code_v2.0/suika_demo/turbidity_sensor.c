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
#define TURBIDITY_MIN_NTU 0
#define TURBIDITY_MAX_NTU 1000
#define TURBIDITY_MIN_CALIB_RANGE_RAW 180
#define TURBIDITY_DEFAULT_CLEAR_RAW 3200
#define TURBIDITY_DEFAULT_TURBID_RAW 1800

static unsigned short g_turbidity_raw = 0;
static int g_turbidity_ntu = 0;
static unsigned short g_calib_clear_raw = TURBIDITY_DEFAULT_CLEAR_RAW;
static unsigned short g_calib_turbid_raw = TURBIDITY_DEFAULT_TURBID_RAW;
static int g_turbidity_initialized = 0;

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

static int ClampInt(int value, int min, int max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static void Turbidity_UpdateCalibration(unsigned short raw)
{
    if (!g_turbidity_initialized) {
        int init_clear = (int)raw + TURBIDITY_MIN_CALIB_RANGE_RAW;
        int init_turbid = (int)raw - TURBIDITY_MIN_CALIB_RANGE_RAW;
        g_calib_clear_raw = (unsigned short)ClampInt(init_clear, 0, ADC_MAX_VALUE);
        g_calib_turbid_raw = (unsigned short)ClampInt(init_turbid, 0, ADC_MAX_VALUE);
        g_turbidity_initialized = 1;
    }

    // Fast track newly observed extrema.
    if (raw > g_calib_clear_raw) {
        g_calib_clear_raw = raw;
    } else {
        // Slow decay to follow long-term drift.
        g_calib_clear_raw = (unsigned short)(g_calib_clear_raw - (g_calib_clear_raw - raw) / 64);
    }

    if (raw < g_calib_turbid_raw) {
        g_calib_turbid_raw = raw;
    } else {
        // Slow rise to follow long-term drift.
        g_calib_turbid_raw = (unsigned short)(g_calib_turbid_raw + (raw - g_calib_turbid_raw) / 64);
    }

    // Keep a minimum mapping span to avoid stuck values.
    if ((int)g_calib_clear_raw - (int)g_calib_turbid_raw < TURBIDITY_MIN_CALIB_RANGE_RAW) {
        int center = ((int)g_calib_clear_raw + (int)g_calib_turbid_raw) / 2;
        int half = TURBIDITY_MIN_CALIB_RANGE_RAW / 2;
        g_calib_clear_raw = (unsigned short)ClampInt(center + half, 0, ADC_MAX_VALUE);
        g_calib_turbid_raw = (unsigned short)ClampInt(center - half, 0, ADC_MAX_VALUE);
    }
}

void Turbidity_Update(void)
{
    int span;
    int ntu;

    Turbidity_CollectSample();
    Turbidity_UpdateCalibration(g_turbidity_raw);

    // Adaptive raw mapping:
    // clear water  -> higher ADC raw -> lower NTU
    // turbid water -> lower ADC raw  -> higher NTU
    span = (int)g_calib_clear_raw - (int)g_calib_turbid_raw;
    if (span < TURBIDITY_MIN_CALIB_RANGE_RAW) {
        span = TURBIDITY_MIN_CALIB_RANGE_RAW;
    }

    if (g_turbidity_raw >= g_calib_clear_raw) {
        ntu = TURBIDITY_MIN_NTU;
    } else if (g_turbidity_raw <= g_calib_turbid_raw) {
        ntu = TURBIDITY_MAX_NTU;
    } else {
        ntu = ((int)g_calib_clear_raw - (int)g_turbidity_raw) * TURBIDITY_MAX_NTU / span;
    }
    ntu = ClampInt(ntu, TURBIDITY_MIN_NTU, TURBIDITY_MAX_NTU);

    // Exponential smoothing for stable display.
    if (g_turbidity_ntu == 0) {
        g_turbidity_ntu = ntu;
    } else {
        g_turbidity_ntu = (g_turbidity_ntu * 3 + ntu) / 4;
    }
}

int Get_TurbidityValue(void)
{
    return g_turbidity_ntu;
}

unsigned short Get_TurbidityRaw(void)
{
    return g_turbidity_raw;
}
