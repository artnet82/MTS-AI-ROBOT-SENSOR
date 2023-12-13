#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

const int SD_CS_PIN = D8;

Adafruit_BME280 bme;

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

const char* grpcServer = "your_GRPC_SERVER";
const int grpcPort = 443;

const char* filePath = "/audio.wav";
File audioFile;

void setup() {
  Serial.begin(9600);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Ошибка инициализации SD-карты!");
    return;
  }

  if (!bme.begin(0x76)) {
    Serial.println("Ошибка инициализации BME280!");
    return;
  }

  connectToWiFi();

  while (true) {
    synthesizeAudio();

    sendAudioToTelegram();

    delay(120000); // Отправлять данные каждые 2 минуты
  }
}

void loop() {
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}

void synthesizeAudio() {
  DynamicJsonDocument jsonDoc(512);
  jsonDoc["text"] = "Текст вашего сообщения";
  jsonDoc["api_address"] = grpcServer;
  jsonDoc["auth_config"]["sso_server_url"] = "your_SSO_SERVER_URL";
  jsonDoc["auth_config"]["realm_name"] = "your_REALM_NAME";
  jsonDoc["auth_config"]["client_id"] = "your_CLIENT_ID";
  jsonDoc["auth_config"]["client_secret"] = "your_CLIENT_SECRET";
  String jsonStr;
  serializeJson(jsonDoc, jsonStr);

  WiFiClientSecure client;
  if (!client.connect(grpcServer, grpcPort)) {
    Serial.println("Не удалось подключиться к серверу gRPC!");
    return;
  }

  client.println("POST /synthesize HTTP/1.1");
  client.println("Host: " + String(grpcServer));
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(jsonStr.length()));
  client.println();
  client.println(jsonStr);
  client.println();

  audioFile = SD.open(filePath, FILE_WRITE);
  if (!audioFile) {
    Serial.println("Не удалось создать аудиофайл!");
    return;
  }

  bool headersReceived = false;
  while (client.connected()) {
    while (client.available()) {
      String line = client.readStringUntil('\r');
      if (line == "\n") {
        headersReceived = true;
        break;
      }
    }

    if (headersReceived) {
      while (client.available()) {
        char buf[512];
        int bytesRead = client.readBytes(buf, 512);
        audioFile.write(buf, bytesRead);
      }
    }
  }

  audioFile.close();

  client.stop();
}

void sendAudioToTelegram() {
  // TODO: add code to send audio file via Telegram
}

void printSensorData() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  Serial.println("===== Данные сенсоров =====");
  Serial.print("Температура: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Влажность: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Давление: ");
  Serial.print(pressure);
  Serial.println(" hPa");
}
