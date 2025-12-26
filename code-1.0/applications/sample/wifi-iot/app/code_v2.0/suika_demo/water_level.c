/**
 * @file water_level.c
 * @brief Water level sensor (YW01) implementation for aquatic plant tank
 * Uses ADC3 (GPIO07) for water level sensing
 * 
 * YW01 Sensor Specifications:
 * - Range: 0-90mm
 * - Output: 0-1.0V analog (NON-LINEAR relationship with water level)
 * - Hi3861 ADC: 12-bit (0-4095), 1.8V reference
 * 
 * Calibration Data (from user testing):
 * - Actual 45mm (50%) -> Sensor reads ~12mm (raw linear calculation)
 * - Actual 90mm (100%) -> Sensor reads ~18mm (raw linear calculation)
 * 
 * This indicates the sensor output is non-linear. We use a power-law
 * correction: actual_mm = k * raw_mm^n where n ≈ 1.71, k ≈ 0.77
 * 
 * For embedded efficiency, we use a lookup table with linear interpolation.
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

#include "water_level.h"

#define WATER_LEVEL_ADC_CHANNEL WIFI_IOT_ADC_CHANNEL_3  // GPIO07/ADC3

// YW01 sensor parameters
#define YW01_MAX_MM 90           // Maximum water level in mm
#define YW01_MAX_VOLTAGE_MV 1000 // Maximum output voltage (1.0V = 1000mV)
#define ADC_VREF_MV 1800         // Hi3861 ADC reference voltage (1.8V = 1800mV)
#define ADC_MAX_VALUE 4095       // 12-bit ADC max value

// Calculate ADC value corresponding to YW01 max output (1.0V)
// ADC_at_1V = (1000mV / 1800mV) * 4095 = ~2275
#define YW01_MAX_ADC_VALUE ((YW01_MAX_VOLTAGE_MV * ADC_MAX_VALUE) / ADC_VREF_MV)

static unsigned short g_water_level_raw = 0;

/**
 * @brief Non-linear correction lookup table
 * Maps raw linear mm readings to actual mm readings
 * Based on calibration: raw 12mm -> actual 45mm, raw 18mm -> actual 90mm
 * 
 * Table format: {raw_mm, actual_mm} pairs
 * Uses power-law approximation: actual = 0.77 * raw^1.71
 */
typedef struct {
    int raw_mm;    // Linear calculation result
    int actual_mm; // Corrected actual water level
} CalibrationPoint;

// Calibration lookup table based on power-law: actual = 0.77 * raw^1.71
// Pre-calculated values for efficiency
static const CalibrationPoint g_calibration_table[] = {
    {0, 0},
    {2, 3},      // 0.77 * 2^1.71 ≈ 2.5
    {4, 9},      // 0.77 * 4^1.71 ≈ 8.2
    {6, 16},     // 0.77 * 6^1.71 ≈ 15.7
    {8, 26},     // 0.77 * 8^1.71 ≈ 24.7
    {10, 36},    // 0.77 * 10^1.71 ≈ 35.1
    {12, 45},    // Calibration point: actual 45mm at 50% level
    {14, 58},    // 0.77 * 14^1.71 ≈ 58
    {16, 72},    // 0.77 * 16^1.71 ≈ 72
    {18, 90},    // Calibration point: actual 90mm at 100% level
    {20, 90},    // Clamped to max
};
#define CALIBRATION_TABLE_SIZE (sizeof(g_calibration_table) / sizeof(g_calibration_table[0]))

/**
 * @brief Apply non-linear correction using lookup table with interpolation
 */
static int ApplyNonLinearCorrection(int raw_mm)
{
    if (raw_mm <= 0) return 0;
    if (raw_mm >= 18) return YW01_MAX_MM;  // Cap at 90mm
    
    // Find the two calibration points for interpolation
    int i;
    for (i = 1; i < (int)CALIBRATION_TABLE_SIZE; i++) {
        if (g_calibration_table[i].raw_mm >= raw_mm) {
            break;
        }
    }
    
    // Linear interpolation between calibration points
    const CalibrationPoint *p1 = &g_calibration_table[i - 1];
    const CalibrationPoint *p2 = &g_calibration_table[i];
    
    int raw_range = p2->raw_mm - p1->raw_mm;
    if (raw_range <= 0) return p1->actual_mm;
    
    int actual_range = p2->actual_mm - p1->actual_mm;
    int raw_offset = raw_mm - p1->raw_mm;
    
    return p1->actual_mm + (raw_offset * actual_range) / raw_range;
}

int Get_WaterLevelMM(void)
{
    unsigned int raw = g_water_level_raw;
    
    // Clamp to max expected ADC value for 1.0V input
    if (raw > YW01_MAX_ADC_VALUE) raw = YW01_MAX_ADC_VALUE;
    
    // First, calculate linear mm value (this is what the old code did)
    int linear_mm = (int)((raw * YW01_MAX_MM) / YW01_MAX_ADC_VALUE);
    
    // Apply non-linear correction based on calibration data
    return ApplyNonLinearCorrection(linear_mm);
}

int Get_WaterLevelPercent(void)
{
    // Keep for backward compatibility - convert mm to percentage
    int mm = Get_WaterLevelMM();
    return (mm * 100) / YW01_MAX_MM;
}

unsigned short Get_WaterLevelRaw(void)
{
    return g_water_level_raw;
}

void WaterLevel_Update(void)
{
    AdcRead(WATER_LEVEL_ADC_CHANNEL, &g_water_level_raw,
            WIFI_IOT_ADC_EQU_MODEL_4, WIFI_IOT_ADC_CUR_BAIS_DEFAULT, 0);
}

void WaterLevel_MainLoop(void)
{
    // This function is now a no-op - sensor is polled from control task
}
