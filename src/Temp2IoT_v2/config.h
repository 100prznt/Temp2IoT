const char _PAGE_HEAD[] PROGMEM = R"=====(
<HTML>
    <HEAD>
            <TITLE>Temp2IoT - Settings</TITLE>
            <style>
                html {
                font-size: 62.5%;
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
                    background: #eee;
                    color: #000;
                    border: 0;
                    text-decoration: none;
                }
                
                h1 { font-size: 24px; margin: 5px 0;}
               
               
            
            </style>
            <meta name = "viewport" content = "width = device-width">
            <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
    </HEAD>
<BODY>
)=====";

const char _PAGE_FOOTER[] PROGMEM = "<br />  <br /> <button type=submit>Config aktualisieren</button></form></BODY></HTML>";
const char _PAGE_START[] PROGMEM  = "<h1>Temp2IoT</h1><form method=get action=/config>";

const char _PAGE_ACTIONS[] PROGMEM = "<label>Funktionen</label>    <a href='/reset'>Neustart</a>   <a href='/update'>Update</a> <a href='/format'>Zurücksetzen</a>";

const char _PAGE_CONFIG_NAME[] PROGMEM  = "<label>Instagram-Name:</label> <input type=text name=instagramname value=%INSTAGRAM% >";
const char _PAGE_CONFIG_MODE1[] PROGMEM  = "<label>Modus: </label> <label class=inline><input type=radio name=mode value=1 checked> Nur Counter</label> <label  class=inline><input type=radio name=mode value=2         > Nur Uhr</label> <label class=inline><input type=radio name=mode value=3> Counter und Uhr im Wechsel</label>" ;
const char _PAGE_CONFIG_MODE2[] PROGMEM  = "<label>Modus: </label> <label class=inline><input type=radio name=mode value=1        > Nur Counter</label> <label  class=inline><input type=radio name=mode value=2 checked > Nur Uhr</label> <label class=inline><input type=radio name=mode value=3> Counter und Uhr im Wechsel</label>" ;
const char _PAGE_CONFIG_MODE3[] PROGMEM  = "<label>Modus: </label> <label class=inline><input type=radio name=mode value=1        > Nur Counter</label> <label  class=inline><input type=radio name=mode value=2         > Nur Uhr</label> <label class=inline><input type=radio name=mode value=3 checked> Counter und Uhr im Wechsel</label>" ;

const char _PAGE_CONFIG_INTENSITY[] PROGMEM  = "<label>Helligkeit</label> <input type=range min=0 max=15 name=intensity value=%INTENSITY% >" ;