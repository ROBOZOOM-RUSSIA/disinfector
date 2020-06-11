/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#define Transistor 4
#define Sensor_1   2

// Set these to your desired credentials.
const char* ssid     = "AntiCovid";
const char* password = "qwe456rty";
const char* motion = "No Motion";

WiFiServer server(80);
char lineBuf[80];
int charCount = 0;
int x = 0;

void setup() {

  pinMode(5, OUTPUT);
  pinMode(Transistor, OUTPUT);
  pinMode(Sensor_1, INPUT);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  x =  digitalRead(Sensor_1);
  Serial.println(x);
  if (x == 1) {
    motion = "Motion detected";
    digitalWrite(Transistor, HIGH);
    digitalWrite(5, HIGH);
  } else {
    motion = "No motion";
    digitalWrite(Transistor, LOW);
    digitalWrite(5, LOW);
  }

  if (client) {
    Serial.println("New client");
    memset(lineBuf, 0, sizeof(lineBuf));
    charCount = 0;
    // HTTP-запрос заканчивается пустой строкой
    boolean currentLineIsBlank = true;
    while (client.connected()) {


      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // считываем HTTP-запрос
        lineBuf[charCount] = c;
        if (charCount < sizeof(lineBuf) - 1) {
          charCount++;
        }
        // на символ конца строки отправляем ответ
        if (c == '\n' && currentLineIsBlank) {
          // отправляем стандартный заголовок HTTP-ответа
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          // тип контента: text/html
          client.println("Connection: close");
          // после отправки ответа связь будет отключена
          client.println();
          // формируем веб-страницу
          String webPage = "<!DOCTYPE HTML>";
          webPage += "<html>";
          webPage += "  <head>";
          webPage += "    <meta name=\"viewport\" content=\"width=device-width,";
          webPage += "    initial-scale=1\">";
          webPage += "  </head>";
          webPage += "  <h1>ESP32 - Web Server</h1>";
          webPage += "  <p>LED #1";
          webPage += "    <a href=\"on1\">";
          webPage += "      <button>ON</button>";
          webPage += "    </a>&nbsp;";
          webPage += "    <a href=\"off1\">";
          webPage += "      <button>OFF</button>";
          webPage += "    </a>";
          webPage += "  </p>";
          webPage += "   digitalPin 4 = ";
          webPage +=    motion;
          webPage += "  <br>";
          webPage += "  </p>";
          webPage += "</html>";
          client.println(webPage);
          break;
        }
        if (c == '\n') {
          // анализируем буфер на наличие запросов
          // если есть запрос, меняем состояние светодиода
          currentLineIsBlank = true;
          if (strstr(lineBuf, "GET /on1") > 0) {
            Serial.println("LED 1 ON");
            digitalWrite(Transistor, HIGH);
          } else if (strstr(lineBuf, "GET /off1") > 0) {
            Serial.println("LED 1 OFF");
            digitalWrite(Transistor, LOW);
          }
          else if (digitalRead(2) == 1) {
            Serial.println("Motion detected");
          }
          else if (digitalRead(2) == 0) {
            Serial.println("Mo Motion");
          }

          // начинаем новую строку
          currentLineIsBlank = true;
          memset(lineBuf, 0, sizeof(lineBuf));
          charCount = 0;
        } else if (c != '\r') {
          // в строке попался новый символ
          currentLineIsBlank = false;
        }
      }
    }
    // даем веб-браузеру время, чтобы получить данные
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
