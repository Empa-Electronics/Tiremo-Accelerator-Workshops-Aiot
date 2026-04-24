#include "EMPA_MqttAws.h"
#include "../DebugLibrary/debug_framework.h"
#include <string.h>

MQTT_Config mqttConfig = {
    .mqttPacketBuffer = mqttPacketBuffer,
    .mode_wifi = STATION_MODE,
    .OSC_enable = SC_DISABLE,
    .wifiID = "YOUR_WIFI_SSID",
    .wifiPassword = "YOUR_WIFI_PASSWORD",
    .timezone = 3,
    .mode_mqtt = MQTT_TLS_1,
    .clientID = "GW-001",
    .username = "YOUR_MQTT_USERNAME",
    .mqttPassword = "YOUR_MQTT_PASSWORD",
    .keepAlive = 300,
    .cleanSession = CLS_1,
    .qos = QOS_0,
    .retain = RTN_0,
    .brokerAddress = "YOUR_BROKER_ADDRESS",
    .reconnect = 0,
    .subtopic = "devices/GW-001/cmd/config",
    .pubtopic = "devices/GW-001/tele/data_batch"
};

uint8_t MQTT_ConnectBroker(void)
{
    uint8_t tryCount = 0;

    while (tryCount < MAX_TRY_FUNC)
    {
        DebugFramework_Printf("[MQTT] Connect attempt %u/%u\n\r", tryCount + 1, MAX_TRY_FUNC);
        if (MQTT_Init(&mqttConfig) == FUNC_SUCCESSFUL)
        {
            LED_Mqttconnected(1);
            DebugFramework_PutsLine("[MQTT] Connected to broker!");

            /* Publish connection confirmation */
            char msg[] = "mqtt successful";
            memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
            LED_MqttTXBlink();
            Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
                (uint16_t)strlen(msg), msg, QOS_0, RTN_0, POLLING_MODE);
            return 0;
        }
        tryCount++;
    }

    LED_Mqttconnected(0);
    DebugFramework_PutsLine("[MQTT] Connection FAILED after all retries");
    return 1;
}

void MQTT_PublishSensorData(const SensorData_t *pData)
{
    static char jsonBuf[256];

    uint16_t len = Sensor_FormatJSON(pData, jsonBuf, sizeof(jsonBuf));

    DebugFramework_Printf("[MQTT] Publishing %u bytes: %s\n\r", len, jsonBuf);

    memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
    LED_MqttTXBlink();
    Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
        len, jsonBuf, QOS_0, RTN_0, POLLING_MODE);
}

void MY_MqttAwsProcess(void)
{
    if (MQTT_ConnectBroker() == 0)
    {
        char msg[] = "mqtt successful";
        memset(mqttPacketBuffer, 0, MQTT_DATA_PACKET_BUFF_SIZE);
        LED_MqttTXBlink();
        Wifi_MqttPubRaw2(mqttPacketBuffer, mqttConfig.pubtopic,
            (uint16_t)strlen(msg), msg, QOS_0, RTN_0, POLLING_MODE);
    }
}
