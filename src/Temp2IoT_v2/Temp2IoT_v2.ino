#include <FS.h>                    //this needs to be first, or it all crashes and burns...
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "JsonStreamingParser.h"   // Json Streaming Parser  


#include <ESP8266HTTPClient.h>     // Web Download
#include <ESP8266httpUpdate.h>     // Web Updater

#include <ArduinoJson.h>          // ArduinoJSON                 https://github.com/bblanchon/ArduinoJson

#include <DNSServer.h>            // - Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     // - Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          // WifiManager 


#include <NTPClient.h>
#include <time.h>

#include <Arduino.h>

#include <ESPStringTemplate.h>

#include "index.h"




const long interval = 3000*1000;  // alle 60 Minuten prüfen
unsigned long previousMillis = millis() - 2980*1000; 
unsigned long lastPressed = millis();

WiFiClientSecure client;

ESP8266WebServer server(80);

char time_value[20];


int textsize = 0;

int follower;
int modules = 4;

int mode = 1;
int modetoggle = 0;

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 1;         // current state of the button
int lastButtonState = 1;     // previous state of the button

#define VERSION "0.1"
#define ROTATE 90
#define USE_SERIAL Serial


#define TOGGLE_PIN 0 // D3




//define default values for the stettings
char systemName[20] = "Temp2IoT";
char temp1Name[20] = "Water Temperature";
char temp2Name[20] = "Ambient Temperature";
int sensorCnt = 2;
bool toggleSensors;





//define your default values here, if there are different values in config.json, they are overwritten.
char instagramName[40];
char matrixIntensity[5];
char maxModules[5];
char htmlBuffer[4096];

// =======================================================================

//flag for saving data
bool shouldSaveConfig = true;

//callback notifying us of the need to save config
void saveConfigCallback()
{
	Serial.println("Should save config");
	shouldSaveConfig = true;
}

void handleRoot()
{

	ESPStringTemplate webpage(htmlBuffer, sizeof(htmlBuffer));

	TokenStringPair pair[1];
	pair[0].setPair("%NAME%", systemName);

	/*webpage.add_P(_PAGE_HEAD);
	webpage.add_P(_PAGE_START);

	webpage.add_P(_PAGE_ACTIONS);

	webpage.add_P(_PAGE_CONFIG_NAME, pair,1);

	switch (mode)
	{
		case 1:
			webpage.add_P(_PAGE_CONFIG_MODE1);
			break;

		case 2:
			webpage.add_P(_PAGE_CONFIG_MODE2);
			break;

		case 3:
			webpage.add_P(_PAGE_CONFIG_MODE3);
			break;

		default:
			webpage.add_P(_PAGE_CONFIG_MODE1);
			break;
	} */

	TokenStringPair intensityPair[1]; 


	/* intensityPair[0].setPair("%NAME1SENSOR%",temp1Name );
	webpage.add_P(_PAGE_CONFIG_INTENSITY, intensityPair, 1);
	webpage.add_P(_PAGE_FOOTER); */

	server.send(200, "text/html", htmlBuffer);
}

void redirectBack()
{
	server.sendHeader("Location", String("/"), true);
	server.send ( 302, "text/plain", "");
}

void getConfig()
{
  	// mode
	String modeString = server.arg("mode");
	mode = modeString.toInt();

	saveConfig();

	redirectBack();
}


void getReset()
{
	redirectBack();
	restartX();
}

void getUpdate()
{
	redirectBack();
	updateFirmware();
}

void getFormat()
{
	redirectBack();
	infoReset();
}



void setup()
{
  	// Serial debugging
	Serial.begin(115200);

  	// Required for instagram api
	//client.setInsecure();


  	// Set Reset-Pin to Input Mode
	pinMode(TOGGLE_PIN, INPUT);


	if (SPIFFS.begin())
	{
		if (SPIFFS.exists("/config.json")) {
      	//file exists, reading and loading

			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile)
			{
				Serial.println("opened config file");
				size_t size = configFile.size();
        		// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonDocument json(1024);
				deserializeJson(json, buf.get());
				serializeJson(json,Serial);

				//strcpy(temp1Name, json["temp1Name"]);
				//strcpy(temp2Name, json["temp2Name"]);

				JsonVariant jsonSystemName = json["systemName"];
				if (!jsonSystemName.isNull())
				{
					strcpy(systemName, json["systemName"]);
				}

				JsonVariant jsonTemp1Name = json["temp1Name"];
				if (!jsonTemp1Name.isNull())
				{
					strcpy(temp1Name, json["temp1Name"]);
				} 
				JsonVariant jsonTemp2Name = json["temp2Name"];
				if (!jsonTemp2Name.isNull())
				{
					strcpy(temp2Name, json["temp2Name"]);
				} 

				JsonVariant jsonSensorCnt = json["sensorCnt"];
				if (!jsonSensorCnt.isNull())
				{ 
					sensorCnt = jsonSensorCnt.as<int>();
				}

				JsonVariant jsonToggleTemps = json["toggleSensors"];
				if (!jsonToggleTemps.isNull())
				{ 
					toggleSensors = jsonToggleTemps.as<bool>();
				}
			}
		}
	}
	else
	{
		Serial.println("failed to mount FS");
	}
  	//end read

	WiFiManager wifiManager;
	//wifiManager.setCustomHeadElement("<style>html{filter: invert(100%); -webkit-filter: invert(100%);}</style>");
	wifiManager.setCustomHeadElement("<style>button,input[type='button'],input[type='submit']{background-color:#ff2e64;color:#fff}</style>");

	WiFiManagerParameter custom_header("<h3>Temp2IoT Settings</h3>");

  	//Definitins of custom parameters
	//Prepare
	char char_sensorCnt[2];
	itoa(sensorCnt, char_sensorCnt, 10);

	char char_toggleSensors[6];
	if (toggleSensors)
	{
		strncpy(char_toggleSensors, "true", 6);
	}
	else
	{
		strncpy(char_toggleSensors, "false", 6);
	}

	//Define
	WiFiManagerParameter custom_systemName("systemName", "Systemname", systemName, 20);
	WiFiManagerParameter custom_temp1Name("temp1Name", "Bezeichnung Sensor 1", temp1Name, 20);
	WiFiManagerParameter custom_temp2Name("temp2Name", "Bezeichnung Sensor 2", temp2Name, 20);
	WiFiManagerParameter custom_sensorCnt("sensorCnt", "Sensor Anzahl (1/2)", char_sensorCnt, 2);
	WiFiManagerParameter custom_toggleSensors("toggleSensors", "Sensoren tauschen (true/false)", char_toggleSensors, 6);

  	//Add parameters to wifiManager
	wifiManager.addParameter(&custom_header);
	wifiManager.addParameter(&custom_systemName);
	wifiManager.addParameter(&custom_temp1Name);
	wifiManager.addParameter(&custom_temp2Name);
	wifiManager.addParameter(&custom_sensorCnt);
	wifiManager.addParameter(&custom_toggleSensors);
 
	delay(500);


	configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  	setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);  // https://github.com/nayarsystems/posix_tz_db 

	//set config save notify callback
	wifiManager.setSaveConfigCallback(saveConfigCallback);


	wifiManager.autoConnect("Temp2IoT");


	server.on("/", handleRoot);
	server.on("/format", getFormat);
	server.on("/update", getUpdate);
	server.on("/reset", getReset);
	server.on("/config", getConfig);

	server.begin();

	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	//read updated parameters
	strcpy(systemName, custom_systemName.getValue());
	strcpy(temp1Name, custom_temp1Name.getValue());
	strcpy(temp2Name, custom_temp2Name.getValue());

	sensorCnt = String(custom_sensorCnt.getValue()).toInt();
	if (String(custom_toggleSensors.getValue()) == String("true") || String(custom_toggleSensors.getValue()) == String("TRUE"))
	{
		toggleSensors = true;
	}
	else
	{
		toggleSensors = false;
	}

	//save the custom parameters to FS
	if (shouldSaveConfig)
	{
		saveConfig();
	}

}

void saveConfig() {
	DynamicJsonDocument json(1024);

	json["systemName"] = systemName;
	json["temp1Name"] = temp1Name;
	json["temp2Name"] = temp2Name;
	json["sensorCnt"] = sensorCnt;
	json["toggleSensors"] = toggleSensors;

	File configFile = SPIFFS.open("/config.json", "w");

	if (!configFile)
	{
		Serial.println("failed to open config file for writing");
	}

	serializeJson(json, Serial);
	serializeJson(json, configFile);
}

void infoWlan() {

	if (WiFi.status() ==  WL_CONNECTED )
	{
	    // WLAN OK
	} 
	else
	{
    	// WLAN Error
	}
}

void infoIP() {
	String localIP = WiFi.localIP().toString();
}

void infoVersion() {
	char versionString[8];
	sprintf(versionString,"Ver. %s", VERSION);
}


void infoReset() {
	Serial.println("Format System");

    // Reset Wifi-Setting
	WiFiManager wifiManager;
	wifiManager.resetSettings();

    // Format Flash
	SPIFFS.format();

    // Restart
	ESP.reset();

}

void restartX() {
	ESP.reset();
}


void update_started() {
	USE_SERIAL.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
	USE_SERIAL.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
	char progressString[10];
	float percent = ((float)cur   / (float)total )  * 100;
	sprintf(progressString, " %s",  String(percent).c_str()  );

	USE_SERIAL.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
	char errorString[8];
	sprintf(errorString, "Err %d", err);

	USE_SERIAL.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void updateFirmware() {

	ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
	ESPhttpUpdate.onStart(update_started);
	ESPhttpUpdate.onEnd(update_finished);
	ESPhttpUpdate.onProgress(update_progress);
	ESPhttpUpdate.onError(update_error);

	t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://counter.buuild.it/static/themes/counter/followercounter.ino.bin");


	switch (ret) {
		case HTTP_UPDATE_FAILED:
		USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
		break;

		case HTTP_UPDATE_NO_UPDATES:
		USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
		break;

		case HTTP_UPDATE_OK:
		USE_SERIAL.println("HTTP_UPDATE_OK");
		break;
	}

}

//  
void loop() {

	server.handleClient();

	buttonState = digitalRead(TOGGLE_PIN);
	unsigned long currentMillis = millis();


	if (buttonState != lastButtonState && currentMillis > lastPressed + 50 ) {

    // if the state has changed, increment the counter
		if (buttonState == LOW) {
      // if the current state is HIGH then the button went from off to on:
			buttonPushCounter++;
			lastPressed = currentMillis;

			Serial.println("push");

			Serial.println(buttonPushCounter);

		} else {
      // if the current state is LOW then the button went from on to off:
			Serial.println("off");
		}
	}

  // Warte 1sec nach dem letzten Tastendruck 
	if (currentMillis > lastPressed + 1000) {

		if (buttonPushCounter > 0 ) {

			Serial.print("number of button pushes: ");
			Serial.println(buttonPushCounter);

			switch (buttonPushCounter) {

				case 1: 
                  // Einmal gedrückt / FollowerCounter-Modus
					mode = 1;
				break;

				case 2:
                  // Zweimal gedrückt / Uhrzeit-Modus
					mode = 2;
				break;

				case 3:
                  // Dreimal gedrückt / Wechselmodus
					mode = 3;
				break;

				case 4:
					infoWlan();
				break;

				case 5:
					infoIP();
				break;

				case 6:
					infoVersion();
				break;

				case 7:
					updateFirmware();
				break;

				case 8:
					restartX();
				break;

				case 10:
					infoReset();
				break;

				default:
					//Too many!
				break;
			}



		}

		buttonPushCounter = 0;
	}

  	// save the current state as the last state, for next time through the loop
	lastButtonState = buttonState;   

}

void printTime() {

	time_t now = time(nullptr);
	String time = String(ctime(&now));
	time.trim();
	time.substring(11,16).toCharArray(time_value, 10); 

	//printString(6,8, time_value,2);
}

