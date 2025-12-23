/**
 * @file tank_control.h
 * @brief Main tank control logic interface for aquatic plant tank
 */

#ifndef __TANK_CONTROL_H__
#define __TANK_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

// Control mode
typedef enum {
    CONTROL_MODE_AUTO = 0,
    CONTROL_MODE_MANUAL = 1
} ControlMode;

// Tank parameters structure
typedef struct {
    // Water level thresholds (percentage)
    int waterLevelMin;
    int waterLevelMax;
    
    // Water temperature thresholds (Celsius)
    float waterTempMin;
    float waterTempMax;
    
    // Light threshold (percentage)
    int lightThreshold;
    
    // Light duration (hours per day)
    int lightDuration;
    
    // TDS thresholds (ppm)
    int tdsMin;
    int tdsMax;
} TankParams;

/**
 * @brief Initialize tank control system
 */
void TankControl_Init(void);

/**
 * @brief Start tank control main loop
 */
void TankControl_MainLoop(void);

/**
 * @brief Set control mode
 * @param mode Control mode (AUTO or MANUAL)
 */
void TankControl_SetMode(ControlMode mode);

/**
 * @brief Get current control mode
 * @return Current control mode
 */
ControlMode TankControl_GetMode(void);

/**
 * @brief Update tank parameters (from AI or user settings)
 * @param params Pointer to new parameters
 */
void TankControl_SetParams(const TankParams *params);

/**
 * @brief Get current tank parameters
 * @return Pointer to current parameters
 */
const TankParams* TankControl_GetParams(void);

/**
 * @brief Set default parameters for a plant type
 * @param plantType Plant type index (0-7)
 */
void TankControl_SetPlantType(int plantType);

/**
 * @brief Manual control - set LED state
 * @param on 1 to turn on, 0 to turn off
 */
void TankControl_ManualLED(int on);

/**
 * @brief Manual control - set fill pump state
 * @param on 1 to turn on, 0 to turn off
 */
void TankControl_ManualFillPump(int on);

/**
 * @brief Manual control - set drain pump state
 * @param on 1 to turn on, 0 to turn off
 */
void TankControl_ManualDrainPump(int on);

/**
 * @brief Manual control - set heater state
 * @param on 1 to turn on, 0 to turn off
 */
void TankControl_ManualHeater(int on);

/**
 * @brief Manual control - set fan speed
 * @param speed Speed percentage (0-100)
 */
void TankControl_ManualFan(int speed);

#ifdef __cplusplus
}
#endif

#endif /* __TANK_CONTROL_H__ */
