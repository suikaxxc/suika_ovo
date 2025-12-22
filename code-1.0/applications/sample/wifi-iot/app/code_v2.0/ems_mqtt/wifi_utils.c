#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

void connect_wifi(void)
{

    WifiErrorCode errCode;
    WifiDeviceConfig apConfig = {};
    int netId = -1;

    // setup your AP params
    // strcpy(apConfig.ssid, "ohos");  //wifi的帐号  ohos
    // strcpy(apConfig.preSharedKey, "aaaaaaaaaa");//wifi密码   aaaaaaaaaa
    strcpy(apConfig.ssid, "TP-LINK_888");       // wifi的帐号
    strcpy(apConfig.preSharedKey, "Cto888com"); // wifi密码
    // strcpy(apConfig.ssid, "TP-LINK_A673");
    // strcpy(apConfig.preSharedKey, "aaaaaaaaaa");
    // strcpy(apConfig.ssid, "TP-LINK_D49F");     //
    // strcpy(apConfig.preSharedKey, "wyz315ao"); //密码
    apConfig.securityType = WIFI_SEC_TYPE_PSK; // 加密方式

    errCode = EnableWifi();
    errCode = AddDeviceConfig(&apConfig, &netId);

    errCode = ConnectTo(netId);
    printf("ConnectTo(%d): %d\r\n", netId, errCode);
    usleep(1000 * 1000);
    // 联网业务开始
    // 这里是网络业务代码...
    struct netif *iface = netifapi_netif_find("wlan0");
    if (iface)
    {
        err_t ret = netifapi_dhcp_start(iface); // 返回IP地址
        printf("netifapi_dhcp_start: %d\r\n", ret);

        // usleep(2000 * 1000);
        sleep(5);
        ; // wait DHCP server give me IP
        ret = netifapi_netif_common(iface, dhcp_clients_info_show, NULL);
        printf("netifapi_netif_common: %d\r\n", ret);
    }
}
