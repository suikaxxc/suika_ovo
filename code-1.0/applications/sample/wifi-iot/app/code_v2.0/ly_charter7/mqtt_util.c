#include <stdio.h>
#include <unistd.h>

#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"

#include "aht20_demo.h"
#include "mq2_demo.h"
#include "ldr_demo.h"
#include "yw01_demo.h"

int mqtt_connect(void)
{
    // 1、声明化变量
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int mysock = 0;
    unsigned char buf[200];
    int buflen = sizeof(buf);

    MQTTString topicString = MQTTString_initializer; // 设置主题
    char payload[200];
    int payloadlen = strlen(payload);
    int len = 0;

    // 2、配置连接参数
    char *host = "192.168.42.169"; // MQTT服务器地址
    int port = 1883;               // MQTT服务器端口
    topicString.cstring = "ems";   // 设置主题

    data.clientID.cstring = "suika-hi3861"; // 修改成自己ID，
    data.keepAliveInterval = 20;
    data.cleansession = 1;
    data.username.cstring = "hi3861_ems";
    data.password.cstring = "testpassword";

    // 3、连接服务器
    mysock = transport_open(host, port);
    if (mysock < 0)
        return mysock;

    printf("Sending to hostname %s port %d\n", host, port);

    len = MQTTSerialize_connect(buf, buflen, &data);
    transport_sendPacketBuffer(mysock, buf, len);

    if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
    {
        unsigned char sessionPresent, connack_rc;

        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
        {
            printf("Unable to connect, return code %d\n", connack_rc);
            goto exit;
        }
    }
    else
        goto exit;

    // 4、发布消息
    while (1 == 1)
    {
        // 获取传感器数据
        float temperature = Get_Temperature();
        float humidity = Get_Humidity();
        unsigned short gas = Get_Mq2Value();
        int light = Get_LightPercent();
        int level = Get_YW01Mm();

        // 封装为json字符串
        snprintf(payload, sizeof(payload), "{\"temp\":%.1f,\"hum\":%.1f,\"gas\":%d,\"light\":%d,\"level\":%d}",
                 temperature, humidity, gas, light, level);
        payloadlen = strlen(payload);
        len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);
        transport_sendPacketBuffer(mysock, buf, len);

        sleep(1); // 每隔1秒发送一次
    }

exit:
    transport_close(mysock);

    return 0;
}
