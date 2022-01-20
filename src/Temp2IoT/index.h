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
 * @package    Temp2IoT
 * @author     Elias Ruemmler <e.ruemmler@rc-art.de>
 * @copyright  2022 RC-Art Solutions
 * @version    2.3
 * @link       https://github.com/100prznt/Temp2IoT
 * @since      2020-06-17
 */

//PROGMEM            <- Ablage im Flash
//R"=====(  )====="  <- Deklaration rawstring, auch Steuerzeichen zulässig

const char _PAGE_WEBUI_HEAD[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Temp2IoT</title>
  <link rel="icon" type="image/svg+xml" href="favicon.svg" sizes="any">
  <style type="text/css">
    html
    {
      font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,"Noto Sans",sans-serif,"Apple Color Emoji","Segoe UI Emoji","Segoe UI Symbol","Noto Color Emoji";
      -webkit-text-size-adjust:100%;
      background: %COLORPRIM%;
      color:#666;
    }

    @media all and (min-width:600px) {
      .card {
        box-shadow: 0 5px 15px rgba(0,0,0,.2);
      }
    }

    body
    {
      margin:0;
    }
    .container {
      right: 0%;
      top: 50%;
      position: fixed;
      -webkit-transform: translateY(-50%);
      -moz-transform: translateY(-50%);
      -ms-transform: translateY(-50%);
      -o-transform: translateY(-50%);
      transform: translateY(-50%);
      width: 100%;
    }
    .card {
      margin: auto;
      background: %COLORPRIM%;
      color: #fff;
      width: 400px;
    }
    .card-header {
      padding: 20px 40px;
      margin: 0 0 20px 0;
    }
    .card-header h2,
    .card-body h1 {
      font-family: -apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,"Noto Sans",sans-serif,"Apple Color Emoji","Segoe UI Emoji","Segoe UI Symbol","Noto Color Emoji";
      font-weight: 400;
      text-transform: none;
    }
    .card-header h2 {
      font-size: 2.0rem;
      line-height: 1.4;
    }
    .card-header a{
      color: #fff;
      text-decoration: none;
      cursor: pointer;
    }
    .card-body h1 {
      font-size: 5rem;
      line-height: 0.8;
      margin: 0 0 15px 0;
    }
    .card-body {
      padding: 40px 40px;
    }
    .card-footer {
      padding: 20px 40px;
      color: rgba(255,255,255,.7);
    }
    .card-footer a{
      color: #fff;
      text-decoration: none;
      cursor: pointer;
    }
    .icon {
      fill: currentColor;
    }
    .trend {
      float: left;
      padding: 0 10px 0 0;
    }
    .trend-value {
      color: rgba(255,255,255,.7);
    }
    .description {
      margin: 0px;
      font-size: 1.5rem;
      line-height: 1.6;
    }
    .units {
      font-size: 2.5rem;
      line-height: 1.4;
    }
    .secondunits {
      font-size: 1.8rem;
      line-height: 1;
    }
    .second h1 {
      font-size: 2.9rem;
      line-height: 0.6;
    }
    .second p {
      font-size: 1.1rem;
      line-height: 0.9;
    }
  </style>
</head>
<body>
)=====";


const char _PAGE_WEBUI_CARDHEAD[] PROGMEM  = R"=====(
<div class="container">
      <div class="card">
        <div class="card-header">
          <h2>
            <span class="icon" style="position: relative; bottom: -5px;">
            <a title="Einstellungen öffnen" href="/setup">
              <img src="favicon.svg" width="36" height="36">
            </a>
            </span>
            %NAME%
          </h2>
        </div>
)=====";

const char _PAGE_WEBUI_CARDBODYSINGLE[] PROGMEM  = R"=====(
        <div class="card-body">
          <h1><span id="temperature1">%T1%</span><sup class="units">&deg;C</sup></h1>
          <p class="description">%N1%</p>
        </div>
)=====";

const char _PAGE_WEBUI_CARDBODY[] PROGMEM  = R"=====(
        <div class="card-body">
          <h1><span id="temperature1">%T1%</span><sup class="units">&deg;C</sup></h1>
          <p class="description">%N1%</p>
          <div class="second">
            <h1><span id="temperature2">%T2%</span><sup class="secondunits">&deg;C</sup></h1>
            <p class="description">%N2%</p>
          </div>
        </div>
)=====";

const char _PAGE_WEBUI_CARDFOOTER[] PROGMEM  = R"=====(
<div class="card-footer">
          <p>SC: <span id="secure-counter">%SC%</span><br>
          <span class="icon" style="line-height: 2; position: relative; bottom: -2px;">
            <svg width="16" height="16" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg" data-svg="github"><path d="M10,1 C5.03,1 1,5.03 1,10 C1,13.98 3.58,17.35 7.16,18.54 C7.61,18.62 7.77,18.34 7.77,18.11 C7.77,17.9 7.76,17.33 7.76,16.58 C5.26,17.12 4.73,15.37 4.73,15.37 C4.32,14.33 3.73,14.05 3.73,14.05 C2.91,13.5 3.79,13.5 3.79,13.5 C4.69,13.56 5.17,14.43 5.17,14.43 C5.97,15.8 7.28,15.41 7.79,15.18 C7.87,14.6 8.1,14.2 8.36,13.98 C6.36,13.75 4.26,12.98 4.26,9.53 C4.26,8.55 4.61,7.74 5.19,7.11 C5.1,6.88 4.79,5.97 5.28,4.73 C5.28,4.73 6.04,4.49 7.75,5.65 C8.47,5.45 9.24,5.35 10,5.35 C10.76,5.35 11.53,5.45 12.25,5.65 C13.97,4.48 14.72,4.73 14.72,4.73 C15.21,5.97 14.9,6.88 14.81,7.11 C15.39,7.74 15.73,8.54 15.73,9.53 C15.73,12.99 13.63,13.75 11.62,13.97 C11.94,14.25 12.23,14.8 12.23,15.64 C12.23,16.84 12.22,17.81 12.22,18.11 C12.22,18.35 12.38,18.63 12.84,18.54 C16.42,17.35 19,13.98 19,10 C19,5.03 14.97,1 10,1 L10,1 Z"></path></svg>
          </span> Das <a href="https://github.com/100prznt/Temp2IoT" target="_blank">Temp2IoT</a> Projekt auf GitHub.</p>
      </div>
    </div>
  </div>
)=====";

const char _PAGE_WEBUI_FOOTER_SCRIPT1[] PROGMEM  = R"=====(
</body>
  <script>
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature1").innerHTML = JSON.parse(this.responseText).sensors[0].value;
        document.getElementById("secure-counter").innerHTML = JSON.parse(this.responseText).secure_counter;
      }
    };
    xhttp.open("GET", "/api", true);
    xhttp.send();
  }, 5000 ) ;
  </script>
</html>
)=====";

const char _PAGE_WEBUI_FOOTER_SCRIPT2[] PROGMEM  = R"=====(
</body>
  <script>
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature1").innerHTML = (JSON.parse(this.responseText).sensors[0].value).toFixed(2);
        document.getElementById("temperature2").innerHTML = (JSON.parse(this.responseText).sensors[1].value).toFixed(2);
        document.getElementById("secure-counter").innerHTML = JSON.parse(this.responseText).secure_counter;
      }
    };
    xhttp.open("GET", "/api", true);
    xhttp.send();
  }, 5000 ) ;
  </script>
</html>
)=====";


const char _PAGE_WEBUI_FOOTER[] PROGMEM = "</body></html>";
