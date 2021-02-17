/*
  This code is based on:
  Developed by M V Subrahmanyam - https://www.linkedin.com/in/veera-subrahmanyam-mediboina-b63997145/
  Project: AWS | NodeMCU ESP8266 Tutorials
  Electronics Innovation - www.electronicsinnovation.com
  GitHub - https://github.com/VeeruSubbuAmi
  YouTube - http://bit.ly/Electronics_Innovation
  ¡Thanks to Veera Subrahmanyam!
  This code was modified by Juan Camilo Velez Olaya (juanchovelezpro)
  GitHub - https://github.com/juanchovelezpro
  All modifications were made for a specific project to control and monitoring devices from a "Smart Home".
*/

// Libraries
#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Network settings
const char* ssid = "ssid";
const char* password = "pass";

//NTP Server ->> To have the date and hour.
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Endpoint AWS
const char* AWS_endpoint = "endpoint";

// A callback to get all messages of a subscribed topic.
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print(message);
  receiveData(message);
  Serial.println();
}

WiFiClientSecure espClient; // A secure client with esp.
PubSubClient client(AWS_endpoint, 8883, callback, espClient); // Publisher Subscriber client in the AWS endpoint.
//============================================================================
#define BUFFER_LEN 256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
//============================================================================

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(LED_BUILTIN, OUTPUT);
  setup_wifi();
  delay(1000);
  loadCertificates();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  publishData();
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100); // wait for a second
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(100); // wait for a second
}


void receiveData(String message) {

  DynamicJsonDocument dataIn(1024);

  deserializeJson(dataIn, message);

  String command = dataIn["command"];

  if(command.equals("switch")){
    digitalWrite(dataIn["switch"], HIGH);
  }

  // Get the variables of the data sent it.

  /////////////////////////////////////

  // int number = dataIn["myNumber"];

  ////////////////////////////////////


  // Something to do with the variables ... ///

  // if(number > 0) { digitalWrite(D1, HIGH); }


  /////////////////////////////////////////////

}

void publishData() {

  long now = millis();
  if (now - lastMsg > 60000) { // If we need to send the data every certain time.
    lastMsg = now;

    DynamicJsonDocument dataOut(1024); // Variable to send json to AWS IoT Core.

    // Values to send to database

    /////////////////////////////////

    // dataOut["attribute"] = "result";

    /////////////////////////////////

    serializeJson(dataOut, msg);

    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("homeDevice", msg);

    Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
  }
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPthing")) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      client.publish("homeDevice", "{\"message\":\"Connected!\"}");
      // ... and resubscribe
      client.subscribe("homeDeviceAction"); // To receive data.
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      espClient.getLastSSLError(buf, 256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Load the certificates saved in the ESP8266 filesystem into the execution program.
void loadCertificates() {

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());

  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");

  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
  if (!ca) {
    Serial.println("Failed to open ca ");
  }
  else
    Serial.println("Success to open ca");

  delay(1000);

  if (espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
}
