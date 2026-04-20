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
#define AZDM01_MIN_NTU 62.5f
#define AZDM01_MAX_NTU 562.5f
// AZDM01 power supply configuration (hardware uses 5V)
#define AZDM01_SUPPLY_VOLTAGE_V 5.0f
// Sensor warm-up time after power-on (seconds)
#define AZDM01_WARMUP_SECONDS 2

// Voltage divider reconstruction ratio for mapping ADC pin voltage back to sensor Vout.
// ratio = Vout / Vadc = (R1 + R2) / R2 (R1: upper resistor, R2: lower resistor to GND)
// This value MUST match the actual resistor divider on hardware.
// Current configuration assumes 4.5V -> 1.8V at ADC pin, so ratio = 4.5 / 1.8 = 2.5.
// One matching example is R1=15kΩ (upper resistor from sensor Vout to ADC node)
// and R2=10kΩ (lower resistor from ADC node to GND): (15k+10k)/10k = 2.5.
#define TURBIDITY_DIVIDER_RATIO 2.5f

// Simple averaging for stable ADC reading:
// 8 samples provides basic noise suppression while keeping control-loop response fast.
#define TURBIDITY_SAMPLE_COUNT 8

static unsigned short g_turbidity_raw = 0;
static float g_turbidity_vout = AZDM01_MAX_VOUT;
static int g_turbidity_ntu = 0;
static int g_turbidity_initialized = 0;
static uint32_t g_update_count = 0;

static int CalculateNTU(float vout)
{
    // Formula from requirement: NTU = -125 * Vout + 625
    float ntu = -125.0f * vout + 625.0f;

    // For AZDM01 Vout range 0.5V~4.5V, theoretical NTU range is 62.5~562.5.
    if (ntu < AZDM01_MIN_NTU) ntu = AZDM01_MIN_NTU;
    if (ntu > AZDM01_MAX_NTU) ntu = AZDM01_MAX_NTU;

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
    // Formula voltage is based on 5V-powered sensor output.
    float voutForFormula = vout * (5.0f / AZDM01_SUPPLY_VOLTAGE_V);

    // Clamp to specified sensor range
    if (vout < AZDM01_MIN_VOUT) vout = AZDM01_MIN_VOUT;
    if (vout > AZDM01_MAX_VOUT) vout = AZDM01_MAX_VOUT;
    if (voutForFormula < AZDM01_MIN_VOUT) voutForFormula = AZDM01_MIN_VOUT;
    if (voutForFormula > AZDM01_MAX_VOUT) voutForFormula = AZDM01_MAX_VOUT;

    g_turbidity_vout = vout;
    g_turbidity_ntu = CalculateNTU(voutForFormula);

    g_update_count++;
    if ((g_update_count % 30U) == 0U) {
        printf("[Turbidity] raw=%u adc=%.3fV vout=%.3fV ntu=%d (VCC=%.1fV)\n",
               g_turbidity_raw, adcVoltage, g_turbidity_vout, g_turbidity_ntu,
               AZDM01_SUPPLY_VOLTAGE_V);
    }
}

void Turbidity_Init(void)
{
    // Ensure GPIO/ADC subsystem is initialized
    GpioInit();

    // Wait sensor analog output to stabilize after power-on.
    sleep(AZDM01_WARMUP_SECONDS);

    g_turbidity_initialized = 1;
    g_update_count = 0;

    // Prime one reading to avoid long initial zero value.
    Turbidity_Update();

    printf("[Turbidity] Initialized on GPIO01/ADC1 (sensor VCC=%.1fV, warmup=%ds)\n",
           AZDM01_SUPPLY_VOLTAGE_V, AZDM01_WARMUP_SECONDS);
}

void Turbidity_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
