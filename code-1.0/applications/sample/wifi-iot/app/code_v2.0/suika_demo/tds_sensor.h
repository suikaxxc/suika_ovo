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

#ifdef __cplusplus
}
#endif

#endif /* __TDS_SENSOR_H__ */
