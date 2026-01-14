/**
 * @file water_level.c
 * @brief Water level sensor (YW01) implementation for aquatic plant tank
 * Uses ADC3 (GPIO07) for water level sensing
 * 
 * YW01 Sensor Specifications:
 * - Range: 0-90mm
 * - Working voltage: 5.0V
 * - Output: 0-1.0V analog (NON-LINEAR relationship with water level)
 * - Hi3861 ADC: 12-bit (0-4095), 1.8V reference
 * 
 * Calibration Data (from user testing - 2024-01-14):
 * - Actual 0mm -> Sensor raw reads ~12mm (offset at zero water level)
 * - Actual ~40mm -> Sensor raw reads ~24mm
 * 
 * Calibration formula: actual_mm = (raw_mm - ZERO_OFFSET) * SCALE_FACTOR
 * Where: ZERO_OFFSET = 12, SCALE_FACTOR = 40 / (24 - 12) = 3.33
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
 * 
 * New calibration data (2024-01-14):
 * - Actual 0mm -> raw reads 12mm (zero offset)
 * - Actual 40mm -> raw reads 24mm
 * 
 * Linear relationship: actual = (raw - 12) * (40 / 12) = (raw - 12) * 3.33
 * 
 * Table format: {raw_mm, actual_mm} pairs
 */
typedef struct {
    int raw_mm;    // Linear calculation result from ADC
    int actual_mm; // Corrected actual water level
} CalibrationPoint;

// Zero offset: sensor reads 12mm when actual water level is 0mm
#define YW01_ZERO_OFFSET 12

// Calibration lookup table based on new data
// Formula: actual = (raw - 12) * 3.33, clamped to [0, 90]
static const CalibrationPoint g_calibration_table[] = {
    {0, 0},      // Below offset - treat as 0
    {12, 0},     // Calibration point: actual 0mm (zero offset)
    {15, 10},    // (15-12) * 3.33 ≈ 10
    {18, 20},    // (18-12) * 3.33 ≈ 20
    {21, 30},    // (21-12) * 3.33 ≈ 30
    {24, 40},    // Calibration point: actual 40mm
    {27, 50},    // (27-12) * 3.33 ≈ 50
    {30, 60},    // (30-12) * 3.33 ≈ 60
    {33, 70},    // (33-12) * 3.33 ≈ 70
    {36, 80},    // (36-12) * 3.33 ≈ 80
    {39, 90},    // (39-12) * 3.33 ≈ 90
    {50, 90},    // Clamped to max
};
#define CALIBRATION_TABLE_SIZE (sizeof(g_calibration_table) / sizeof(g_calibration_table[0]))

/**
 * @brief Apply non-linear correction using lookup table with interpolation
 * Handles zero offset and linear scaling based on calibration data
 */
static int ApplyNonLinearCorrection(int raw_mm)
{
    // Below zero offset - treat as 0mm
    if (raw_mm <= YW01_ZERO_OFFSET) return 0;
    
    // Above max calibrated value - cap at 90mm
    if (raw_mm >= 39) return YW01_MAX_MM;
    
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
