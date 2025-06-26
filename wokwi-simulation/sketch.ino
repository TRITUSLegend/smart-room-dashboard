#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define DHTPIN 12
#define DHTTYPE DHT22
#define BUTTON_PIN 5
#define LIGHT_PIN 2
#define FAN_PIN 4

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* FIREBASE_HOST = "home-automation-system-1dbcf-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* FIREBASE_PATH = "/room1.json";
const int HTTPS_PORT = 443;

DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  client.setInsecure();
}

void loop() {
  bool motionDetected = digitalRead(BUTTON_PIN);
  float temp = dht.readTemperature();

  if (isnan(temp)) {
    Serial.println("DHT read failed");
    delay(1000);
    return;
  }

  // 1. Read Firebase fan/light control
  String fanState = "OFF";
  String lightState = "OFF";

  if (client.connect(FIREBASE_HOST, HTTPS_PORT)) {
    client.print(String("GET ") + FIREBASE_PATH + " HTTP/1.1\r\n" +
                 "Host: " + FIREBASE_HOST + "\r\n" +
                 "Connection: close\r\n\r\n");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }

    String response = client.readString();
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      fanState = doc["fan"] | "OFF";
      lightState = doc["light"] | "OFF";
    } else {
      Serial.println("GET JSON parse failed");
    }
    client.stop();
  } else {
    Serial.println("GET connection failed");
  }

  // 2. Apply control
  digitalWrite(FAN_PIN, fanState == "ON" ? HIGH : LOW);
  digitalWrite(LIGHT_PIN, lightState == "ON" ? HIGH : LOW);

  // 3. Upload temperature & motion only
  String payload = "{";
  payload += "\"temperature\":" + String(temp, 1) + ",";
  payload += "\"motion\":" + String(motionDetected ? "true" : "false");
  payload += "}";

  if (client.connect(FIREBASE_HOST, HTTPS_PORT)) {
    client.print(String("PATCH /room1.json HTTP/1.1\r\n") +
                 "Host: " + FIREBASE_HOST + "\r\n" +
                 "Content-Type: application/json\r\n" +
                 "Content-Length: " + payload.length() + "\r\n\r\n" +
                 payload + "\r\n");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") break;
    }
    client.readString();
    client.stop();
  } else {
    Serial.println("PATCH connection failed");
  }

  delay(1000);
}

