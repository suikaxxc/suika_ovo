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
#include "access_control_task.h"

int toStop = 0;
int owner = 1;


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

	int msgid = 1;
	int req_qos = 0;

	//char *host = "121.36.35.193";
	char *host = "192.168.59.50";
	int port = 1883;

	mysock = transport_open(host, port);
	if (mysock < 0)
		return mysock;

	printf("Sending to hostname %s port %d\n", host, port);

	data.clientID.cstring = "hi3861_access_control";
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

	/* subscribe */
	topicString.cstring = "access_control_cmd";//发布主题
	// topicString.cstring = "substopic";
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);
	rc = transport_sendPacketBuffer(mysock, buf, len);
	if (MQTTPacket_read(buf, buflen, transport_getdata) == SUBACK) /* wait for suback */
	{
		unsigned short submsgid;
		int subcount;
		int granted_qos;

		rc = MQTTDeserialize_suback(&submsgid, 1, &subcount, &granted_qos, buf, buflen);
		if (granted_qos != 0)
		{
			printf("granted qos != 0, %d\n", granted_qos);
			goto exit;
		}
	}
	else
		goto exit;

	topicString.cstring = "access_control";//订阅主题
	while (!toStop)
	{
		if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBLISH)
		{
			unsigned char dup;
			int qos;
			unsigned char retained;
			unsigned short msgid;
			int payloadlen_in;
			unsigned char *payload_in;

			MQTTString receivedTopic;
			rc = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
										 &payload_in, &payloadlen_in, buf, buflen);
			// printf("message arrived %d,%s\n", payloadlen_in, payload_in);
			owner = payload_in[0] - '0';
			rc = rc;
		}

		snprintf(payload, sizeof(payload), "{\"isHuman\":%d,\"isCall\":%d,\"owner\":%d}", isHuman, isCall, owner);
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
