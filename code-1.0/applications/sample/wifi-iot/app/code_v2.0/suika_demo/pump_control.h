/**
 * @file pump_control.h
 * @brief Water pump control interface (L9110S motor driver) for aquatic plant tank
 */

#ifndef __PUMP_CONTROL_H__
#define __PUMP_CONTROL_H__

#ifdef __cplusplus
extern "C" {
#endif

// Pump types
typedef enum {
    PUMP_DRAIN = 0,  // Drain pump (extract water)
    PUMP_FILL = 1    // Fill pump (add water)
} PumpType;

// Pump status
typedef enum {
    PUMP_OFF = 0,
    PUMP_ON = 1
} PumpStatus;

/**
 * @brief Initialize pump control GPIO pins
 */
void Pump_Init(void);

/**
 * @brief Set pump state
 * @param pump Pump type (PUMP_DRAIN or PUMP_FILL)
 * @param status Pump status (PUMP_OFF or PUMP_ON)
 */
void Pump_SetState(PumpType pump, PumpStatus status);

/**
 * @brief Get current pump state
 * @param pump Pump type
 * @return Current pump status
 */
PumpStatus Pump_GetState(PumpType pump);

/**
 * @brief Turn on drain pump
 */
void Pump_StartDrain(void);

/**
 * @brief Turn off drain pump
 */
void Pump_StopDrain(void);

/**
 * @brief Turn on fill pump
 */
void Pump_StartFill(void);

/**
 * @brief Turn off fill pump
 */
void Pump_StopFill(void);

/**
 * @brief Emergency stop all pumps
 */
void Pump_StopAll(void);

#ifdef __cplusplus
}
#endif

#endif /* __PUMP_CONTROL_H__ */
