#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Setup for DS18B20 temperature sensors using OneWire Bus:
// Data wire is connected to the Arduino digital pin 4
// In this case pin 2 is used, which refers to GPIO pin 2, which is D4 on the wemos D1 mini
#define ONE_WIRE_BUS 2
// Setup a oneWire instance to communicate with any OneWire devices
// (the protocoll used by the sensor)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// 
unsigned long timeTrack = 0;
float origTemp;

// wifi connection details
const char* ssid = "YOUR_SSID";
const char* pass = "YOUR_PASSWORD";

// Add your MQTT Broker IP address
const char* mqtt_server = "YOUR_IP";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're connected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connnected");
    } else {
      Serial.print("failed, rc=");
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {
  // put your setup code here, to run once:
  // initilize serial output console (at baud-rate)
  Serial.begin(115200);
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // read data from DS18B20 sensor :: setup
  // for setput assume the sensor is facing with the flat side to you (a single part)
  // connect red wire / VDD / right pin to 3.3 / 3V3 or 5V on arduino
  // connect black wire / GND / left pin to GND on arduino
  // connect yellow / DQ / middle to data pin on arduino and 4.7k ohm resistor to VDD

  // initilize the OneWire sensors:
  sensors.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  // important: need to check, whether the client is connected to the MQTT server
  if (!client.connected()) {
    // if not: reconnect to the server.
    reconnect();
  }
  // and then run the loop to make sure, data can be sent and received.
  client.loop();
    // Call sensors.requestTemperatures() to issue a gloabal temperture and Requests to all devices on the bus
    sensors.requestTemperatures();

    Serial.print("Temperature in degree Celsius: ");
    // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
    Serial.print(sensors.getTempCByIndex(0));
    Serial.print(" - Fahrenheit temperature: ");
    Serial.println(sensors.getTempFByIndex(0));
    // This is needed for the MQTT message as it cannot send floats.
    char temp[10];
    // Get the temperature in degrees celsius.
    origTemp = sensors.getTempCByIndex(0);
    Serial.print("This is the original temperature as a float: ");
    Serial.println(origTemp);
    // convert the float to char to send it via MQTT.
    dtostrf(origTemp, 6, 6, temp);
    // Send the message via MQTT.
    client.publish("esp8266/pool/temp", temp);
    Serial.println("value was sent to MQTT");
    Serial.print("Temperature in char is: ");
    for (int i = 0; i < sizeof(temp); i++) {
      Serial.print(temp[i]);
    }
    Serial.println();
    // The print to serial is so you do not need to look at MQTT.

    // Wait an hour.
    delay(3600000);
}
