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
    strcpy(apConfig.ssid, "chinasofti111");             // wifi帐号：chinasofti
    strcpy(apConfig.preSharedKey, "22446688"); // wifi密码：chinasofti@123
    apConfig.securityType = WIFI_SEC_TYPE_PSK;       // 加密方式：WIFI_SEC_TYPE_PSK

    // 3、启动wifi
    errCode = EnableWifi();
    errCode = AddDeviceConfig(&apConfig, &netId);

    // 4、连接wifi
    errCode = ConnectTo(netId);
    printf("ConnectTo(%d): %d\r\n", netId, errCode);
    usleep(1000 * 1000);

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