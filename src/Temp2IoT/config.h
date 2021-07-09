const char _PAGE_HEAD[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
    <head>
            <title>Temp2IoT - Einstellungen</title>
            <link rel="icon" type="image/svg+xml" href="favicon.svg" sizes="any">
            <style>
                html {
                    font-size: 80%;
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
                    border-bottom: 1px solid #ddd; 
                    padding: 5px 0;
                    margin: 10px 0 0 0; 
                }
                label.inline {
                    display: block;
                    font-size: 15px;
                    border-bottom: 0 none;
                    padding: 2px 0;
                    margin: 0;
                }
                button, a { 
                    display: inline-block; 
                    padding: 5px 12px 3px 12px; 
                    margin: 2px; 
                    background: %COLORPRIM%;
                    color: #fff;
                    border: 0;
                    text-decoration: none;
                    font-size: 110%;
                }                
                h1 {
                    font-size: 24px; margin: 5px 0;
                }
            </style>
            <meta name = "viewport" content = "width = device-width">
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    </head>
<body>
)=====";

const char _PAGE_START[] PROGMEM  = R"=====(<h1>Temp2IoT</h1><p>v%VERSION%<br>a 100prznt.de project by E. Ruemmler</p><form method="get" action="/config">)=====";

const char _PAGE_ACTIONS[] PROGMEM = R"=====(
<label>Funktionen</label>
    <a href="/reset">Neustart</a>
    <a href="/update">Update</a>
    <a href="/format">Zurücksetzen</a>
)=====";

const char _PAGE_CONFIG_SYSNAME[] PROGMEM  = R"=====(<label>Gerätename (Überschrift)</label> <input type="text" name="systemName" value="%SYSNAME%" >)=====";
const char _PAGE_CONFIG_SENSOR1NAME[] PROGMEM  = R"=====(<label>Bezeichnung Sensor 1</label> <input type="text" name="temp1Name" value="%SENSOR1NAME%" >)=====";
const char _PAGE_CONFIG_SENSOR2NAME[] PROGMEM  = R"=====(<label>Bezeichnung Sensor 2</label> <input type="text" name="temp2Name" value="%SENSOR2NAME%" >)=====";

const char _PAGE_CONFIG_SENSORTOOGLETRUE[] PROGMEM   = R"=====(<label>Sensoren tauschen</label> <input type="checkbox" name="toggleSensors" checked >)=====";
const char _PAGE_CONFIG_SENSORTOOGLEFALSE[] PROGMEM  = R"=====(<label>Sensoren tauschen</label> <input type="checkbox" name="toggleSensors" >)=====";

const char _PAGE_CONFIG_SENSORCNT1[] PROGMEM  = R"=====(<label>Modus</label> <label class="inline"><input type="radio" name="sensorCnt" value="1" checked> Ein Sensor</label> <label  class="inline"><input type="radio" name="sensorCnt" value="2"         > Zwei Sensoren</label>)=====";
const char _PAGE_CONFIG_SENSORCNT2[] PROGMEM  = R"=====(<label>Modus</label> <label class="inline"><input type="radio" name="sensorCnt" value="1"        > Ein Sensor</label> <label  class="inline"><input type="radio" name="sensorCnt" value="2" checked > Zwei Sensoren</label>)=====";

const char _PAGE_CONFIG_COLORSCHEME_1[] PROGMEM  = R"=====(
<label>Erscheinungsbild</label>
<select name="colorScheme">
<option value="1" selected>100prznt</option>
<option value="2">Classic</option>
<option value="3">Total</option>
<option value="4">Power</option>
<option value="5">Sun</option>
<option value="6">Dark</option>
</select>
)=====";
const char _PAGE_CONFIG_COLORSCHEME_2[] PROGMEM  = R"=====(
<label>Erscheinungsbild</label>
<select name="colorScheme">
<option value="1">100prznt</option>
<option value="2" selected>Classic</option>
<option value="3">Total</option>
<option value="4">Power</option>
<option value="5">Sun</option>
<option value="6">Dark</option>
</select>
)=====";
const char _PAGE_CONFIG_COLORSCHEME_3[] PROGMEM  = R"=====(
<label>Erscheinungsbild</label>
<select name="colorScheme">
<option value="1">100prznt</option>
<option value="2">Classic</option>
<option value="3" selected>Total</option>
<option value="4">Power</option>
<option value="5">Sun</option>
<option value="6">Dark</option>
</select>
)=====";
const char _PAGE_CONFIG_COLORSCHEME_4[] PROGMEM  = R"=====(
<label>Erscheinungsbild</label>
<select name="colorScheme">
<option value="1">100prznt</option>
<option value="2">Classic</option>
<option value="3">Total</option>
<option value="4" selected>Power</option>
<option value="5">Sun</option>
<option value="6">Dark</option>
</select>
)=====";
const char _PAGE_CONFIG_COLORSCHEME_5[] PROGMEM  = R"=====(
<label>Erscheinungsbild</label>
<select name="colorScheme">
<option value="1">100prznt</option>
<option value="2">Classic</option>
<option value="3">Total</option>
<option value="4">Power</option>
<option value="5" selected>Sun</option>
<option value="6">Dark</option>
</select>
)=====";
const char _PAGE_CONFIG_COLORSCHEME_6[] PROGMEM  = R"=====(
<label>Erscheinungsbild</label>
<select name="colorScheme">
<option value="1">100prznt</option>
<option value="2">Classic</option>
<option value="3">Total</option>
<option value="4">Power</option>
<option value="5">Sun</option>
<option value="6" selected>Dark</option>
</select>
)=====";

const char _PAGE_CONFIG_NTP[] PROGMEM  = R"=====(<label>NTP Server</label> <input type="text" name="ntpServer" value="%NTPSERVER%" >)=====";

const char _PAGE_FOOTER[] PROGMEM = R"=====(<br />  <br /> <button type="submit">Einstellungen übernehmen</button></form>
<label>Links</label>
<a href="https://github.com/100prznt/Temp2IoT" target="_blank">Temp2IoT Repository</a>
<a href="https://100prznt.de/" target="_blank">100prznt.de</a>
<a href="/api" target="_blank">API</a>
<a href="/">WebUI</a>

</body>
</html>)=====";
