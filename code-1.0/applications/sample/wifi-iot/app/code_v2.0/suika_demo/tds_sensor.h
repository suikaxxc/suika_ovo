/**
 * @file tds_sensor.h
 * @brief TDS water quality sensor interface for aquatic plant tank
 */

#ifndef __TDS_SENSOR_H__
#define __TDS_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize TDS sensor (no-op, for compatibility)
 */
void TDS_MainLoop(void);

/**
 * @brief Collect ADC sample for median filtering
 *        Call this frequently (e.g., every 40-100ms) for stable readings
 */
void TDS_CollectSample(void);

/**
 * @brief Update TDS sensor reading (call periodically from control task)
 *        This calculates TDS from collected samples using median filter
 */
void TDS_Update(void);

/**
 * @brief Get current TDS value in ppm
 * @return TDS value in ppm
 */
int Get_TDSValue(void);

/**
 * @brief Get raw TDS ADC value (median of samples)
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_TDSRaw(void);

/**
 * @brief Enable/disable manual TDS override mode
 * @param enabled 1-enable, 0-disable
 */
void TDS_SetManualMode(int enabled);

/**
 * @brief Get manual TDS override mode
 * @return 1-enabled, 0-disabled
 */
int TDS_IsManualMode(void);

/**
 * @brief Set manual TDS value used when manual mode is enabled
 * @param ppm TDS value in ppm (will be clamped to 0-1000)
 */
void TDS_SetManualValue(int ppm);

#ifdef __cplusplus
}
#endif

#endif /* __TDS_SENSOR_H__ */
