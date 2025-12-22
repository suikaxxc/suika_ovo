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
 * @brief Initialize and start TDS sensor task
 */
void TDS_MainLoop(void);

/**
 * @brief Get current TDS value in ppm
 * @return TDS value in ppm
 */
int Get_TDSValue(void);

/**
 * @brief Get raw TDS ADC value
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_TDSRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __TDS_SENSOR_H__ */
