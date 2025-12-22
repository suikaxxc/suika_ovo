/**
 * @file wifi_connect.c
 * @brief WiFi connection implementation for aquatic plant tank
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

#include "wifi_connect.h"

// WiFi configuration - modify these for your network
// For production, these should be stored in configuration or set at build time
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

static int g_wifi_connected = 0;

int WiFi_Connect(void)
{
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    printf("[WiFi] Connecting to %s...\n", WIFI_SSID);

    // Set WiFi parameters
    strcpy(apConfig.ssid, WIFI_SSID);
    strcpy(apConfig.preSharedKey, WIFI_PASSWORD);
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    // Enable WiFi
    errCode = EnableWifi();
    if (errCode != WIFI_SUCCESS)
    {
        printf("[WiFi] EnableWifi failed: %d\n", errCode);
        return -1;
    }
    printf("[WiFi] EnableWifi: %d\n", errCode);

    // Clear existing configurations
    WifiDeviceConfig configs[WIFI_MAX_CONFIG_SIZE] = {0};
    unsigned int size = WIFI_MAX_CONFIG_SIZE;
    if (GetDeviceConfigs(configs, &size) == WIFI_SUCCESS)
    {
        for (unsigned int i = 0; i < size; i++)
        {
            RemoveDevice(configs[i].netId);
        }
    }

    // Add device configuration
    errCode = AddDeviceConfig(&apConfig, &netId);
    if (errCode != WIFI_SUCCESS)
    {
        printf("[WiFi] AddDeviceConfig failed: %d\n", errCode);
        return -1;
    }
    printf("[WiFi] AddDeviceConfig: %d, netId: %d\n", errCode, netId);

    // Connect to WiFi
    errCode = ConnectTo(netId);
    if (errCode != WIFI_SUCCESS)
    {
        printf("[WiFi] ConnectTo failed: %d\n", errCode);
        return -1;
    }
    printf("[WiFi] ConnectTo(%d): %d\n", netId, errCode);

    // Wait for connection
    sleep(3);

    // Get IP address via DHCP
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        err_t ret = netifapi_dhcp_start(iface);
        printf("[WiFi] DHCP start: %d\n", ret);

        sleep(5);

        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("[WiFi] DHCP info: %d\n", ret);

        g_wifi_connected = 1;
        printf("[WiFi] Connected successfully\n");
        return 0;
    }

    printf("[WiFi] Failed to get network interface\n");
    return -1;
}

int WiFi_IsConnected(void)
{
    return g_wifi_connected;
}

void WiFi_Disconnect(void)
{
    DisableWifi();
    g_wifi_connected = 0;
    printf("[WiFi] Disconnected\n");
}
