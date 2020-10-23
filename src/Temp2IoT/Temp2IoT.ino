#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <stdio.h>

#include "WifiCredentials.h" //Make sure that you have put your credentials in the 'WifiCredentials.h.template' file and renamed the file to 'WifiCredentials.h'
#include "WebUi.h"

#define HTTP_REST_PORT 80
#define MAX_WIFI_INIT_RETRY 50
#define ONE_WIRE_BUS D3
#define WIFI_RETRY_DELAY 500


ESP8266WebServer http_rest_server(HTTP_REST_PORT);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char TemperatureStr[6];
unsigned int SecureCounter;


int init_wifi() {
    int retries = 0;

    Serial.println("Connecting to WiFi AP..........");

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_passwd);
    // check the status of WiFi connection to be WL_CONNECTED
    while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
        retries++;
        delay(WIFI_RETRY_DELAY);
        Serial.print("#");
    }
    return WiFi.status(); // return the WiFi connection status
}

void readTemperature() {
    Serial.println("Reading");
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on
    float temp;
    int cnt = 3; //retry counter
    do {
       if (cnt <= 0) {
          String nanStr = "NaN";
          nanStr.toCharArray(TemperatureStr, 6);
          break;
       }
       DS18B20.requestTemperatures(); 
       temp = DS18B20.getTempCByIndex(0);
       
       dtostrf(temp, 2, 2, TemperatureStr);
       delay(100);
       cnt--;
    } while (temp == 85.0 || temp == (-127.0));
    SecureCounter++;
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
}

void getApi() {
    readTemperature();
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& jsonObj = jsonBuffer.createObject();
    char JSONmessageBuffer[200];
  
    jsonObj["secure_counter"] = SecureCounter;
    jsonObj["symbol"] = "°C";
    jsonObj["temperature"] = TemperatureStr;
    jsonObj["unit"] = "Celsius";
    jsonObj.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    http_rest_server.send(200, "application/json", JSONmessageBuffer);
}

void getIndex() {
    readTemperature();
    String html = web_ui_html;
    html.replace("%T%", String(TemperatureStr));
    html.replace("%SC%", String(SecureCounter));
    http_rest_server.send(200, "text/html", html);
}

void config_rest_server_routing() {
    http_rest_server.on("/", HTTP_GET, getIndex);
    http_rest_server.on("/api", HTTP_GET, getApi);
}

void setup(void) {
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on
    Serial.begin(115200);
    delay(10);

    DS18B20.begin();
    SecureCounter = 0;

    if (init_wifi() == WL_CONNECTED) {
        Serial.print("Connetted to ");
        Serial.print(wifi_ssid);
        Serial.print("--- IP: ");
        Serial.println(WiFi.localIP());
    }
    else {
        Serial.print("Error connecting to: ");
        Serial.println(wifi_ssid);
    }

    config_rest_server_routing();

    http_rest_server.begin();
    Serial.println("HTTP Temp2IoT server started");
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
}

void loop(void) {
    http_rest_server.handleClient();
}
