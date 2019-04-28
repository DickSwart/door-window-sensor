#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>  // https://github.com/esp8266/Arduino
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <WiFiClient.h>
#include "SwartNinjaReedSwitch.h"

#include "user_config.h" // Fixed user configurable options
#ifdef USE_CONFIG_OVERRIDE
#include "user_config_override.h" // Configuration overrides for my_user_config.h
#endif

/* -------------------------------------------------
 *  WiFi
 * ------------------------------------------------- */
// variables declaration
int previousWiFiSignalStrength = -1;
unsigned long previousMillis = 0;
int reqConnect = 0;
int isConnected = 0;
const long interval = 500;
const long reqConnectNum = 15; // number of intervals to wait for connection
WiFiEventHandler mConnectHandler;
WiFiEventHandler mDisConnectHandler;
WiFiEventHandler mGotIpHandler;

// function declaration
void setupWiFi(void);
void connectWiFi(void);
void onConnected(const WiFiEventStationModeConnected &event);
void onDisconnect(const WiFiEventStationModeDisconnected &event);
void onGotIP(const WiFiEventStationModeGotIP &event);
void loopWiFiSensor(void);
int getWiFiSignalStrength(void);

// Initialize the Ethernet client object
WiFiClient wifiClient;

/* -------------------------------------------------
 *  MQTT
 * ------------------------------------------------- */
// function declaration
void setupMQTT();
void connectToMQTT();
void subscribeToMQTT(char *p_topic);
void publishToMQTT(char *p_topic, char *p_payload);

// variables declaration
volatile unsigned long lastMQTTConnection = 0;
char MQTT_CLIENT_ID[7] = {0};
char MQTT_PAYLOAD[8] = {0};
char MQTT_AVAILABILITY_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_AVAILABILITY_TOPIC_TEMPLATE) - 2] = {0};
char MQTT_WIFI_QUALITY_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(WIFI_QUALITY_SENSOR_NAME) - 4] = {0};
char MQTT_DOOR_SENSOR_01_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DOOR_SENSOR_NAME_01) - 4] = {0};
char MQTT_DOOR_SENSOR_02_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DOOR_SENSOR_NAME_02) - 4] = {0};
char MQTT_DOOR_SENSOR_03_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DOOR_SENSOR_NAME_03) - 4] = {0};
char MQTT_DOOR_SENSOR_04_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DOOR_SENSOR_NAME_04) - 4] = {0};
char MQTT_DOOR_SENSOR_05_TOPIC[sizeof(MQTT_CLIENT_ID) + sizeof(MQTT_SENSOR_TOPIC_TEMPLATE) + sizeof(DOOR_SENSOR_NAME_05) - 4] = {0};

// Initialize the mqtt client object
PubSubClient mqttClient(wifiClient);

/* -------------------------------------------------
 *  SwartNinjaReedSwitch
 * ------------------------------------------------- */
// function declaration
void handleDoorStateUpdate(bool state, int pinNumber);

// Initialize the reed switch objects
SwartNinjaReedSwitch door01(DOOR_PIN_01, handleDoorStateUpdate);
SwartNinjaReedSwitch door02(DOOR_PIN_02, handleDoorStateUpdate);
SwartNinjaReedSwitch door03(DOOR_PIN_03, handleDoorStateUpdate);
SwartNinjaReedSwitch door04(DOOR_PIN_04, handleDoorStateUpdate);
SwartNinjaReedSwitch door05(DOOR_PIN_05, handleDoorStateUpdate);
// add switches to an array
SwartNinjaReedSwitch switches[5] = {door01, door02, door03, door04, door05};

///////////////////////////////////////////////////////////////////////////
//   Main Setup & loop
///////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);

  // WIFI
  setupWiFi();

  // MQTT
  setupMQTT();

  // Reed Switched
  for (int i = 0; i < 5; i++)
  {
    switches[i].init();

    // #ifdef DEBUG
    //     Serial.println("[SETUP]: Pin ");
    //     Serial.print(switches[i].getPinNumber());
    //     Serial.print(" = ");
    //     Serial.println(switches[i].getCurrentState() ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF);
    // #endif
  }
}

void loop()
{
  // WIFI
  connectWiFi();

  // Code will only run if connected to WiFi
  if (isConnected == 2)
  {
    // MQTT
    if (!mqttClient.connected())
    {
      connectToMQTT();
    }
    mqttClient.loop();

    // Check WiFi signal
    loopWiFiSensor();

    // Switches
    for (int i = 0; i < 5; i++)
    {
      switches[i].loop();
    }
  }
}

///////////////////////////////////////////////////////////////////////////
//   SwartNinjaReedSwitch
///////////////////////////////////////////////////////////////////////////

void handleDoorStateUpdate(bool state, int pinNumber)
{
  switch (pinNumber)
  {
  case DOOR_PIN_01:
    if (state)
    {
      publishToMQTT(MQTT_DOOR_SENSOR_01_TOPIC, MQTT_PAYLOAD_ON);
    }
    else
    {
      publishToMQTT(MQTT_DOOR_SENSOR_01_TOPIC, MQTT_PAYLOAD_OFF);
    }
    break;
  case DOOR_PIN_02:
    if (state)
    {
      publishToMQTT(MQTT_DOOR_SENSOR_02_TOPIC, MQTT_PAYLOAD_ON);
    }
    else
    {
      publishToMQTT(MQTT_DOOR_SENSOR_02_TOPIC, MQTT_PAYLOAD_OFF);
    }

    break;
  case DOOR_PIN_03:
    if (state)
    {
      publishToMQTT(MQTT_DOOR_SENSOR_03_TOPIC, MQTT_PAYLOAD_ON);
    }
    else
    {
      publishToMQTT(MQTT_DOOR_SENSOR_03_TOPIC, MQTT_PAYLOAD_OFF);
    }
    break;
  case DOOR_PIN_04:
    if (state)
    {
      publishToMQTT(MQTT_DOOR_SENSOR_04_TOPIC, MQTT_PAYLOAD_ON);
    }
    else
    {
      publishToMQTT(MQTT_DOOR_SENSOR_04_TOPIC, MQTT_PAYLOAD_OFF);
    }
    break;
  case DOOR_PIN_05:
    if (state)
    {
      publishToMQTT(MQTT_DOOR_SENSOR_05_TOPIC, MQTT_PAYLOAD_ON);
    }
    else
    {
      publishToMQTT(MQTT_DOOR_SENSOR_05_TOPIC, MQTT_PAYLOAD_OFF);
    }
    break;

  default:
    Serial.print("[SwartNinjaReedSwitch]: handleDoorStateUpdate - Pin ");
    Serial.print(pinNumber);
    Serial.print(" = ");
    Serial.println(state ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF);
    break;
  }
}
///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////

/*
 * Function called to setup WiFi module
 */
void setupWiFi(void)
{
  WiFi.disconnect();
  WiFi.persistent(false);
  mConnectHandler = WiFi.onStationModeConnected(onConnected);
  mDisConnectHandler = WiFi.onStationModeDisconnected(onDisconnect);
  mGotIpHandler = WiFi.onStationModeGotIP(onGotIP);
}

/*
 * Function called to connect to WiFi
 */
void connectWiFi(void)
{
  if (WiFi.status() != WL_CONNECTED && reqConnect > reqConnectNum && isConnected < 2)
  {
    reqConnect = 0;
    isConnected = 0;
    WiFi.disconnect();

    Serial.println();
    Serial.print("[WIFI]: Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("[WIFI]: Connecting...");
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    reqConnect++;
  }
}

/*
 * Function called to handle WiFi events
 */
void onConnected(const WiFiEventStationModeConnected &event)
{
  char macAdddress[20];
  sprintf(macAdddress, "%02X:%02X:%02X:%02X:%02X:%02X", event.bssid[5], event.bssid[4], event.bssid[3], event.bssid[2], event.bssid[1], event.bssid[0]);
  Serial.print(F("[WIFI]: You're connected to the AP. (MAC - "));
  Serial.print(macAdddress);
  Serial.println(")");
  isConnected = 1;
}

void onDisconnect(const WiFiEventStationModeDisconnected &event)
{
  Serial.println("[WIFI]: Disconnected");
  Serial.print("[WIFI]: Reason: ");
  Serial.println(event.reason);
  isConnected = 0;
}

void onGotIP(const WiFiEventStationModeGotIP &event)
{
  Serial.print("[WIFI]: IP Address : ");
  Serial.println(event.ip);
  Serial.print("[WIFI]: Subnet     : ");
  Serial.println(event.mask);
  Serial.print("[WIFI]: Gateway    : ");
  Serial.println(event.gw);

  isConnected = 2;
}

/*
 * Function to check WiFi signal strength
 */
void loopWiFiSensor(void)
{
  static unsigned long lastWiFiQualityMeasure = 0;
  if (lastWiFiQualityMeasure + WIFI_QUALITY_INTERVAL <= millis() || previousWiFiSignalStrength == -1)
  {
    lastWiFiQualityMeasure = millis();
    int currentWiFiSignalStrength = getWiFiSignalStrength();
    if (isnan(previousWiFiSignalStrength) || currentWiFiSignalStrength <= previousWiFiSignalStrength - WIFI_QUALITY_OFFSET_VALUE || currentWiFiSignalStrength >= previousWiFiSignalStrength + WIFI_QUALITY_OFFSET_VALUE)
    {
      previousWiFiSignalStrength = currentWiFiSignalStrength;
      // dtostrf(currentWiFiQuality, 2, 2, MQTT_PAYLOAD);
      // publishToMQTT(MQTT_WIFI_QUALITY_TOPIC, MQTT_PAYLOAD);
    }
  }
}

/*
 * Helper function to get the current WiFi signal strength
 */
int getWiFiSignalStrength(void)
{
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

///////////////////////////////////////////////////////////////////////////
//   MQTT
///////////////////////////////////////////////////////////////////////////

/*
 * Function called to setup MQTT topics
 */
void setupMQTT()
{
  sprintf(MQTT_CLIENT_ID, "%06X", ESP.getChipId());
  sprintf(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_TOPIC_TEMPLATE, MQTT_CLIENT_ID);

  Serial.print(F("[MQTT]: MQTT availability topic: "));
  Serial.println(MQTT_AVAILABILITY_TOPIC);

  sprintf(MQTT_WIFI_QUALITY_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, WIFI_QUALITY_SENSOR_NAME);
  Serial.print(F("[MQTT]: MQTT WiFi Quality topic: "));
  Serial.println(MQTT_WIFI_QUALITY_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_01_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_01);
  Serial.print(F("[MQTT]: MQTT Door 1 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_01_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_02_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_02);
  Serial.print(F("[MQTT]: MQTT Door 2 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_02_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_03_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_03);
  Serial.print(F("[MQTT]: MQTT Door 3 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_03_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_04_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_04);
  Serial.print(F("[MQTT]: MQTT Door 4 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_04_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_05_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_05);
  Serial.print(F("[MQTT]: MQTT Door 5 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_05_TOPIC);

  mqttClient.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
}

/*
  Function called to connect/reconnect to the MQTT broker
*/
void connectToMQTT()
{
  // Loop until we're connected / reconnected
  while (!mqttClient.connected())
  {
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_AVAILABILITY_TOPIC, 0, 1, "offline"))
    {
      Serial.println(F("[MQTT]: The client is successfully connected to the MQTT broker"));
      publishToMQTT(MQTT_AVAILABILITY_TOPIC, "online");

      // send initial values for door sensors
      for (int i = 0; i < 5; i++)
      {
        handleDoorStateUpdate(switches[i].getCurrentState(), switches[i].getPinNumber());
      }
    }
    else
    {
      Serial.println(F("[MQTT]: ERROR - The connection to the MQTT broker failed"));
      Serial.print(F("[MQTT]: MQTT username: "));
      Serial.println(MQTT_USERNAME);
      Serial.print(F("[MQTT]: MQTT password: "));
      Serial.println(MQTT_PASSWORD);
      Serial.print(F("[MQTT]: MQTT broker: "));
      Serial.println(MQTT_SERVER);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
  Function called to subscribe to a MQTT topic
*/
void subscribeToMQTT(char *p_topic)
{
  if (mqttClient.subscribe(p_topic))
  {
    Serial.print(F("[MQTT]: subscribeToMQTT - Sending the MQTT subscribe succeeded for topic: "));
    Serial.println(p_topic);
  }
  else
  {
    Serial.print(F("[MQTT]: subscribeToMQTT - ERROR, Sending the MQTT subscribe failed for topic: "));
    Serial.println(p_topic);
  }
}

/*
  Function called to publish to a MQTT topic with the given payload
*/
void publishToMQTT(char *p_topic, char *p_payload)
{
  if (mqttClient.publish(p_topic, p_payload, true))
  {
    Serial.print(F("[MQTT]: publishToMQTT - MQTT message published successfully, topic: "));
    Serial.print(p_topic);
    Serial.print(F(", payload: "));
    Serial.println(p_payload);
  }
  else
  {
    Serial.println(F("[MQTT]: publishToMQTT - ERROR, MQTT message not published, either connection lost, or message too large. Topic: "));
    Serial.print(p_topic);
    Serial.print(F(" , payload: "));
    Serial.println(p_payload);
  }
}
