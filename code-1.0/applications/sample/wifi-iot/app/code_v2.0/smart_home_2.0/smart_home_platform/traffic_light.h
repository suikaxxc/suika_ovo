
#ifndef TRAFFIC_LIGHT
#define TRAFFIC_LIGHT
#include "wifiiot_gpio.h"
/**
 * led灯初始化GPIO
 */
void initLights(void);

/***
 * 控制函数
 */
void lights_controller(unsigned char * cmd);
#endif