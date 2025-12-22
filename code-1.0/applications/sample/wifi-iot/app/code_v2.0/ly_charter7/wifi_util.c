#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

void connect_wifi(void)
{

    // 1、定义wifi相关变量
    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    // 2、配置wifi 参数
    strcpy(apConfig.ssid, "suika2025");             // wifi帐号：suika2025
    strcpy(apConfig.preSharedKey, "957957957"); // wifi密码：957957957
    apConfig.securityType = WIFI_SEC_TYPE_PSK;       // 加密方式：WIFI_SEC_TYPE_PSK

    // 3、启动wifi
    errCode = EnableWifi();
    printf("EnableWifi: %d\r\n", errCode);
    
    // 清除旧配置，防止配置满
    WifiDeviceConfig configs[WIFI_MAX_CONFIG_SIZE] = {0};
    unsigned int size = WIFI_MAX_CONFIG_SIZE;
    if (GetDeviceConfigs(configs, &size) == WIFI_SUCCESS) {
        for (unsigned int i = 0; i < size; i++) {
            RemoveDevice(configs[i].netId);
        }
    }

    errCode = AddDeviceConfig(&apConfig, &netId);
    printf("AddDeviceConfig: %d, netId: %d\r\n", errCode, netId);

    // 4、连接wifi
    errCode = ConnectTo(netId);
    printf("ConnectTo(%d): %d\r\n", netId, errCode);
    sleep(3);

    // 5、获取IP地址
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        // 启动DHCP
        err_t ret = netifapi_dhcp_start(iface);
        printf("netifapi_dhcp_start: %d\r\n", ret);

        sleep(5);
        // 打印IP地址
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("netifapi_netif_common: %d\r\n", ret);
    }
}
