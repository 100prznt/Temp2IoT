<img src="docu/Temp2IoT_Icon_Black.svg" alt="Temp2IoT wiring" width="64" height="64" />

# Temp2IoT
Very basic IoT thermometer with REST API and Web-UI. Implemented according to the "KISS" approach - **K**eep **I**t **S**imple and **S**tupid

## Features
* Temperature measurement at one measuring points using a DS18B20 sensors
* Shows the measured values on integrated Web UI
* REST API for integration with master systems (e.g. ioBroker)


## Hardware

### BOM

| Description          | Qty.   | Price     | ASIN (partner-link from amazon.de)    |
|----------------------|--------|----------:|---------------------------------------|
| WeMos D1 mini        | 1      | EUR 5,99  | [B01N9RXGHY](https://amzn.to/3clRAiP) |
| DS18B20 waterproof   | 1 or 2 | EUR 3,25  | [B01MZG48OE](https://amzn.to/2NQUkvc) |
| Resistor 2K2 (0,6 W) | 1      | EUR 0,99  | [B007R3QXUE](https://amzn.to/3ja1jww) |
|----------------------|--------|-----------|---------------------------------------|
| Total                |        | EUR 10,23 |                                       |

*quoted prices from 2021/06/24*

Also a USB-micro cable and a USB power supply is required

### Wiring

![Temp2IoT wiring](hardware/temp2iot_wiring_2.png)

### Power Supply
:warning: Caution Danger to life :warning:

If the thermometer is used to measure water temperatures, it is essential to use an appropriately classified power supply unit. It is important that the power supply is designed as a safety transformer.

<img src="docu/Sitrenn.svg" alt="Temp2IoT wiring" width="180" height="210" />
Symbol of a short-circuit proof, closed safety transformer 

### Photos

![Temp2IoT minimal build up](hardware/hardware_raw.jpg)
Minimal build up with SMD resistor on the back and only one connected DS18B20. 

## Setup

You have two options to upload the software to the WeMos D1 mini.

### ESP8266 Flasher (Windows)

You can download the compiled Flasher Tool from the GitHub Repository
* [nodemcu/nodemcu-flasher](https://github.com/nodemcu/nodemcu-flasher)

1. Clone or download the repository
2. Start the ESP8266Flasher.exe
3. Select the pre-compiled Temp2IoT binary on the config-tab, find at: `src\Temp2IoT\emp2IoT.ino.d1_mini.bin`
4. Switch back to the operation-tab and select the right COM-port
5. Press the Flash-Button

### Arduino IDE

If no WeMos D1 mini have been programmed with the Arduino IDE so far, an appropriate setup is required first.
* [Wiki - Setup Arduino IDE for WeMos D1 mini](https://github.com/100prznt/Temp2IoT/wiki/Setup-Arduino-IDE)

1. Clone or download the repository
2. Open the `src\Temp2IoT\Temp2IoT.ino` with the Arduino IDE
3. Compile and upload
4. Scan for WiFi networks and try to connect the WiFi with SSID: "Temp2IoT"
5. Select your local WiFi from the list, enter your passwort an save the configuration
6. Open the serial monitor (in Arduino IDE) to see the WiFi status and the applied IP address


## REST API
* URL: `http://<Temp2IoT IP>/api`
* Method: `GET`

```
{
  "systemname": "Temp2IoT",
  "secure_counter": 999,
  "firmware": "2.0a",
  "sensors": [
    {
      "name": "name of sensor 1",
      "value": "25.44",
      "unit": "Celsius",
      "time": "Thu Jun 24 20:15:00 2021"
    },
    {
      "name": "name of sensor 2",
      "value": "25.25",
      "unit": "Celsius",
      "time": "Thu Jun 24 20:15:05 2021"
    }
  ]
}
```

## Web UI
* URL: `http://<Temp2IoT IP>/`
  <br>
  <br>
  <br>
![Web UI on a desktop browser](docu/webui_desktop.png)
Web UI on a desktop browser
  <br>
  <br>
  <br>
![Web UI on a smartphone browser](docu/webui_smartphone.png)
Web UI on a smartphone browser

## System Setup
* URL: `http://<Temp2IoT IP>/setup/`
  <br>
  <br>
  <br>
![Web UI on a desktop browser](docu/setup_desktop.png)
Setup page on a desktop browser

## Color Scheme
<img src="docu/Scheme_100prznt.png" alt="Temp2IoT Color Scheme 100prznt" width="260" height="332" /><img src="docu/Scheme_Classic.png" alt="Temp2IoT Color Scheme CLASSIC" width="260" height="332" /><img src="docu/Scheme_Total.png" alt="Temp2IoT Color Scheme TOTAL" width="260" height="332" /><img src="docu/Scheme_Sun.png" alt="Temp2IoT Color Scheme SUN" width="260" height="332" /><img src="docu/Scheme_Power.png" alt="Temp2IoT Color Scheme POWER" width="260" height="332" /><img src="docu/Scheme_100przntDark.png" alt="Temp2IoT Color Scheme DARK" width="260" height="332" />

## Enclosure
In the folder [hardware/enclosure](hardware/enclosure) you will find 2 STL files of a suitable enclosure. If the cover is printed upside-down, you can print the first two layers in a different color to make the Temp2IoT icon stand out better.

### Photos

![3d-printed enclousure with Temp2IoT icon](hardware/temp2iot_logo_case.jpg)
3d-printed enclousure with Temp2IoT icon
  <br>
  <br>
![3d-printed enclousure open](hardware/temp2iot_logo_case_open.jpg)
Blackened inside so that the LED produces a focused light spot
