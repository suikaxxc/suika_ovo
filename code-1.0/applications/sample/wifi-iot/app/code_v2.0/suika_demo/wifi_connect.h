/**
 * @file wifi_connect.h
 * @brief WiFi connection interface for aquatic plant tank
 */

#ifndef __WIFI_CONNECT_H__
#define __WIFI_CONNECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Connect to WiFi network
 * @return 0 on success, -1 on failure
 */
int WiFi_Connect(void);

/**
 * @brief Check if WiFi is connected
 * @return 1 if connected, 0 if not
 */
int WiFi_IsConnected(void);

/**
 * @brief Disconnect from WiFi
 */
void WiFi_Disconnect(void);

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_CONNECT_H__ */
