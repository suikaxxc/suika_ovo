/**
 * @file turbidity_sensor.c
 * @brief AZDM01 turbidity sensor implementation for aquatic plant tank
 * Uses ADC1 (GPIO01) for turbidity measurement
 *
 * Sensor output: analog voltage (typically 0.5V ~ 4.5V)
 * Acquisition follows MQ2-style direct ADC sampling, then converts to NTU.
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
// Sensor warm-up time after power-on (seconds)
#define AZDM01_WARMUP_SECONDS 2

// Voltage divider reconstruction ratio for mapping ADC pin voltage back to sensor Vout.
// ratio = Vout / Vadc = (R1 + R2) / R2 (R1: upper resistor, R2: lower resistor to GND)
// This value MUST match the actual resistor divider on hardware.
// Current configuration assumes 4.5V -> 1.8V at ADC pin, so ratio = 4.5 / 1.8 = 2.5.
// If your hardware divider differs, measure Vadc and Vout under a stable condition and set:
// TURBIDITY_DIVIDER_RATIO = Vout / Vadc
#define TURBIDITY_DIVIDER_RATIO 2.5f

// MQ2-style direct read + short averaging for better stability
#define TURBIDITY_SAMPLE_COUNT 8
#define TURBIDITY_LOG_UPDATE_COUNT 30U
#define TURBIDITY_NTU_MIN 0
#define TURBIDITY_NTU_MAX 1000

static unsigned short g_turbidity_raw = 0;
static float g_turbidity_vout = AZDM01_MAX_VOUT;
static int g_turbidity_ntu = 0;
static int g_turbidity_initialized = 0;
static uint32_t g_update_count = 0;
static int g_turbidity_manual_mode = 0;
static int g_turbidity_manual_ntu = 120;

static int ReadRawMq2Style(unsigned short *rawOut)
{
    unsigned short raw = 0;
    if (AdcRead(TURBIDITY_ADC_CHANNEL, &raw,
                WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0) != WIFI_IOT_SUCCESS) {
        return 0;
    }
    *rawOut = raw;
    return 1;
}

static int ReadAveragedRaw(unsigned short *rawOut)
{
    unsigned int sum = 0;
    int validSamples = 0;
    int i;

    for (i = 0; i < TURBIDITY_SAMPLE_COUNT; i++) {
        unsigned short raw = 0;
        if (ReadRawMq2Style(&raw)) {
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

int Get_TurbidityNTU(void)
{
    if (g_turbidity_manual_mode) {
        return g_turbidity_manual_ntu;
    }
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

void Turbidity_SetManualMode(int enabled)
{
    g_turbidity_manual_mode = (enabled != 0) ? 1 : 0;
    printf("[Turbidity] Manual mode: %s\n", g_turbidity_manual_mode ? "ON" : "OFF");
}

int Turbidity_IsManualMode(void)
{
    return g_turbidity_manual_mode;
}

void Turbidity_SetManualValue(int ntu)
{
    if (ntu < TURBIDITY_NTU_MIN) ntu = TURBIDITY_NTU_MIN;
    if (ntu > TURBIDITY_NTU_MAX) ntu = TURBIDITY_NTU_MAX;
    g_turbidity_manual_ntu = ntu;
    printf("[Turbidity] Manual NTU set to %d\n", g_turbidity_manual_ntu);
}

void Turbidity_Update(void)
{
    if (!g_turbidity_initialized) {
        return;
    }
    if (g_turbidity_manual_mode) {
        return;
    }

    unsigned short raw = 0;
    if (!ReadAveragedRaw(&raw)) {
        return;
    }

    g_turbidity_raw = raw;

    // Convert ADC reading to ADC pin voltage, then reconstruct sensor output voltage
    float adcVoltage = ((float)g_turbidity_raw / ADC_MAX_VALUE) * ADC_VREF_V;
    float vout = adcVoltage * TURBIDITY_DIVIDER_RATIO;

    // Clamp to specified sensor range
    if (vout < AZDM01_MIN_VOUT) vout = AZDM01_MIN_VOUT;
    if (vout > AZDM01_MAX_VOUT) vout = AZDM01_MAX_VOUT;
    g_turbidity_vout = vout;

    // AZDM01 characteristic: clearer water -> higher voltage, dirtier water -> lower voltage
    float ratio = (AZDM01_MAX_VOUT - g_turbidity_vout) / (AZDM01_MAX_VOUT - AZDM01_MIN_VOUT);
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    int instantNtu = (int)(ratio * (float)TURBIDITY_NTU_MAX + 0.5f);
    if (instantNtu < TURBIDITY_NTU_MIN) instantNtu = TURBIDITY_NTU_MIN;
    if (instantNtu > TURBIDITY_NTU_MAX) instantNtu = TURBIDITY_NTU_MAX;

    // Light smoothing for display/control stability
    if (g_update_count == 0U) {
        g_turbidity_ntu = instantNtu;
    } else {
        g_turbidity_ntu = (g_turbidity_ntu * 3 + instantNtu) / 4;
    }

    g_update_count++;
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

    // Wait sensor analog output to stabilize after power-on.
    osDelay(((uint32_t)AZDM01_WARMUP_SECONDS) * 1000U);

    g_turbidity_initialized = 1;
    g_update_count = 0;
    g_turbidity_raw = 0;
    g_turbidity_vout = AZDM01_MAX_VOUT;
    g_turbidity_ntu = 0;

    // Prime one reading right after warm-up
    Turbidity_Update();

    printf("[Turbidity] Initialized on GPIO01/ADC1 (MQ2-style ADC read, VCC=%.1fV, warmup=%ds)\n",
           AZDM01_SUPPLY_VOLTAGE_V, AZDM01_WARMUP_SECONDS);
}

void Turbidity_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
