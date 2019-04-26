#ifndef _USER_CONFIG_OVERRIDE_H_
#define _USER_CONFIG_OVERRIDE_H_

// force the compiler to show a warning to confirm that this file is inlcuded
#warning **** user_config_override.h: Using Settings from this File ****

///////////////////////////////////////////////////////////////////////////
//   ESP8266-01
///////////////////////////////////////////////////////////////////////////
#undef  TX_PIN
#define TX_PIN 7

#undef  RX_PIN
#define RX_PIN 6

///////////////////////////////////////////////////////////////////////////
//   WIFI
///////////////////////////////////////////////////////////////////////////
#undef  WIFI_SSID
#define WIFI_SSID "wifi_ssid"

#undef  WIFI_PASSWORD
#define WIFI_PASSWORD "wifi_password"

#undef  WIFI_QUALITY_OFFSET_VALUE
#define WIFI_QUALITY_OFFSET_VALUE 2

#undef  WIFI_QUALITY_INTERVAL
#define WIFI_QUALITY_INTERVAL 50000       // [ms]

#undef  WIFI_QUALITY_SENSOR_NAME
#define WIFI_QUALITY_SENSOR_NAME "wifi"

///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////
#undef  MQTT_SERVER
#define MQTT_SERVER "xxx.xxx.xxx.xxx"

#undef  MQTT_SERVER_PORT
#define MQTT_SERVER_PORT 1883

#undef  MQTT_USERNAME
#define MQTT_USERNAME "mqtt_user_name"

#undef  MQTT_PASSWORD
#define MQTT_PASSWORD "mqtt_password"

#undef  MQTT_AVAILABILITY_TOPIC_TEMPLATE
#define MQTT_AVAILABILITY_TOPIC_TEMPLATE "%s/status" // MQTT availability: online/offline

#undef  MQTT_SENSOR_TOPIC_TEMPLATE
#define MQTT_SENSOR_TOPIC_TEMPLATE "%s/sensor/%s"

#undef  MQTT_LIGHT_STATE_TOPIC_TEMPLATE
#define MQTT_LIGHT_STATE_TOPIC_TEMPLATE "%s/light/state"

#undef  MQTT_LIGHT_COMMAND_TOPIC_TEMPLATE
#define MQTT_LIGHT_COMMAND_TOPIC_TEMPLATE "%s/light/set"

#undef  MQTT_PAYLOAD_ON
#define MQTT_PAYLOAD_ON "ON"

#undef  MQTT_PAYLOAD_OFF
#define MQTT_PAYLOAD_OFF "OFF"

#undef  MQTT_CONNECTION_TIMEOUT
#define MQTT_CONNECTION_TIMEOUT 5000

///////////////////////////////////////////////////////////////////////////
//   Door sensors
///////////////////////////////////////////////////////////////////////////

#undef  DOOR_SENSOR_NAME_01
#define DOOR_SENSOR_NAME_01 "door01"
#undef  DOOR_PIN_01
#define DOOR_PIN_01 9

#undef  DOOR_SENSOR_NAME_02
#define DOOR_SENSOR_NAME_02 "door02"
#undef  DOOR_PIN_02
#define DOOR_PIN_02 10

#undef  DOOR_SENSOR_NAME_03
#define DOOR_SENSOR_NAME_03 "door03"
#undef  DOOR_PIN_03
#define DOOR_PIN_03 11

#undef  DOOR_SENSOR_NAME_04
#define DOOR_SENSOR_NAME_04 "door04"
#undef  DOOR_PIN_04
#define DOOR_PIN_04 12

#undef  DOOR_SENSOR_NAME_05
#define DOOR_SENSOR_NAME_05 "door05"
#undef  DOOR_PIN_05
#define DOOR_PIN_05 13

///////////////////////////////////////////////////////////////////////////
//   Over-the-Air update (OTA)
///////////////////////////////////////////////////////////////////////////
#undef  OTA_PORT
#define OTA_PORT 8266  // port 8266 by default


#endif  // _USER_CONFIG_OVERRIDE_H_