/**
 * @file mqtt_client.h
 * @brief MQTT client interface for aquatic plant tank
 */

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start MQTT communication task
 */
void MQTT_MainLoop(void);

/**
 * @brief Check if MQTT is connected
 * @return 1 if connected, 0 if not
 */
int MQTT_IsConnected(void);

#ifdef __cplusplus
}
#endif

#endif /* __MQTT_CLIENT_H__ */
