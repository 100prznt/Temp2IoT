#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <stdio.h>

#include "WifiCredentials.h" //Make sure that you have put your credentials in the 'WifiCredentials.h.template' file and renamed the file to 'WifiCredentials.h'

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
   //TemperatureStr = tempStr;
   SecureCounter++;
}

void getTemperature() {
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


void config_rest_server_routing() {
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            "<!DOCTYPE html><html><head><title>Temp2IoT</title></head><body><h2>Welcome to the Temp2IoT Web Server</h2><p>Refer to the <a href=\"/temp\">REST API</a>, to show the current temperature.</p><p>Find project details on GitHub <a href=\"https://github.com/100prznt/Temp2IoT/\" target=\"_blank\">100prznt/Temp2IoT/</a></p></body></htm>");
    });
    http_rest_server.on("/temp", HTTP_GET, getTemperature);

    http_rest_server.on("/css/uikit.min.css", HTTP_GET, get_uikitcss);
}

void setup(void) {
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
}

void loop(void) {
    http_rest_server.handleClient();
}
