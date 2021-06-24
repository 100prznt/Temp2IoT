/*
This project is based on the architecture and code of @jegade his followercounter.
https://github.com/jegade/followercounter
*/


#include <FS.h>                    // this needs to be first, or it all crashes and burns...

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

#include <DallasTemperature.h>
#include <OneWire.h>
#include <stdio.h>


#include "index.h"
#include "config.h"


#define VERSION "2.0b"
#define ROTATE 90
#define USE_SERIAL Serial
#define ONE_WIRE_BUS D3


const long interval = 3000 * 1000;  // check every 60 minutes
unsigned long previousMillis = millis() - 2980 * 1000;

WiFiClientSecure client;

ESP8266WebServer server(80);


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char Temperature1Str[6] = "NaN";
char Temperature2Str[6] = "NaN";
unsigned int SecureCounter;

char time_value[20];

//define your default values here, if there are different values in config.json, they are overwritten.
char systemName[20] = "Temp2IoT";
char temp1Name[20] = "Water Temperature";
char temp2Name[20] = "Ambient Temperature";
int sensorCnt = 2;
bool toggleSensors;

//init up
char measTime[26] = "Thu Jan  1 00:00:00 1970";
char htmlBuffer[8000];

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

	TokenStringPair pair_SystemName[1];
	pair_SystemName[0].setPair("%NAME%", systemName);

	TokenStringPair pair_Data[4];

	if (toggleSensors)
	{
		pair_Data[0].setPair("%T1%", Temperature2Str);
		pair_Data[1].setPair("%N1%", temp2Name);
		pair_Data[2].setPair("%T2%", Temperature1Str);
		pair_Data[3].setPair("%N2%", temp1Name);
	}
	else
	{
		pair_Data[0].setPair("%T1%", Temperature1Str);
		pair_Data[1].setPair("%N1%", temp1Name);
		pair_Data[2].setPair("%T2%", Temperature2Str);
		pair_Data[3].setPair("%N2%", temp2Name);
	}

	TokenStringPair pair_SecureCounter[1];
	char secureCounterStr[20];
	itoa(SecureCounter, secureCounterStr, 10);
	pair_SecureCounter[0].setPair("%SC%", secureCounterStr);

	webpage.add_P(_PAGE_WEBUI_HEAD);

	webpage.add_P(_PAGE_WEBUI_CARDHEAD, pair_SystemName, 1);
	
	if (sensorCnt == 2)
	{
		webpage.add_P(_PAGE_WEBUI_CARDBODY, pair_Data, 4);
	}
	else
	{
		webpage.add_P(_PAGE_WEBUI_CARDBODYSINGLE, pair_Data, 2);
	}

	webpage.add_P(_PAGE_WEBUI_CARDFOOTER, pair_SecureCounter, 1);
	
	if (sensorCnt == 2)
	{
		webpage.add_P(_PAGE_WEBUI_FOOTER_SCRIPT2);
	}
	else
	{
		//webpage.add_P(_PAGE_WEBUI_FOOTER_SCRIPT1);
	}	
	server.send(200, "text/html", htmlBuffer);
}

void handleApi()
{
	char JSONmessageBuffer[1024];
	DynamicJsonDocument doc(1024);

	doc["systemname"] = systemName;
	doc["secure_counter"] = SecureCounter;
	doc["firmware"] = VERSION;
	doc["sensors"][0] = getData(1);
	if (sensorCnt == 2)
	{
		doc["sensors"][1] = getData(2);
	}

	serializeJsonPretty(doc, JSONmessageBuffer);

	server.send(200, "application/json", JSONmessageBuffer);
}

DynamicJsonDocument getData(int idx)
{
	DynamicJsonDocument doc(1024);

	switch (idx)
	{
		case 1:
		{
			doc["name"] = temp1Name;
			doc["value"] = Temperature1Str;
		}
		break;
		case 2:
		{
			doc["name"] = temp2Name;
			doc["value"] = Temperature2Str;
		}
		break;
		default:
		break;
	}

	doc["unit"] = "Celsius";
	doc["time"] = measTime;

	return doc;
}

void handleConfig()
{
	ESPStringTemplate webpage(htmlBuffer, sizeof(htmlBuffer));

	TokenStringPair pair[3];
	pair[0].setPair("%SYSNAME%", systemName);
	pair[1].setPair("%SENSOR1NAME%", temp1Name);
	pair[2].setPair("%SENSOR2NAME%", temp2Name);

	webpage.add_P(_PAGE_HEAD);
	webpage.add_P(_PAGE_START);

	webpage.add_P(_PAGE_ACTIONS);

	webpage.add_P(_PAGE_CONFIG_SYSNAME, pair, 1);
	webpage.add_P(_PAGE_CONFIG_SENSOR1NAME, pair, 2);
	webpage.add_P(_PAGE_CONFIG_SENSOR2NAME, pair, 3);

	switch (sensorCnt)
	{
		case 1:
		webpage.add_P(_PAGE_CONFIG_SENSORCNT1);
		break;

		case 2:
		webpage.add_P(_PAGE_CONFIG_SENSORCNT2);
		break;

		default:
		webpage.add_P(_PAGE_CONFIG_SENSORCNT1);
		break;
	}

	if (toggleSensors)
	{
		webpage.add_P(_PAGE_CONFIG_SENSORTOOGLETRUE);
	}
	else
	{
		webpage.add_P(_PAGE_CONFIG_SENSORTOOGLEFALSE);		
	}

	TokenStringPair intensityPair[1]; 


	intensityPair[0].setPair("%NAME1SENSOR%",temp1Name );
	webpage.add_P(_PAGE_FOOTER);

	server.send(200, "text/html", htmlBuffer);
}

void redirectBack()
{
	server.sendHeader("Location", String("/"), true);
	server.send ( 302, "text/plain", "");
}

void getConfig()
{
  	String systemNameString = server.arg("systemName");
  	systemNameString.toCharArray(systemName,20);
  	String temp1NameString = server.arg("temp1Name");
  	temp1NameString.toCharArray(temp1Name,20);
  	String temp2NameString = server.arg("temp2Name");
  	temp2NameString.toCharArray(temp2Name,20);


	String toggleSensorsString = server.arg("toggleSensors");
	if (toggleSensorsString == "on")
	{
		toggleSensors = true;
	}
	else
	{
		toggleSensors = false;
	}


  	// sensorCnt
	String sensorCntString = server.arg("sensorCnt");
	sensorCnt = sensorCntString.toInt();

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

void readTemperature() {
	Serial.println("Start new reading on 1-Wire bus...");
    digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on
    float temp1;
    float temp2;

    time_t now = time(nullptr);
	String time = String(ctime(&now));
    time.toCharArray(measTime, 25);

    int cnt = 3; //retry counter
    do {
    	if (cnt <= 0) {
    		String nanStr = "NaN";
    		nanStr.toCharArray(Temperature1Str, 6);
    		nanStr.toCharArray(Temperature2Str, 6);
    		break;
    	}
    	DS18B20.requestTemperatures(); 
    	temp1 = DS18B20.getTempCByIndex(0);
    	dtostrf(temp1, 2, 2, Temperature1Str);
    	if (sensorCnt > 1)
    	{
    		temp2 = DS18B20.getTempCByIndex(1);
    		if (temp2 != 127.94)
    		{
    			dtostrf(temp2, 2, 2, Temperature2Str);
    		}
    	}

    	delay(100);
    	cnt--;
    } while (temp1 == 85.0 || temp1 == (-127.0) || temp1 == 127.94);
    SecureCounter++;
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
}

void setup()
{
  	// Serial debugging
	Serial.begin(115200);

  	// Required for instagram api
	//client.setInsecure();

	pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on


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

  	//Add parameters to wifiManager
    wifiManager.addParameter(&custom_header);
    wifiManager.addParameter(&custom_systemName);
    wifiManager.addParameter(&custom_temp1Name);
    wifiManager.addParameter(&custom_temp2Name);

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
  	server.on("/setup", handleConfig);
  	server.on("/api", handleApi);

  	server.begin();

  	Serial.print("IP address: ");
  	Serial.println(WiFi.localIP());

	//read updated parameters
  	strcpy(systemName, custom_systemName.getValue());
  	strcpy(temp1Name, custom_temp1Name.getValue());
  	strcpy(temp2Name, custom_temp2Name.getValue());

	/* sensorCnt = String(custom_sensorCnt.getValue()).toInt();
	if (String(custom_toggleSensors.getValue()) == String("true") || String(custom_toggleSensors.getValue()) == String("TRUE"))
	{
		toggleSensors = true;
	}
	else
	{
		toggleSensors = false;
	} */

	//save the custom parameters to FS
  	if (shouldSaveConfig)
  	{
  		saveConfig();
  	}


  	DS18B20.begin();
  	SecureCounter = 0;

  	Serial.println("HTTP Temp2IoT server started");
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
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


void update_started()
{
	USE_SERIAL.println("CALLBACK:  HTTP update process started");
}

void update_finished()
{
	USE_SERIAL.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total)
{
	char progressString[10];
	float percent = ((float)cur   / (float)total )  * 100;
	sprintf(progressString, " %s",  String(percent).c_str()  );

	USE_SERIAL.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err)
{
	char errorString[8];
	sprintf(errorString, "Err %d", err);

	USE_SERIAL.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void updateFirmware()
{

	/*ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
	ESPhttpUpdate.onStart(update_started);
	ESPhttpUpdate.onEnd(update_finished);
	ESPhttpUpdate.onProgress(update_progress);
	ESPhttpUpdate.onError(update_error);

	t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://<URL>Temp2IoT_v2.ino.bin");


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
	}*/
}

//  
void loop()
{
	server.handleClient();

	unsigned long currentMillis = millis();

	if (currentMillis % 5000 == 0 )
	{ 
		readTemperature();
	}
}

void getTime()
{
	time_t now = time(nullptr);
	String time = String(ctime(&now));
	time.trim();
	time.substring(11,16).toCharArray(time_value, 10); 

	char timeStr[20];
	time.toCharArray(timeStr, 16);

	//printString(6,8, time_value,2);
}

