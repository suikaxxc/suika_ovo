#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

#include <unistd.h>
#include "hi_wifi_api.h"
//#include "wifi_sta.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"

#include "lwip/sockets.h"

#include "MQTTPacket.h"
#include "transport.h"
#include "living_room_task.h"

int toStop = 0;

int mqtt_connect(void)
{
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	int mysock = 0;
	unsigned char buf[200];
	int buflen = sizeof(buf);
	// int msgid = 1;
	MQTTString topicString = MQTTString_initializer;
	// int req_qos = 0;
	char payload[200];
	int payloadlen = strlen(payload);
	int len = 0;

	//char *host = "121.36.35.193";
	char *host = "192.168.173.50";  //我们主机的ip地址
	int port = 1883;

	mysock = transport_open(host, port);
	if (mysock < 0)
		return mysock;

	printf("Sending to hostname %s port %d\n", host, port);

	data.clientID.cstring = "hi3861_living_room";
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	// data.username.cstring = "testuser";
	// data.password.cstring = "testpassword";

	len = MQTTSerialize_connect(buf, buflen, &data);
	rc = transport_sendPacketBuffer(mysock, buf, len);

	/* wait for connack */
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

	topicString.cstring = "living_room";
	while (!toStop)
	{

		snprintf(payload, sizeof(payload), "{\"fire_alarm\":%d,\"fire_state\":%d,\"temperature\":%.2f,\"humidity\":%.2f}", fire_alarm, fire_state, temperature, humidity);
		printf("published: %s\n\r", payload);
		payloadlen = strlen(payload);
		// printf("publishing reading\n");
		len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char *)payload, payloadlen);
		rc = transport_sendPacketBuffer(mysock, buf, len);
		sleep(1);
	}

	printf("disconnecting\n");
	len = MQTTSerialize_disconnect(buf, buflen);
	rc = transport_sendPacketBuffer(mysock, buf, len);
exit:
	transport_close(mysock);

	rc = rc;

	return 0;
}
