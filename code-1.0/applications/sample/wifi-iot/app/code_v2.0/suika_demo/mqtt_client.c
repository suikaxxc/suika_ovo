/**
 * @file mqtt_client.c
 * @brief MQTT client implementation for aquatic plant tank
 * Publishes sensor data and receives control commands from HarmonyOS app
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "MQTTPacket.h"
#include "transport.h"

#include "mqtt_client.h"
#include "wifi_connect.h"
#include "water_level.h"
#include "ds18b20.h"
#include "tds_sensor.h"
#include "light_sensor.h"
#include "pump_control.h"
#include "temp_control.h"
#include "led_control.h"
#include "alarm.h"
#include "tank_control.h"

#define MQTT_TASK_STACK_SIZE 4096

// MQTT configuration - modify these for your setup
// For production, these should be stored in configuration or set at build time
#define MQTT_HOST "192.168.1.100"  // MQTT broker address
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "hi3861_aquatic_tank"
#define MQTT_USERNAME "mqtt_user"
#define MQTT_PASSWORD "mqtt_password"

// MQTT topics (matching HarmonyOS app)
#define MQTT_TOPIC_DATA "tank/data"      // Publish sensor data
#define MQTT_TOPIC_CONTROL "tank/control" // Subscribe for control commands

static int g_mqtt_connected = 0;
static int g_mqtt_socket = -1;

int MQTT_IsConnected(void)
{
    return g_mqtt_connected;
}

// Build and publish sensor data JSON
static void PublishSensorData(int socket)
{
    unsigned char buf[512];
    int buflen = sizeof(buf);
    char payload[400];
    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = MQTT_TOPIC_DATA;

    // Get all sensor values
    int waterLevel = Get_WaterLevelPercent();
    float waterTemp = Get_WaterTemperature();
    int lightIntensity = Get_LightIntensity();
    int tdsValue = Get_TDSValue();

    // Get actuator states
    int pumpStatus = Pump_GetState(PUMP_FILL);
    int waterPumpStatus = Pump_GetState(PUMP_DRAIN);
    int heaterStatus = Heater_GetState();
    int fanSpeed = Fan_GetSpeed();
    int ledStatus = LED_GetState();
    AlarmLevel alarmLevel = Alarm_GetLevel();
    const char *alarmMsg = Alarm_GetMessage();

    // Build JSON payload matching HarmonyOS app MqttData interface
    snprintf(payload, sizeof(payload),
             "{"
             "\"waterLevel\":%d,"
             "\"waterTemp\":%.1f,"
             "\"lightIntensity\":%d,"
             "\"tdsValue\":%d,"
             "\"pumpStatus\":%d,"
             "\"waterPumpStatus\":%d,"
             "\"heaterStatus\":%d,"
             "\"fanSpeed\":%d,"
             "\"ledStatus\":%d,"
             "\"alarmStatus\":%d,"
             "\"alarmMessage\":\"%s\""
             "}",
             waterLevel, waterTemp, lightIntensity, tdsValue,
             pumpStatus, waterPumpStatus, heaterStatus, fanSpeed, ledStatus,
             (int)alarmLevel, alarmMsg);

    int payloadlen = strlen(payload);
    int len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString,
                                     (unsigned char *)payload, payloadlen);
    transport_sendPacketBuffer(socket, buf, len);
}

// Parse and handle control commands from HarmonyOS app
static void HandleControlCommand(const char *payload, int payloadLen)
{
    char cmdBuf[256];
    if (payloadLen >= (int)sizeof(cmdBuf) - 1)
    {
        payloadLen = (int)sizeof(cmdBuf) - 1;
    }
    memcpy(cmdBuf, payload, payloadLen);
    cmdBuf[payloadLen] = '\0';

    printf("[MQTT] Received command: %s\n", cmdBuf);

    // Parse JSON command
    // Expected format from HarmonyOS app ControlData:
    // {"type":"led|pump|heater|fan|mode|settings","value":0|1|...}

    // Simple JSON parsing (avoiding complex library)
    char *typeStart = strstr(cmdBuf, "\"type\":\"");
    char *valueStart = strstr(cmdBuf, "\"value\":");

    if (typeStart && valueStart)
    {
        typeStart += 8;  // Skip "\"type\":\""
        char *typeEnd = strchr(typeStart, '"');
        if (!typeEnd) return;
        *typeEnd = '\0';
        const char *cmdType = typeStart;

        valueStart += 8;  // Skip "\"value\":"
        int value = atoi(valueStart);

        printf("[MQTT] Command type: %s, value: %d\n", cmdType, value);

        // Handle different command types
        if (strcmp(cmdType, "led") == 0)
        {
            TankControl_ManualLED(value);
        }
        else if (strcmp(cmdType, "pump") == 0)
        {
            TankControl_ManualFillPump(value);
        }
        else if (strcmp(cmdType, "waterPump") == 0 || strcmp(cmdType, "drain") == 0)
        {
            TankControl_ManualDrainPump(value);
        }
        else if (strcmp(cmdType, "heater") == 0)
        {
            TankControl_ManualHeater(value);
        }
        else if (strcmp(cmdType, "fan") == 0)
        {
            TankControl_ManualFan(value);
        }
        else if (strcmp(cmdType, "mode") == 0)
        {
            TankControl_SetMode(value == 0 ? CONTROL_MODE_AUTO : CONTROL_MODE_MANUAL);
        }
        else if (strcmp(cmdType, "plant") == 0)
        {
            TankControl_SetPlantType(value);
        }
        else if (strcmp(cmdType, "settings") == 0)
        {
            // Parse settings from full payload
            // Expected: {"type":"settings","waterTempMin":20,"waterTempMax":28,...}
            TankParams params;
            const TankParams *current = TankControl_GetParams();
            memcpy(&params, current, sizeof(TankParams));

            char *ptr;
            ptr = strstr(cmdBuf, "\"waterTempMin\":");
            if (ptr) params.waterTempMin = (float)atof(ptr + 15);

            ptr = strstr(cmdBuf, "\"waterTempMax\":");
            if (ptr) params.waterTempMax = (float)atof(ptr + 15);

            ptr = strstr(cmdBuf, "\"waterLevelMin\":");
            if (ptr) params.waterLevelMin = atoi(ptr + 16);

            ptr = strstr(cmdBuf, "\"waterLevelMax\":");
            if (ptr) params.waterLevelMax = atoi(ptr + 16);

            ptr = strstr(cmdBuf, "\"lightThreshold\":");
            if (ptr) params.lightThreshold = atoi(ptr + 17);

            ptr = strstr(cmdBuf, "\"lightDuration\":");
            if (ptr) params.lightDuration = atoi(ptr + 16);

            TankControl_SetParams(&params);
        }
    }
}

static void MQTT_Task(void *arg)
{
    (void)arg;
    unsigned char buf[512];
    int buflen = sizeof(buf);

    printf("[MQTT] Task started\n");

    // Wait for system to stabilize before trying WiFi
    sleep(2);

    while (1)
    {
        // Wait for WiFi connection
        while (!WiFi_IsConnected())
        {
            WiFi_Connect();
            sleep(10);
        }

        printf("[MQTT] Connecting to broker...\n");

        // Connect to MQTT broker
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        data.clientID.cstring = MQTT_CLIENT_ID;
        data.keepAliveInterval = 60;
        data.cleansession = 1;
        data.username.cstring = MQTT_USERNAME;
        data.password.cstring = MQTT_PASSWORD;

        g_mqtt_socket = transport_open((char *)MQTT_HOST, MQTT_PORT);
        if (g_mqtt_socket < 0)
        {
            printf("[MQTT] Broker connect failed\n");
            sleep(10);
            continue;
        }

        int len = MQTTSerialize_connect(buf, buflen, &data);
        transport_sendPacketBuffer(g_mqtt_socket, buf, len);

        if (MQTTPacket_read(buf, buflen, transport_getdata) != CONNACK)
        {
            transport_close(g_mqtt_socket);
            sleep(10);
            continue;
        }

        unsigned char sessionPresent, connack_rc;
        if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
        {
            transport_close(g_mqtt_socket);
            sleep(10);
            continue;
        }

        printf("[MQTT] Connected\n");
        g_mqtt_connected = 1;

        // Subscribe to control topic
        MQTTString topicFilters[1];
        topicFilters[0].cstring = MQTT_TOPIC_CONTROL;
        int reqQos[1] = {1};
        len = MQTTSerialize_subscribe(buf, buflen, 0, 1, 1, topicFilters, reqQos);
        transport_sendPacketBuffer(g_mqtt_socket, buf, len);

        MQTTPacket_read(buf, buflen, transport_getdata);  // Wait for SUBACK

        // Main communication loop
        while (g_mqtt_connected)
        {
            // Publish sensor data every 2 seconds
            PublishSensorData(g_mqtt_socket);

            // Check for incoming messages (non-blocking with timeout)
            int packetType = MQTTPacket_read(buf, buflen, transport_getdata);
            if (packetType == PUBLISH)
            {
                unsigned char dup, retained;
                unsigned short packetId;
                int qos;
                MQTTString topicName;
                unsigned char *payloadIn;
                int payloadInLen;

                if (MQTTDeserialize_publish(&dup, &qos, &retained, &packetId,
                                            &topicName, &payloadIn, &payloadInLen,
                                            buf, buflen) == 1)
                {
                    HandleControlCommand((const char *)payloadIn, payloadInLen);
                }
            }
            else if (packetType == -1)
            {
                // Connection lost
                g_mqtt_connected = 0;
                break;
            }

            sleep(2);
        }

        transport_close(g_mqtt_socket);
        g_mqtt_socket = -1;
        sleep(5);
    }
}

void MQTT_MainLoop(void)
{
    osThreadAttr_t attr;
    attr.name = "MQTT_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = MQTT_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)MQTT_Task, NULL, &attr) == NULL)
    {
        printf("[MQTT] Failed to create task!\n");
    }
}
