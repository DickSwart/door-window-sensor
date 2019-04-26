#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

///////////////////////////////////////////////////////////////////////////
//   ESP8266-01
///////////////////////////////////////////////////////////////////////////
#define TX_PIN 7
#define RX_PIN 6

///////////////////////////////////////////////////////////////////////////
//   WIFI
///////////////////////////////////////////////////////////////////////////
#define WIFI_SSID "wifi_ssid"
#define WIFI_PASSWORD "wifi_password"
#define WIFI_QUALITY_OFFSET_VALUE 2
#define WIFI_QUALITY_INTERVAL 50000 // [ms]
#define WIFI_QUALITY_SENSOR_NAME "wifi"

///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////
#define MQTT_SERVER "xxx.xxx.xxx.xxx"
#define MQTT_SERVER_PORT 1883
#define MQTT_USERNAME "mqtt_user_name"
#define MQTT_PASSWORD "mqtt_password"

#define MQTT_AVAILABILITY_TOPIC_TEMPLATE "%s/status" // MQTT availability: online/offline
#define MQTT_SENSOR_TOPIC_TEMPLATE "%s/sensor/%s"

#define MQTT_PAYLOAD_ON "ON"
#define MQTT_PAYLOAD_OFF "OFF"

#undef  MQTT_CONNECTION_TIMEOUT
#define MQTT_CONNECTION_TIMEOUT 5000

///////////////////////////////////////////////////////////////////////////
//   Door sensors
///////////////////////////////////////////////////////////////////////////

#define DOOR_SENSOR_NAME_01 "door01"
#define DOOR_PIN_01 9

#define DOOR_SENSOR_NAME_02 "door02"
#define DOOR_PIN_02 10

#define DOOR_SENSOR_NAME_03 "door03"
#define DOOR_PIN_03 11

#define DOOR_SENSOR_NAME_04 "door04"
#define DOOR_PIN_04 12

#define DOOR_SENSOR_NAME_05 "door05"
#define DOOR_PIN_05 13

///////////////////////////////////////////////////////////////////////////
//   Over-the-Air update (OTA)
///////////////////////////////////////////////////////////////////////////
#define OTA_PORT 8266  // port 8266 by default

#endif  // _USER_CONFIG_H_