#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include "WiFiEsp.h"      // https://github.com/bportaluri/WiFiEsp
#include "SwartNinjaReedSwitch.h"

#include "user_config.h"                 // Fixed user configurable options
#ifdef USE_CONFIG_OVERRIDE
  #include "user_config_override.h"         // Configuration overrides for my_user_config.h
#endif

// Emulate Serial1 on pins TX_PIN  and RX_PIN if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(TX_PIN, RX_PIN); //  TX, RX
#endif

/* -------------------------------------------------
 *  WiFi
 * ------------------------------------------------- */
// function declaration
void setupWiFi();
void connectWiFi();
void getWiFiQuality();
long calculateWiFiQuality(void);

// variables declaration
int previousWiFiQualityMeasure = -1;

// Initialize the Ethernet client object
WiFiEspClient wifiClient;

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

///////////////////////////////////////////////////////////////////////////
//   Main Setup & loop
///////////////////////////////////////////////////////////////////////////
void setup()
{
  // initialize serial for debugging
  Serial.begin(115200);

  // WiFi
  setupWiFi();

  // MQTT
  setupMQTT();

  // Reed Switched
  door01.init();
  door02.init();
  door03.init();
  door04.init();
  door05.init();
}

void loop()
{
  // WiFi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WiFi]: WiFi Disconnected. Attempting reconnection.");
    delay(10);
    connectWiFi();
    return;
  }

  // MQTT
  if (!mqttClient.connected())
  {
    connectToMQTT();
  }
  mqttClient.loop();

  // Sensors
  getWiFiQuality();

  door01.loop();
  door02.loop();
  door03.loop();
  door04.loop();
  door05.loop();
}

///////////////////////////////////////////////////////////////////////////
//   WiFi
///////////////////////////////////////////////////////////////////////////

/*
 * Function called to setup the connection to the WiFi AP
 */
void setupWiFi()
{
  // initialize serial for ESP module
  Serial1.begin(9600);

  // initialize ESP module
  WiFi.init(&Serial1);

  // connect to WiFi
  connectWiFi();
}

/*
 * Function called to make WiFi connection
 */
void connectWiFi()
{
  Serial.println("--------------------------- WIFI ---------------------------");
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true)
      ;
  }

  // attempt to connect to WiFi network
  bool showAttemptingToConnectMsg = true;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (showAttemptingToConnectMsg)
    {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(WIFI_SSID);
      showAttemptingToConnectMsg = false;
    }

    // Connect to WPA/WPA2 network
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("You're connected to the network");

  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to
  byte bssid[6];
  WiFi.BSSID(bssid);
  char buf[20];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", bssid[5], bssid[4], bssid[3], bssid[2], bssid[1], bssid[0]);
  Serial.print("BSSID: ");
  Serial.println(buf);

  // print the received signal strength
  Serial.print("Signal strength : ");
  Serial.println(calculateWiFiQuality());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
  Serial.print("MAC address: ");
  Serial.println(buf);
  Serial.println("------------------------------------------------------------");
}

/*
 * Function called to in main loop
 */
void getWiFiQuality()
{
  static unsigned long lastWiFiQualityMeasure = 0;
  if (lastWiFiQualityMeasure + WIFI_QUALITY_INTERVAL <= millis() || previousWiFiQualityMeasure == -1)
  {
    lastWiFiQualityMeasure = millis();
    int currentWiFiQuality = calculateWiFiQuality();
    if (isnan(previousWiFiQualityMeasure) || currentWiFiQuality <= previousWiFiQualityMeasure - WIFI_QUALITY_OFFSET_VALUE || currentWiFiQuality >= previousWiFiQualityMeasure + WIFI_QUALITY_OFFSET_VALUE)
    {
      Serial.println("[WiFi Quality Loop]: Send MQTT message.");
      previousWiFiQualityMeasure = currentWiFiQuality;
      dtostrf(currentWiFiQuality, 2, 2, MQTT_PAYLOAD);
      publishToMQTT(MQTT_WIFI_QUALITY_TOPIC, MQTT_PAYLOAD);
    }
  }
}

/*
 * Helper function to detirmine signal strength
 */
long calculateWiFiQuality(void)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    return -1;
  }

  long dBm = WiFi.RSSI();
  if (dBm <= -100)
  {
    return 0;
  }
  else if (dBm >= -50)
  {
    return 100;
  }

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
  byte mac[6];
  WiFi.macAddress(mac);
  sprintf(MQTT_CLIENT_ID, "%02X%02X%02X", mac[2], mac[1], mac[0]);
  sprintf(MQTT_AVAILABILITY_TOPIC, MQTT_AVAILABILITY_TOPIC_TEMPLATE, MQTT_CLIENT_ID);

  Serial.print(F("[MQTT Setup]: MQTT availability topic: "));
  Serial.println(MQTT_AVAILABILITY_TOPIC);

  sprintf(MQTT_WIFI_QUALITY_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, WIFI_QUALITY_SENSOR_NAME);
  Serial.print(F("[MQTT Setup]: MQTT WiFi Quality topic: "));
  Serial.println(MQTT_WIFI_QUALITY_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_01_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_01);
  Serial.print(F("[MQTT Setup]: MQTT Door 1 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_01_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_02_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_02);
  Serial.print(F("[MQTT Setup]: MQTT Door 2 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_02_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_03_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_03);
  Serial.print(F("[MQTT Setup]: MQTT Door 3 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_03_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_04_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_04);
  Serial.print(F("[MQTT Setup]: MQTT Door 4 topic: "));
  Serial.println(MQTT_DOOR_SENSOR_04_TOPIC);

  sprintf(MQTT_DOOR_SENSOR_05_TOPIC, MQTT_SENSOR_TOPIC_TEMPLATE, MQTT_CLIENT_ID, DOOR_SENSOR_NAME_05);
  Serial.print(F("[MQTT Setup]: MQTT Door 5 topic: "));
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
      handleDoorStateUpdate(door01.getCurrentState(), DOOR_PIN_01);
      handleDoorStateUpdate(door02.getCurrentState(), DOOR_PIN_02);
      handleDoorStateUpdate(door03.getCurrentState(), DOOR_PIN_03);
      handleDoorStateUpdate(door04.getCurrentState(), DOOR_PIN_04);
      handleDoorStateUpdate(door05.getCurrentState(), DOOR_PIN_05);
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

///////////////////////////////////////////////////////////////////////////
//   SwartNinjaReedSwitch
///////////////////////////////////////////////////////////////////////////

void handleDoorStateUpdate(bool state, int pinNumber)
{
  Serial.print("[SwartNinjaReedSwitch]: handleDoorStateUpdate - Pin ");
  Serial.print(pinNumber);
  Serial.print(" = ");
  Serial.println(state ? MQTT_PAYLOAD_ON : MQTT_PAYLOAD_OFF);

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