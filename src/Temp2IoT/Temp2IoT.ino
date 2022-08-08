/**
 * Temp2IoT
 * Very basic IoT thermometer with REST API and web UI,
 * based on ESP8266 hardware (WeMos D1 mini) 
 *
 * a 100prznt.de project by E. Ruemmler
 *
 *
 * C++11
 *
 * Copyright (C) 2022  Elias Ruemmler
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
 * @package		Temp2IoT
 * @author		Elias Ruemmler <e.ruemmler@rc-art.de>
 * @copyright	2022 RC-Art Solutions
 * @version		2.3
 * @link		https://github.com/100prznt/Temp2IoT
 * @since		2020-06-17
 *
 *
 * The architecture of this project is based on the code of @jegade his followercounter.
 * https://github.com/jegade/followercounter
 */


#include <FS.h>						// this needs to be first, or it all crashes and burns...

#include "Arduino.h"
#include <ESP8266WiFi.h>
//#include "JsonStreamingParser.h"	// Json Streaming Parser 		https://github.com/squix78/json-streaming-parser

#include <ESP8266HTTPClient.h>		// Web Download
#include <ESP8266httpUpdate.h>		// Web Updater

#include <ArduinoJson.h>			// ArduinoJson 					https://github.com/bblanchon/ArduinoJson

#include <DNSServer.h>				// Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>		// Local WebServer used to serve the configuration portal
#include <WiFiManager.h>			// WifiManager 					https://github.com/tzapu/WiFiManager


#include <NTPClient.h>				// Connect to a NTP Server		https://github.com/arduino-libraries/NTPClient
#include <time.h>

#include <Arduino.h>

#include <ESPStringTemplate.h>		// ESPStringTemplate			https://github.com/DaleGia/ESPStringTemplate

#include <DallasTemperature.h>		// Maxim Temperature ICs		https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <OneWire.h>				// 1-Wire library				https://www.pjrc.com/teensy/td_libs_OneWire.html
#include <stdio.h>


#include "index.h"
#include "config.h"
#include "favicon.h"


#define VERSION "2.3.05-b"
#define ROTATE 90
#define USE_SERIAL Serial
#define ONE_WIRE_BUS D3


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
char ntpServer[20] = "time.nist.gov";
//parameters for calibration polynom (2. order)
//squareValue * x^2 + gainValue * x + offsetValue
float offsetValue1 = 0;
float offsetValue2 = 0;
float gainValue1 = 1;
float gainValue2 = 1;
float squareValue1 = 0;
float squareValue2 = 0;

//init up
char measTime[26] = "Thu Jan  1 00:00:00 1970";
char primaryColor[8] = "#ff2e64"; //"#1e87f0"
char htmlBuffer[8000];


//data storage for tend analysis
#define MEMORY_DEPTH 1440 //count of measvalues which are held in the ring buffer
#define MEMORY_VALUES_PER_HOUR 60 //value must be divisor of 720

float MeasValues[MEMORY_DEPTH];
int MeasValues_Index = 0;
bool MeasValues_IsFull = false;

float MeasValueMean;
int cnt_Readings = 720 / MEMORY_VALUES_PER_HOUR;
float MeasValueInterval = 3600.0 / MEMORY_VALUES_PER_HOUR;

//=======================================================================


//flag for saving data
bool shouldSaveConfig = true;

//callback notifying us of the need to save config
void saveConfigCallback()
{
	USE_SERIAL.println("Should save config");
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
		webpage.add_P(_PAGE_WEBUI_CARDBODY, pair_Data, 4);
	else
		webpage.add_P(_PAGE_WEBUI_CARDBODYSINGLE, pair_Data, 2);

	webpage.add_P(_PAGE_WEBUI_CARDFOOTER, pair_SecureCounter, 1);
	
	if (sensorCnt == 2)
	{
		if (toggleSensors)
			webpage.add_P(_PAGE_WEBUI_FOOTER_SCRIPT2_TOGGLED);
		else
			webpage.add_P(_PAGE_WEBUI_FOOTER_SCRIPT2);
	}
	else
		webpage.add_P(_PAGE_WEBUI_FOOTER_SCRIPT1);

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
		doc["sensors"][1] = getData(2);

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
			if (MeasValue1 == 999.9)
				doc["value"] = "NaN";
			else
				doc["value"] = MeasValue1;
			if (!toggleSensors)
			{
				doc["mean-1"] = getData_MeanValue(1);
				doc["mean-24"] = getData_MeanValue(24);
			}
		}
		break;
		case 2:
		{
			doc["name"] = temp2Name;
			if (MeasValue2 == 999.9)
				doc["value"] = "NaN";
			else
				doc["value"] = MeasValue2;
			if (toggleSensors)
			{
				doc["mean-1"] = getData_MeanValue(1);
				doc["mean-24"] = getData_MeanValue(24);
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
	DynamicJsonDocument doc(1024);
	int cnt_MeasValues_InUse = 0;
	
	int periodSec = period * 3600; //to [sec]
	//periodSec = 300; //5 min
	
	int offset = periodSec / MeasValueInterval; //round?
	
	USE_SERIAL.print("offset = ");
	USE_SERIAL.println(offset);

	if (offset > MEMORY_DEPTH || (!MeasValues_IsFull && MeasValues_Index < offset))
	{
		doc["value"] = "NaN";
	}
	else
	{
		float sum = 0;

		if (MeasValues_Index >= offset) //no overflow
		{
			//Meanvalue calculation
			for (int i = MeasValues_Index - offset; i < MeasValues_Index; i++)
			{
				sum += MeasValues[i];
				cnt_MeasValues_InUse++;
			}
		}
		else //with overflow
		{
			for (int i = 0; i < MeasValues_Index; i++)
			{
				sum += MeasValues[i];
				cnt_MeasValues_InUse++;
			}

			int firstIdx = MEMORY_DEPTH - (offset - MeasValues_Index);
			for (int i = firstIdx; i < MEMORY_DEPTH; i++)
			{
				sum += MeasValues[i];
				cnt_MeasValues_InUse++;
			}
		}

		doc["value"] = sum / cnt_MeasValues_InUse;
	}

	doc["count"] = cnt_MeasValues_InUse;
	doc["period"] = periodSec;

	return doc;
}

void handleConfig()
{
	ESPStringTemplate webpage(htmlBuffer, sizeof(htmlBuffer));

	TokenStringPair pair_Style[1];
	pair_Style[0].setPair("%COLORPRIM%", primaryColor);

	TokenStringPair pair[5];
	pair[0].setPair("%SYSNAME%", systemName);
	pair[1].setPair("%VERSION%", VERSION);
	pair[2].setPair("%SENSOR1NAME%", temp1Name);
	pair[3].setPair("%SENSOR2NAME%", temp2Name);
	pair[4].setPair("%NTPSERVER%", ntpServer);

	webpage.add_P(_PAGE_HEAD, pair_Style, 1);
	webpage.add_P(_PAGE_START), pair, 2;

	webpage.add_P(_PAGE_ACTIONS);

	webpage.add_P(_PAGE_CONFIG_SYSNAME, pair, 1);
	webpage.add_P(_PAGE_CONFIG_SENSOR1NAME, pair, 3);
	webpage.add_P(_PAGE_CONFIG_SENSOR2NAME, pair, 4);
	

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
		webpage.add_P(_PAGE_CONFIG_SENSORTOOGLETRUE);
	else
		webpage.add_P(_PAGE_CONFIG_SENSORTOOGLEFALSE);

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

	webpage.add_P(_PAGE_CONFIG_NTP, pair, 5);

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
		buffer = true;

	if (buffer != toggleSensors)
	{
		USE_SERIAL.println("# Buffer.Clear");
		MeasValues_IsFull = false;
		MeasValues_Index = 0;
		toggleSensors = buffer;
	}

	//sensorCnt
	String sensorCntString = server.arg("sensorCnt");
	sensorCnt = sensorCntString.toInt();

	//colorScheme
	String colorSchemeString = server.arg("colorScheme");
	colorScheme = colorSchemeString.toInt();

	switch (colorScheme)
	{
		case 2: //Classic
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

	String ntpServerString = server.arg("ntpServer");
	ntpServerString.toCharArray(ntpServer,20);

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

float checkTemperature(float value)
{
	//HACK: use 999.9 as indicator for NaN

	if (value == -127.0)
		return 999.9;
	else if (value == 0.0)
		return 999.9;
	else if (value == 85.0)
		return 999.9;
	else if (value == 127.0)
		return 999.9;
	else if (value == 127.94)
		return 999.9;
	else
		return value;
}

void readTemperature() {
	USE_SERIAL.print("Start new reading on 1-Wire bus, SC = ");
	USE_SERIAL.println(SecureCounter);

	digitalWrite(LED_BUILTIN, LOW);  //turn the LED on

	time_t now = time(nullptr);
	String time = String(ctime(&now));
	time.toCharArray(measTime, 25);


	String nanStr = "NaN";
	bool readingError = false;
	int cnt = 3; //retry counter

	do
	{
		DS18B20.requestTemperatures(); 
		MeasValue1 = checkTemperature(DS18B20.getTempCByIndex(0));
		if (MeasValue1 == 999.9)
		{
			readingError = true;
			nanStr.toCharArray(Temperature1Str, 6);
		}
		else
		{
			MeasValue1 = squareValue1 * pow(MeasValue1, 2) + gainValue1 * MeasValue1 + offsetValue1;
			dtostrf(MeasValue1, 5, 2, Temperature1Str);
		}

		if (sensorCnt > 1)
		{
			MeasValue2 = checkTemperature(DS18B20.getTempCByIndex(1));
			if (MeasValue2 == 999.9)
			{
				readingError = true;
				nanStr.toCharArray(Temperature2Str, 6);
			}
			else
			{
				MeasValue2 = squareValue2 * pow(MeasValue2, 2) + gainValue2 * MeasValue2 + offsetValue2;
				dtostrf(MeasValue2, 5, 2, Temperature2Str);
			}
		}

		if (!readingError)
			break;
		
		cnt--;
		delay(100);
	} while (cnt > 0);


	//fill measvalue buffer
	cnt_Readings++;
	if (cnt_Readings >= 720 / MEMORY_VALUES_PER_HOUR)
	{
		USE_SERIAL.println("# Store received measvalue in ring buffer");
		
		if (MeasValues_Index >= MEMORY_DEPTH)
		{
			MeasValues_IsFull = true;
			MeasValues_Index = 0;
		}

		if (!toggleSensors)
		{
			if (MeasValue1 == 999.9)
			{
				USE_SERIAL.println("# Invalid temperature reading, ring buffer has been reset!");
				MeasValues_IsFull = false;
				MeasValues_Index = -1;
			}
			else
				MeasValues[MeasValues_Index] = MeasValue1;
		}
		else
		{
			if (MeasValue2 == 999.9)
			{
				USE_SERIAL.println("# Invalid temperature reading, ring buffer has been reset!");
				MeasValues_IsFull = false;
				MeasValues_Index = -1;
			}
			else
				MeasValues[MeasValues_Index] = MeasValue2;
		}

		MeasValues_Index++;
		cnt_Readings = 0;

		USE_SERIAL.print("# Buffer.IsFull = ");
		USE_SERIAL.println(MeasValues_IsFull);
		USE_SERIAL.print("# Buffer.Index = ");
		USE_SERIAL.println(MeasValues_Index);
	}

	SecureCounter++;
	digitalWrite(LED_BUILTIN, HIGH);  //turn the LED off
}

void setup()
{
	//serial debugging
	USE_SERIAL.begin(115200);
	USE_SERIAL.setDebugOutput(true);
	delay(500);

	//print out some crazy startup logo ;)
	USE_SERIAL.println();
	USE_SERIAL.println(R"=====(       _____               ___ ___    _____        )=====");
	USE_SERIAL.println(R"=====(      |_   _|__ _ __  _ __|_  )_ _|__|_   _|       )=====");
	USE_SERIAL.println(R"=====(        | |/ -_) '  \| '_ \/ / | |/ _ \| |         )=====");
	USE_SERIAL.println(R"=====(        |_|\___|_|_|_| .__/___|___\___/|_|         )=====");
	USE_SERIAL.println(R"=====(                     |_|                           )=====");
	USE_SERIAL.println();
	USE_SERIAL.println(R"=====(***************************************************)=====");
	USE_SERIAL.println(R"=====(       a 100prznt.de project by E. Ruemmler        )=====");
	
	USE_SERIAL.print(R"=====(                      v)=====");
	USE_SERIAL.println(VERSION);
	USE_SERIAL.println();
	
	USE_SERIAL.println("Setting up...");

	pinMode(LED_BUILTIN, OUTPUT);	 //initialize the LED_BUILTIN pin as an output
	digitalWrite(LED_BUILTIN, LOW);   //turn the LED on

	client.setInsecure();

	//const uint8_t fingerprint[20] = {0x90, 0x7F, 0x62, 0x4B, 0x5B, 0x7B, 0x70, 0xF6, 0x8A, 0xCE, 0x56, 0x1B, 0xAE, 0x90, 0x0D, 0x49, 0xE4, 0xA5, 0x84, 0x99};
	//client.setFingerprint(fingerprint);



	if (SPIFFS.begin())
	{
		if (SPIFFS.exists("/config.json"))
		{
		//file exists, reading and loading

			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile)
			{
				USE_SERIAL.println("opened config file");
				size_t size = configFile.size();
				//allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonDocument json(1024);
				deserializeJson(json, buf.get());
				serializeJson(json,Serial);

				JsonVariant jsonSystemName = json["systemName"];
				if (!jsonSystemName.isNull())
					strcpy(systemName, json["systemName"]);

				JsonVariant jsonTemp1Name = json["temp1Name"];
				if (!jsonTemp1Name.isNull())
					strcpy(temp1Name, json["temp1Name"]);

				JsonVariant jsonTemp2Name = json["temp2Name"];
				if (!jsonTemp2Name.isNull())
					strcpy(temp2Name, json["temp2Name"]);

				JsonVariant jsonSensorCnt = json["sensorCnt"];
				if (!jsonSensorCnt.isNull())
					sensorCnt = jsonSensorCnt.as<int>();

				JsonVariant jsonToggleTemps = json["toggleSensors"];
				if (!jsonToggleTemps.isNull())
					toggleSensors = jsonToggleTemps.as<bool>();

				JsonVariant jsonPrimaryColor = json["primaryColor"];
				if (!jsonPrimaryColor.isNull())
					strcpy(primaryColor, json["primaryColor"]);

				JsonVariant jsonNtpServer = json["ntpServer"];
				if (!jsonNtpServer.isNull())
					strcpy(ntpServer, json["ntpServer"]);

				USE_SERIAL.println();
				USE_SERIAL.print("primaryColor: ");
				USE_SERIAL.println(primaryColor);

				if (strcmp(primaryColor, "#1e87f0") == 0)
					colorScheme = 2;
				else if (strcmp(primaryColor, "#30a4a1") == 0)
					colorScheme = 3;
				else if (strcmp(primaryColor, "#325c84") == 0)
					colorScheme = 4;
				else if (strcmp(primaryColor, "#f08a00") == 0)
					colorScheme = 5;
				else if (strcmp(primaryColor, "#060d2a") == 0)
					colorScheme = 6;
				else
					colorScheme = 1;

				USE_SERIAL.print("colorScheme: ");
				USE_SERIAL.println(colorScheme);
			}
			else
				USE_SERIAL.println("failed to open /config.json");
		}
		else
			USE_SERIAL.println("/config.json not found");
	}
	else
		USE_SERIAL.println("failed to mount FS");
	//end read

	WiFiManager wifiManager;
	wifiManager.setCustomHeadElement("<style>button,input[type='button'],input[type='submit']{background-color:#ff2e64;color:#fff}</style>");

	WiFiManagerParameter custom_header("<h3>Temp2IoT Settings</h3>");

	//definitinos of custom parameters
	//prepare
	char char_sensorCnt[2];
	itoa(sensorCnt, char_sensorCnt, 10);

	char char_toggleSensors[6];
	if (toggleSensors)
		strncpy(char_toggleSensors, "true", 6);
	else
		strncpy(char_toggleSensors, "false", 6);

	//define
	WiFiManagerParameter custom_systemName("systemName", "Systemname", systemName, 20);
	WiFiManagerParameter custom_temp1Name("temp1Name", "Bezeichnung Sensor 1", temp1Name, 20);
	WiFiManagerParameter custom_temp2Name("temp2Name", "Bezeichnung Sensor 2", temp2Name, 20);

	//add parameters to wifiManager
	wifiManager.addParameter(&custom_header);
	wifiManager.addParameter(&custom_systemName);
	wifiManager.addParameter(&custom_temp1Name);
	wifiManager.addParameter(&custom_temp2Name);

	delay(500);

	//ToDo: Check cust. ntp server address
	//configTime(0, 0, "pool.ntp.org", "time.nist.gov");
	configTime(0, 0, ntpServer, "pool.ntp.org", "time.nist.gov");
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

	//USE_SERIAL.print("IP address: ");
	//USE_SERIAL.println(WiFi.localIP());

	//read updated parameters
	strcpy(systemName, custom_systemName.getValue());
	strcpy(temp1Name, custom_temp1Name.getValue());
	strcpy(temp2Name, custom_temp2Name.getValue());

	//save the custom parameters to FS
	if (shouldSaveConfig)
		saveConfig();

	DS18B20.begin();
	SecureCounter = 0;

	USE_SERIAL.println();
	USE_SERIAL.println("HTTP Temp2IoT server started, you can reach the web UI on:");
	USE_SERIAL.print("http://");
	USE_SERIAL.print(WiFi.localIP());
	USE_SERIAL.println("/");
	digitalWrite(LED_BUILTIN, HIGH);  //turn the LED off
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
	json["ntpServer"] = ntpServer;

	json["offsetValue1"] = offsetValue1;
	json["gainValue1"] = gainValue1;
	json["squareValue1"] = squareValue1;
	json["offsetValue2"] = offsetValue2;
	json["gainValue2"] = gainValue2;
	json["squareValue2"] = squareValue2;

	File configFile = SPIFFS.open("/config.json", "w");

	if (!configFile)
		USE_SERIAL.println("failed to open config file for writing");

	serializeJson(json, Serial);
	serializeJson(json, configFile);
}

void infoReset()
{
	USE_SERIAL.println("Format System");

	//reset Wifi-Setting
	WiFiManager wifiManager;
	wifiManager.resetSettings();

	//format Flash
	SPIFFS.format();

	//restart
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
	float percent = ((float)cur / (float)total) * 100;
	sprintf(progressString, " %s",  String(percent).c_str());

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
	USE_SERIAL.println("UPDATE:  Start update process...");
	ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

	/*USE_SERIAL.println("UPDATE:  Stop ESP8266WebServer");
	server.close();
	server.stop();
	USE_SERIAL.println("UPDATE:  ESP8266WebServer is stopped");

	delay(500);*/


	USE_SERIAL.println("UPDATE:  Register callback events");
	// Add optional callback notifiers
	ESPhttpUpdate.onStart(update_started);
	ESPhttpUpdate.onEnd(update_finished);
	ESPhttpUpdate.onProgress(update_progress);
	ESPhttpUpdate.onError(update_error);


	USE_SERIAL.println("UPDATE:  Call update()");
	t_httpUpdate_return ret = ESPhttpUpdate.update(client, "https://pool.100prznt.de/temp2iot/bin/release/latest/Temp2IoT.ino.d1_mini.bin");


	switch (ret)
	{
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

	if (currentMillis % 5000 == 0) //each 5 seconds
	{ 
		readTemperature();
	}
}
