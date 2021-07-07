/**
 * Temp2IoT
 * Very basic IoT thermometer with REST API and web UI,
 * based on ESP8266 hardware (WeMos D1 mini) 
 *
 * C++11
 *
 * Copyright (C) 2021  Elias Ruemmler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @package    Temp2IoT
 * @author     Elias Ruemmler <e.ruemmler@rc-art.de>
 * @copyright  2021 RC-Art Solutions
 * @version    2.1
 * @link       https://github.com/100prznt/Temp2IoT
 * @since      2020-06-17
 *
 *
 * The architecture of this project is based on the code of @jegade his followercounter.
 * https://github.com/jegade/followercounter
 */


#include <FS.h>						// this needs to be first, or it all crashes and burns...

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "JsonStreamingParser.h"	// Json Streaming Parser 		https://github.com/squix78/json-streaming-parser

#include <ESP8266HTTPClient.h>		// Web Download
#include <ESP8266httpUpdate.h>		// Web Updater

#include <ArduinoJson.h>			// ArduinoJson 					https://github.com/bblanchon/ArduinoJson

#include <DNSServer.h>				// - Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>		// - Local WebServer used to serve the configuration portal
#include <WiFiManager.h>			// WifiManager   				https://github.com/tzapu/WiFiManager


#include <NTPClient.h>				// Connect to a NTP Server		https://github.com/arduino-libraries/NTPClient
#include <time.h>

#include <Arduino.h>

#include <ESPStringTemplate.h>		// ESPStringTemplate			https://github.com/DaleGia/ESPStringTemplate

#include <DallasTemperature.h>
#include <OneWire.h>
#include <stdio.h>

#include <cppQueue.h>				// Queue handling library		https://github.com/SMFSW/Queue


#include "index.h"
#include "config.h"
#include "favicon.h"


#define VERSION "2.2.01-b"
#define ROTATE 90
#define USE_SERIAL Serial
#define ONE_WIRE_BUS D3


unsigned long previousMillis = millis() - 2980 * 1000;

WiFiClientSecure client;
ESP8266WebServer server(80);


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char Temperature1Str[6] = "NaN";
char Temperature2Str[6] = "NaN";
float MeasValue1;
float MeasValue2;

unsigned int SecureCounter;

char time_value[20];

//define your default values here, if there are different values in config.json, they are overwritten.
char systemName[20] = "Temp2IoT";
char temp1Name[20] = "Water Temperature";
char temp2Name[20] = "Ambient Temperature";
int colorScheme = 1;
int sensorCnt = 2;
bool toggleSensors;

//init up
char measTime[26] = "Thu Jan  1 00:00:00 1970";
char primaryColor[8] = "#ff2e64"; //"#1e87f0"
char htmlBuffer[8000];



//data storage for tend analysis
#define	IMPLEMENTATION FIFO
struct strRec {
	time_t	timestamp;
	float	measvalue;
} Rec;

cppQueue queue_MeasValues(sizeof(Rec), 1440, IMPLEMENTATION, true);	//Init queue; 1440 min = 24 h
float MeasValueMean;

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

	TokenStringPair pair_Style[1];
	pair_Style[0].setPair("%COLORPRIM%", primaryColor);

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

	webpage.add_P(_PAGE_WEBUI_HEAD, pair_Style, 1);

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

void handleFaviconSvg()
{
	server.send(200, "image/svg+xml", _FAVICON_SVG_WHITE);
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

	doc["test_mean"] = getData_MeanValue(1);

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
			doc["value"] = MeasValue1;
			if (!toggleSensors)
			{
				doc["mean"] = MeasValueMean;
			}
			else
			{
				doc["mean"] = "NaN";
			}
		}
		break;
		case 2:
		{
			doc["name"] = temp2Name;
			doc["value"] = MeasValue2;
			if (toggleSensors)
			{
				doc["mean"] = MeasValueMean;
			}
			else
			{
				doc["mean"] = "NaN";
			}
		}
		break;
		default:
		break;
	}

	doc["unit"] = "Celsius";
	doc["time"] = measTime;

	return doc;
}

DynamicJsonDocument getData_MeanValue(int period)
{
	int periodSec = period * 3600; //to [sec]
	
	periodSec = 100;

	//Generate period start time
	time_t now = time(nullptr);
	unsigned long startTime = now - periodSec;




	//Meanvalue calculation
	float sum = 0;
	bool inTimeRange = false;
	int cnt_MeasValues_InUse = 0;
	int cnt_MeasValues = queue_MeasValues.getCount();
	for (int i = 0; i < cnt_MeasValues; i++)
	{
		strRec cRec;
		queue_MeasValues.peekIdx(&cRec, i);

		if (inTimeRange || cRec.timestamp > startTime)
		{
			inTimeRange = true;
		
			cnt_MeasValues_InUse++;
			sum = sum + cRec.measvalue;
		}
	}

	DynamicJsonDocument doc(1024);

	doc["count"] = cnt_MeasValues_InUse;
	doc["value"] = sum / cnt_MeasValues_InUse;

	doc["period"] = periodSec;



	return doc;

}

void handleConfig()
{
	ESPStringTemplate webpage(htmlBuffer, sizeof(htmlBuffer));

	TokenStringPair pair_Style[1];
	pair_Style[0].setPair("%COLORPRIM%", primaryColor);

	TokenStringPair pair[3];
	pair[0].setPair("%SYSNAME%", systemName);
	pair[1].setPair("%SENSOR1NAME%", temp1Name);
	pair[2].setPair("%SENSOR2NAME%", temp2Name);

	webpage.add_P(_PAGE_HEAD, pair_Style, 1);
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


	switch (colorScheme)
	{
		case (2):
			webpage.add_P(_PAGE_CONFIG_COLORSCHEME_2);	
		break;
		case (3):
			webpage.add_P(_PAGE_CONFIG_COLORSCHEME_3);	
		break;
		case (4):
			webpage.add_P(_PAGE_CONFIG_COLORSCHEME_4);	
		break;
		case (5):
			webpage.add_P(_PAGE_CONFIG_COLORSCHEME_5);	
		break;
		case (6):
			webpage.add_P(_PAGE_CONFIG_COLORSCHEME_6);	
		break;
		default: //1 -> 100prznt
			webpage.add_P(_PAGE_CONFIG_COLORSCHEME_1);	
		break;
	}


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
	bool buffer = false;
	if (toggleSensorsString == "on")
	{
		buffer = true;
	}

	if (buffer != toggleSensors)
	{
		queue_MeasValues.clean();
		toggleSensors = buffer;
	}


  	// sensorCnt
	String sensorCntString = server.arg("sensorCnt");
	sensorCnt = sensorCntString.toInt();


	 // colorScheme
	String colorSchemeString = server.arg("colorScheme");
	colorScheme = colorSchemeString.toInt();

	switch (colorScheme)
	{
		case 2: //Clasic
			String("#1e87f0").toCharArray(primaryColor, 8);		
		break;
		case 3: //Total
			String("#30a4a1").toCharArray(primaryColor, 8);		
		break;
		case 4: //Power
			String("#325c84").toCharArray(primaryColor, 8);	
		break;
		case 5: //Sun
			String("#f08a00").toCharArray(primaryColor, 8);	
		break;
		case 6: //100prznt secondary
			String("#060d2a").toCharArray(primaryColor, 8);	
		break;
		default: //1 -> 100prznt
			String("#ff2e64").toCharArray(primaryColor, 8);	
		break;
	}

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
    //float temp1;
    //float temp2;

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
    	MeasValue1 = DS18B20.getTempCByIndex(0);
    	dtostrf(MeasValue1, 2, 2, Temperature1Str);
    	if (sensorCnt > 1)
    	{
    		MeasValue2 = DS18B20.getTempCByIndex(1);
    		if (MeasValue2 != 127.94)
    		{
    			dtostrf(MeasValue2, 2, 2, Temperature2Str);
    		}
    	}

    	delay(100);
    	cnt--;
    } while (MeasValue1 == 85.0 || MeasValue1 == (-127.0) || MeasValue1 == 127.94);


    strRec rec = { now, MeasValue1 };
    if (toggleSensors)
    {
    	rec = {now, MeasValue2 };
    }
    queue_MeasValues.push(&rec);


	//Meanvalue calculation
	float sum = 0;
	int cnt_MeasValues = queue_MeasValues.getCount();
	for (int i = 0; i < cnt_MeasValues; i++)
	{
		strRec cRec;
		queue_MeasValues.peekIdx(&cRec, i);
		sum = sum + cRec.measvalue;
	}
	MeasValueMean = sum / cnt_MeasValues;


    SecureCounter++;
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
}

void setup()
{
  	// Serial debugging
	Serial.begin(115200);

  	// Required for instagram api
	client.setInsecure();

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

    			JsonVariant jsonPrimaryColor = json["primaryColor"];
    			if (!jsonPrimaryColor.isNull())
    			{
    				strcpy(primaryColor, json["primaryColor"]);
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


    //configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    configTime(0, 0, "pool.ntp.org", "192.168.0.41");
  	setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);  // https://github.com/nayarsystems/posix_tz_db 

	//set config save notify callback
  	wifiManager.setSaveConfigCallback(saveConfigCallback);


  	wifiManager.autoConnect("Temp2IoT");


  	server.on("/", handleRoot);
  	server.on("/favicon.svg", handleFaviconSvg);
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

void saveConfig()
{
	DynamicJsonDocument json(1024);

	json["systemName"] = systemName;
	json["temp1Name"] = temp1Name;
	json["temp2Name"] = temp2Name;
	json["sensorCnt"] = sensorCnt;
	json["toggleSensors"] = toggleSensors;
	json["primaryColor"] = primaryColor;

	File configFile = SPIFFS.open("/config.json", "w");

	if (!configFile)
	{
		Serial.println("failed to open config file for writing");
	}

	serializeJson(json, Serial);
	serializeJson(json, configFile);
}

void infoReset()
{
	Serial.println("Format System");

    // Reset Wifi-Setting
	WiFiManager wifiManager;
	wifiManager.resetSettings();

    // Format Flash
	SPIFFS.format();

    // Restart
	ESP.reset();
}

void restartX()
{
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

	ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
	ESPhttpUpdate.onStart(update_started);
	ESPhttpUpdate.onEnd(update_finished);
	ESPhttpUpdate.onProgress(update_progress);
	ESPhttpUpdate.onError(update_error);

	t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://pool.100prznt.de/temp2iot/bin/release/latest/Temp2IoT.ino.d1_mini.bin");


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
 
void loop()
{
	server.handleClient();

	unsigned long currentMillis = millis();

	if (currentMillis % 5000 == 0 )
	{ 
		readTemperature();

		/*USE_SERIAL.print("Queue count: ");
		USE_SERIAL.println(queue_MeasValues.getCount());

		strRec cRec;
		queue_MeasValues.peekPrevious(&cRec);

		USE_SERIAL.print("Last meastime: ");
		String timeStamp = String(ctime(&cRec.timestamp));
		timeStamp.trim();
		USE_SERIAL.println(timeStamp);
		USE_SERIAL.print("Last measvalue: ");
		USE_SERIAL.println(cRec.measvalue);

		//Meanvalue calculation
		float sum = 0;
		int cnt = queue_MeasValues.getCount();
		for (int i = 0; i < cnt; i++)
		{
			strRec cRec;
			queue_MeasValues.peekIdx(&cRec, i);

			sum = sum + cRec.measvalue;
		}
		float meanValue = sum / cnt;

		USE_SERIAL.print("Meanvalue: ");
		USE_SERIAL.println(meanValue);*/

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

