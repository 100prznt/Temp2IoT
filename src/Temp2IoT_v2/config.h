const char _PAGE_HEAD[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
    <head>
            <title>Temp2IoT - Einstellungen</title>
            <style>
                html {
                    font-size: 70%;
                    box-sizing: border-box;
                }
                html  {
                    padding: 1em;
                }
                *, *::before, *::after {
                margin: 0;
                padding: 0;
                box-sizing: inherit;
                font-family: sans-serif;
                }
                label {
                    display: block;
                    font-size: 18px;
                    border-bottom: 1px solid #eee; 
                    padding: 5px 0;
                    margin: 10px 0 0 0; 
                }
                label.inline {
                    display: block;
                    font-size: 13px;
                    border-bottom: 0 none;
                    padding: 2px 0;
                    margin: 0;
                }
                button, a { 
                    display: inline-block; 
                    padding: 5px 12px 3px 12px; 
                    margin: 2px; 
                    background: #ff2e64;
                    color: #fff;
                    border: 0;
                    text-decoration: none;
                }
                a {
                    font-size: 110%;
                }
                
                h1 { font-size: 24px; margin: 5px 0;}
               
               
            
            </style>
            <meta name = "viewport" content = "width = device-width">
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    </head>
<body>
)=====";

const char _PAGE_START[] PROGMEM  = R"=====(<h1>Temp2IoT</h1><form method="get" action="/config">)=====";

const char _PAGE_ACTIONS[] PROGMEM = R"=====(
<label>Funktionen</label>
    <a href="/reset">Neustart</a>
    <a href="/update">Update</a>
    <a href="/format">Zurücksetzen</a>
)=====";

const char _PAGE_CONFIG_SYSNAME[] PROGMEM  = R"=====(<label>Gerätename (Überschrift)</label> <input type="text" name="systemName" value="%SYSNAME%" >)=====";
const char _PAGE_CONFIG_SENSOR1NAME[] PROGMEM  = R"=====(<label>Bezeichnung Sensor 1</label> <input type="text" name="temp1Name" value="%SENSOR1NAME%" >)=====";
const char _PAGE_CONFIG_SENSOR2NAME[] PROGMEM  = R"=====(<label>Bezeichnung Sensor 2</label> <input type="text" name="temp2Name" value="%SENSOR2NAME%" >)=====";

const char _PAGE_CONFIG_SENSORTOOGLETRUE[] PROGMEM   = R"=====(<label>Sesoren tauschen</label> <input type="checkbox" name="toggleSensors" checked >)=====";
const char _PAGE_CONFIG_SENSORTOOGLEFALSE[] PROGMEM  = R"=====(<label>Sesoren tauschen</label> <input type="checkbox" name="toggleSensors" >)=====";

const char _PAGE_CONFIG_SENSORCNT1[] PROGMEM  = R"=====(<label>Modus</label> <label class="inline"><input type="radio" name="sensorCnt" value="1" checked> Ein Sensor</label> <label  class="inline"><input type="radio" name="sensorCnt" value="2"         > Zwei Sensoren</label>)=====";
const char _PAGE_CONFIG_SENSORCNT2[] PROGMEM  = R"=====(<label>Modus</label> <label class="inline"><input type="radio" name="sensorCnt" value="1"        > Ein Sensor</label> <label  class="inline"><input type="radio" name="sensorCnt" value="2" checked > Zwei Sensoren</label>)=====";

const char _PAGE_FOOTER[] PROGMEM = R"=====(<br />  <br /> <button type="submit">Einstellungen übernehmen</button></form></body></html>)=====";
