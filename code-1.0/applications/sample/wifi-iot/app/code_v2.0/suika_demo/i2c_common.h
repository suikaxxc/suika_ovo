#ifndef I2C_COMMON_H
#define I2C_COMMON_H

#include "cmsis_os2.h"

// 声明全局 I2C 互斥锁
extern osMutexId_t g_i2cMutex;

// I2C 初始化函数（带锁创建）
void I2C_CommonInit(void);

#endif
