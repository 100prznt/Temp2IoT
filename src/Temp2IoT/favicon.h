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
 * @version    2.2
 * @link       https://github.com/100prznt/Temp2IoT
 * @since      2020-06-17
 */

//PROGMEM            <- Ablage im Flash
//R"=====(  )====="  <- Deklaration rawstring, auch Steuerzeichen zulässig

const char _FAVICON_SVG_WHITE[] PROGMEM = R"=====(<?xml version="1.0" encoding="utf-8"?>
<!-- Generator: 100prznt.de  -->
<svg version="1.1" id="Layer1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" x="0px" y="0px"
	 viewBox="0 0 512 512" style="enable-background:new 0 0 512 512;" xml:space="preserve">
<path d="M227.89,331.58V77.15c0-31.56-25.59-57.15-57.15-57.15s-57.15,25.59-57.15,57.15v254.43
	c-11.85,13.43-19.05,31.06-19.05,50.37c0,42.08,34.12,76.2,76.2,76.2s76.2-34.12,76.2-76.2
	C246.94,362.63,239.74,345.01,227.89,331.58z M170.74,439.1c-31.56,0-57.15-25.59-57.15-57.15c0-16.92,7.36-32.12,19.05-42.59V77.15
	c0-21.04,17.06-38.1,38.1-38.1s38.1,17.06,38.1,38.1v262.21c11.69,10.46,19.05,25.66,19.05,42.59
	C227.89,413.51,202.3,439.1,170.74,439.1z" fill="white"/>
<path d="M285.04,381.95c0,18.38-4.36,35.73-12.07,51.11l17.04,8.52c8.99-17.95,14.08-38.19,14.08-59.63
	c0-21.44-5.09-41.69-14.08-59.63l-17.04,8.52C280.68,346.22,285.04,363.57,285.04,381.95z" fill="white"/>
<path d="M342.19,381.95c0,27.57-6.54,53.59-18.11,76.67l17.07,8.54c12.85-25.64,20.09-54.58,20.09-85.21s-7.24-59.57-20.09-85.21
	l-17.07,8.53C335.65,328.36,342.19,354.38,342.19,381.95z" fill="white"/>
<path d="M399.34,381.95c0,36.76-8.69,71.48-24.1,102.25l17.04,8.52c16.7-33.33,26.11-70.95,26.11-110.77s-9.41-77.44-26.11-110.77
	l-17.04,8.52C390.65,310.47,399.34,345.19,399.34,381.95z" fill="white"/>
<path d="M189.79,348.99V229.55v-76.2c0-10.52-8.53-19.05-19.05-19.05c-10.52,0-19.05,8.53-19.05,19.05v76.2v119.44
	c-11.38,6.59-19.05,18.87-19.05,32.96c0,21.04,17.06,38.1,38.1,38.1s38.1-17.06,38.1-38.1
	C208.84,367.86,201.17,355.58,189.79,348.99z" fill="white"/>
</svg>)=====";