/**
 * @file wifi_connect.c
 * @brief WiFi connection implementation for aquatic plant tank
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/ip4_addr.h"
#include "lwip/err.h"

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

    printf("[WiFi] Connecting to %s...\r\n", WIFI_SSID);

    // Set WiFi parameters
    strcpy(apConfig.ssid, WIFI_SSID);
    strcpy(apConfig.preSharedKey, WIFI_PASSWORD);
    apConfig.securityType = WIFI_SEC_TYPE_PSK;

    // Enable WiFi
    errCode = EnableWifi();
    if (errCode != WIFI_SUCCESS)
    {
        printf("[WiFi] EnableWifi failed: %d\r\n", errCode);
        return -1;
    }

    // Wait for WiFi to be ready
    sleep(1);

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
        printf("[WiFi] AddDeviceConfig failed: %d\r\n", errCode);
        return -1;
    }

    // Connect to WiFi
    errCode = ConnectTo(netId);
    if (errCode != WIFI_SUCCESS)
    {
        printf("[WiFi] ConnectTo failed: %d\r\n", errCode);
        return -1;
    }

    // Wait for connection
    sleep(3);

    // Get IP address via DHCP
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        err_t ret = netifapi_dhcp_start(iface);
        if (ret != ERR_OK)
        {
            printf("[WiFi] DHCP start failed: %d\r\n", ret);
            return -1;
        }

        // Wait for DHCP
        for (int i = 0; i < 10; i++)
        {
            sleep(1);
            if (iface->ip_addr.addr != 0)
            {
                break;
            }
        }

        if (iface->ip_addr.addr != 0)
        {
            g_wifi_connected = 1;
            printf("[WiFi] Connected, IP: %s\r\n", ip4addr_ntoa(&iface->ip_addr));
            return 0;
        }
    }

    printf("[WiFi] Failed to get IP address\r\n");
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
}
