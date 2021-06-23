const char _PAGE_WEBUI_HEAD[] PROGMEM = R"=====(
<html>
    <head>
            <title>Temp2IoT</title>
            <meta name = "viewport" content = "width = device-width">
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    </head>
<body>
)=====";



const char _PAGE_WEBUI_START[] PROGMEM  = "<h1>%NAME%</h1>";
const char _PAGE_WEBUI_SENSOR1[] PROGMEM = "<h2>Sensor1: %NAME%</h2><p>%VALUE% degC</p>";
const char _PAGE_WEBUI_SENSOR2[] PROGMEM = "<h2>Sensor2: %NAME%</h2><p>%VALUE% degC</p>";
const char _PAGE_WEBUI_ERROR[] PROGMEM = "<h2>ERROR</h2>";
const char _PAGE_WEBUI_FOOTER[] PROGMEM = "</body></html>";