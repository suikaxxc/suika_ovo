/**
 * @file water_level.h
 * @brief Water level sensor (YW01) interface for aquatic plant tank
 * 
 * YW01 Sensor: 0-90mm range, 0-1.0V analog output
 */

#ifndef __WATER_LEVEL_H__
#define __WATER_LEVEL_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize water level sensor (no-op, for compatibility)
 */
void WaterLevel_MainLoop(void);

/**
 * @brief Update water level reading (call periodically from control task)
 */
void WaterLevel_Update(void);

/**
 * @brief Get current water level in millimeters (0-90mm)
 * @return Water level in mm
 */
int Get_WaterLevelMM(void);

/**
 * @brief Get current water level in percentage (0-100) - for backward compatibility
 * @return Water level percentage
 */
int Get_WaterLevelPercent(void);

/**
 * @brief Get raw water level ADC value
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_WaterLevelRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __WATER_LEVEL_H__ */
