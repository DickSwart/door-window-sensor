#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

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

///////////////////////////////////////////////////////////////////////////
//   Door sensors
///////////////////////////////////////////////////////////////////////////

#define DOOR_SENSOR_NAME_01 "door01"
#define DOOR_PIN_01 D1
#define DOOR_SENSOR_NAME_02 "door02"
#define DOOR_PIN_02 D2
#define DOOR_SENSOR_NAME_03 "door03"
#define DOOR_PIN_03 D5
#define DOOR_SENSOR_NAME_04 "door04"
#define DOOR_PIN_04 D6
#define DOOR_SENSOR_NAME_05 "door05"
#define DOOR_PIN_05 D7

///////////////////////////////////////////////////////////////////////////
//   Over-the-Air update (OTA)
///////////////////////////////////////////////////////////////////////////
#define OTA_PORT 8266  // port 8266 by default

#endif  // _USER_CONFIG_H_