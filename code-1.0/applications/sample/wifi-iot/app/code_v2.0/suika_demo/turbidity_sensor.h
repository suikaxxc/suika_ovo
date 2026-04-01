/**
 * @file turbidity_sensor.h
 * @brief Turbidity sensor interface for aquatic plant tank
 */

#ifndef __TURBIDITY_SENSOR_H__
#define __TURBIDITY_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Collect turbidity sensor digital input state
 */
void Turbidity_CollectSample(void);

/**
 * @brief Update turbidity value (NTU)
 */
void Turbidity_Update(void);

/**
 * @brief Get current turbidity value in NTU
 * @return Turbidity value (0-1000 NTU)
 */
int Get_TurbidityValue(void);

/**
 * @brief Get mapped raw turbidity digital value
 * @return 0 when GPIO01 is LOW(triggered), 4095 when GPIO01 is HIGH(not triggered)
 */
unsigned short Get_TurbidityRaw(void);

#ifdef __cplusplus
}
#endif

#endif /* __TURBIDITY_SENSOR_H__ */
