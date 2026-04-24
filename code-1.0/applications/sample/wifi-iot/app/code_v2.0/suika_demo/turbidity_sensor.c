/**
 * @file turbidity_sensor.c
 * @brief AZDM01 turbidity sensor implementation for aquatic plant tank
 * Uses ADC1 (GPIO01) for turbidity measurement
 *
 * Sensor output: analog voltage (typically 0.5V ~ 4.5V)
 * Read ADC raw first (MQ2-style acquisition), then map voltage to NTU with fixed linear mapping.
 */

#include <stdio.h>
#include <stdint.h>

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
// AZDM01 power supply configuration (hardware uses 5V)
#define AZDM01_SUPPLY_VOLTAGE_V 5.0f

// Voltage divider reconstruction ratio for mapping ADC pin voltage back to sensor Vout.
// ratio = Vout / Vadc = (R1 + R2) / R2 (R1: upper resistor, R2: lower resistor to GND)
// This value MUST match the actual resistor divider on hardware.
// Current configuration assumes 4.5V -> 1.8V at ADC pin, so ratio = 4.5 / 1.8 = 2.5.
// One matching example is R1=15kΩ (from sensor Vout to ADC junction node)
// and R2=10kΩ (from ADC junction node to GND): (15k+10k)/10k = 2.5.
#define TURBIDITY_DIVIDER_RATIO 2.5f

// Simple averaging for stable ADC reading:
// 8 samples provides basic noise suppression while keeping control-loop response fast.
// This choice aligns with periodic control-loop sampling cadence in suika_demo.
#define TURBIDITY_SAMPLE_COUNT 8
// Control loop runs every ~2s; log every 30 updates (~60s) to avoid serial flooding.
#define TURBIDITY_LOG_UPDATE_COUNT 30U
#define TURBIDITY_MIN_NTU 0.0f
#define TURBIDITY_MAX_NTU 1000.0f
// Consider values near ADC full-scale (4095) and near-zero as potential saturation.
// Warn only after persistent saturation across multiple updates to reduce false alarms.
#define TURBIDITY_SAT_HIGH_RAW 4080
#define TURBIDITY_SAT_LOW_RAW 15
#define TURBIDITY_SAT_WARN_COUNT 20U

static unsigned short g_turbidity_raw = 0;
static float g_turbidity_vout = AZDM01_MAX_VOUT;
static int g_turbidity_ntu = 0;
static int g_turbidity_initialized = 0;
static uint32_t g_update_count = 0;
static uint32_t g_adc_high_saturation_count = 0;
static uint32_t g_adc_low_saturation_count = 0;

static int ReadAveragedRaw(unsigned short *rawOut)
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
        return 0;
    }

    *rawOut = (unsigned short)(sum / (unsigned int)validSamples);
    return 1;
}

static int CalculateNTUFromVoltage(float vout)
{
    float ratio = (AZDM01_MAX_VOUT - vout) / (AZDM01_MAX_VOUT - AZDM01_MIN_VOUT);
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    float ntu = TURBIDITY_MIN_NTU + ratio * (TURBIDITY_MAX_NTU - TURBIDITY_MIN_NTU);
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
    if (!g_turbidity_initialized) {
        return;
    }

    unsigned short raw = 0;
    if (!ReadAveragedRaw(&raw)) {
        return;
    }

    g_turbidity_raw = raw;
    if (g_turbidity_raw >= TURBIDITY_SAT_HIGH_RAW) {
        g_adc_high_saturation_count++;
    } else {
        g_adc_high_saturation_count = 0;
    }
    if (g_turbidity_raw <= TURBIDITY_SAT_LOW_RAW) {
        g_adc_low_saturation_count++;
    } else {
        g_adc_low_saturation_count = 0;
    }
    if ((g_adc_high_saturation_count == TURBIDITY_SAT_WARN_COUNT) ||
        (g_adc_low_saturation_count == TURBIDITY_SAT_WARN_COUNT)) {
        printf("[Turbidity][Warn] ADC saturation detected: raw=%u (check divider/wiring/power)\n",
               g_turbidity_raw);
    }

    // Convert ADC reading to ADC pin voltage, then reconstruct sensor output voltage
    float adcVoltage = ((float)g_turbidity_raw / ADC_MAX_VALUE) * ADC_VREF_V;
    float vout = adcVoltage * TURBIDITY_DIVIDER_RATIO;

    // Clamp to specified sensor range
    if (vout < AZDM01_MIN_VOUT) vout = AZDM01_MIN_VOUT;
    if (vout > AZDM01_MAX_VOUT) vout = AZDM01_MAX_VOUT;

    g_turbidity_vout = vout;
    g_turbidity_ntu = CalculateNTUFromVoltage(g_turbidity_vout);

    g_update_count++;
    // uint32_t wraparound is acceptable here: periodic modulo logging remains valid after overflow.
    if ((g_update_count % TURBIDITY_LOG_UPDATE_COUNT) == 0U) {
        printf("[Turbidity] raw=%u adc=%.3fV vout=%.3fV ntu=%d (VCC=%.1fV)\n",
               g_turbidity_raw, adcVoltage, g_turbidity_vout, g_turbidity_ntu,
               AZDM01_SUPPLY_VOLTAGE_V);
    }
}

void Turbidity_Init(void)
{
    // Ensure GPIO/ADC subsystem is initialized
    GpioInit();

    g_turbidity_initialized = 1;
    g_update_count = 0;
    g_adc_high_saturation_count = 0;
    g_adc_low_saturation_count = 0;

    // Prime one reading (MQ2-like direct ADC acquisition path)
    Turbidity_Update();

    printf("[Turbidity] Initialized on GPIO01/ADC1 (sensor VCC=%.1fV, no warmup delay)\n",
           AZDM01_SUPPLY_VOLTAGE_V);
}

void Turbidity_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
