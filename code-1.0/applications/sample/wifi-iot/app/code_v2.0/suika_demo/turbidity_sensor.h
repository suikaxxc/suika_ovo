/**
 * @file turbidity_sensor.h
 * @brief AZDM01 turbidity sensor interface for aquatic plant tank
 *
 * Sensor output: analog voltage (typically 0.5V ~ 4.5V)
 * Acquisition: MQ2-like raw ADC sampling + voltage-based NTU conversion
 */

#ifndef __TURBIDITY_SENSOR_H__
#define __TURBIDITY_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize AZDM01 turbidity sensor.
 *        Includes sensor warm-up stage to avoid unstable startup readings.
 */
void Turbidity_Init(void);

/**
 * @brief No-op placeholder kept for API consistency with other sensor modules.
 *        Similar to WaterLevel_MainLoop/TDS_MainLoop/LightSensor_MainLoop, this module
 *        does not spawn a dedicated task and is sampled by TankControl_Task.
 *        Periodic acquisition is done via Turbidity_Update().
 */
void Turbidity_MainLoop(void);

/**
 * @brief Update turbidity sensor reading (call periodically)
 */
void Turbidity_Update(void);

/**
 * @brief Get current turbidity in NTU
 * @return NTU value
 */
int Get_TurbidityNTU(void);

/**
 * @brief Get current sensor output voltage (Vout, volts)
 * @return Vout in volts
 */
float Get_TurbidityVoltage(void);

/**
 * @brief Get averaged raw ADC value
 * @return Raw ADC value (0-4095)
 */
unsigned short Get_TurbidityRaw(void);

/**
 * @brief Enable/disable manual turbidity override mode
 * @param enabled 1-enable, 0-disable
 */
void Turbidity_SetManualMode(int enabled);

/**
 * @brief Get manual turbidity override mode
 * @return 1-enabled, 0-disabled
 */
int Turbidity_IsManualMode(void);

/**
 * @brief Set manual turbidity value used when manual mode is enabled
 * @param ntu Turbidity value in NTU (will be clamped to 0-1000)
 */
void Turbidity_SetManualValue(int ntu);

#ifdef __cplusplus
}
#endif

#endif /* __TURBIDITY_SENSOR_H__ */
