<img src="docu/Temp2IoT_Icon_Black.svg" alt="Temp2IoT wiring" width="64" height="64" />

# Temp2IoT
Very basic IoT thermometer with REST API and Web-UI. Implemented according to the "KISS" approach - **K**eep **I**t **S**imple and **S**tupid

## Features
* Temperature measurement at one measuring points using a DS18B20 sensor
* Shows the measured values on integrated Web UI
* REST API for integration with master systems


## Hardware

### BOM

| Description          | Qty. | Price    | ASIN (partner-link from amazon.de)    |
|----------------------|------|----------:|---------------------------------------|
| WeMos D1 mini        | 1    | EUR 5,99  | [B076FBY2V3](https://amzn.to/30WTQI6) |
| DS18B20 waterproof   | 1    | EUR 2,65  | [B07THK58HZ](https://amzn.to/30WTQI6) |
| Resistor 4K7 (1/4 W) | 1    | EUR 1,68  | [B01LXS3YX2](https://amzn.to/30WTQI6) |
|----------------------|------|-----------|---------------------------------------|
| Total                |      | EUR 10,32 |                                       |

*quoted prices from 2020/06/18*

Also a USB-micro cable and a USB power supply is required

### Wiring

![Temp2IoT wiring](hardware/temp2iot_wiring.png)

### Photos

![Temp2IoT minimal build up](hardware/hardware_raw.jpg)
Minimal build up with SMD resistor on the back
  <br>
  <br>
![Temp2IoT Logo-Case](hardware/temp2iot_logo_case.jpg)
3d-printed enclosure with the Temp2IoT icon

## Setup
1. Clone or download the repository
2. Put your credentials in the `src\Temp2IoT\WifiCredentials.h.template` file and rename the file to `WifiCredentials.h`
3. Open the `src\Temp2IoT\Temp2IoT.ino` with the Arduino IDE
4. Compile and upload
5. Open the serial monitor (in Arduino IDE) to see the WiFi status and the applied IP address


## REST API
* URL: `http://<Temp2IoT IP>/api`
* Method: `GET`

```
{
  "secure_counter": 3,
  "symbol": "°C",
  "temperature": "25.63",
  "unit": "Celsius"
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
