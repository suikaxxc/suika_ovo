#ifndef I2C_COMMON_H
#define I2C_COMMON_H

#include "cmsis_os2.h"

// Global I2C mutex for thread-safe access
extern osMutexId_t g_i2cMutex;

// I2C initialization function with mutex setup
void I2C_CommonInit(void);

#endif
