String web_ui_html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Temp2IoT</title>
    <style type="text/css">
      html
      {
        font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,"Noto Sans",sans-serif,"Apple Color Emoji","Segoe UI Emoji","Segoe UI Symbol","Noto Color Emoji";
        -webkit-text-size-adjust:100%;
        background: #1e87f0;
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
        background: #1e87f0;
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
    .description {
      margin: 0px;
      font-size: 1.5rem;
      line-height: 1.4;
    }
    .units {
      font-size: 2.5rem;
      line-height: 1.4;
    }
    </style>
</head>
  <body>
  <div class="container">
      <div class="card">
        <div class="card-header">
          <h2>
            <span class="icon" style="position: relative; bottom: -7px;">
              <svg vwidth="32" height="32" viewBox="0 0 512 512">
              <path d="M227.89,331.58V77.15c0-31.56-25.59-57.15-57.15-57.15s-57.15,25.59-57.15,57.15v254.43
                c-11.85,13.43-19.05,31.06-19.05,50.37c0,42.08,34.12,76.2,76.2,76.2s76.2-34.12,76.2-76.2
                C246.94,362.63,239.74,345.01,227.89,331.58z M170.74,439.1c-31.56,0-57.15-25.59-57.15-57.15c0-16.92,7.36-32.12,19.05-42.59V77.15
                c0-21.04,17.06-38.1,38.1-38.1s38.1,17.06,38.1,38.1v262.21c11.69,10.46,19.05,25.66,19.05,42.59
                C227.89,413.51,202.3,439.1,170.74,439.1z"/>
              <path d="M285.04,381.95c0,18.38-4.36,35.73-12.07,51.11l17.04,8.52c8.99-17.95,14.08-38.19,14.08-59.63
                c0-21.44-5.09-41.69-14.08-59.63l-17.04,8.52C280.68,346.22,285.04,363.57,285.04,381.95z"/>
              <path d="M342.19,381.95c0,27.57-6.54,53.59-18.11,76.67l17.07,8.54c12.85-25.64,20.09-54.58,20.09-85.21s-7.24-59.57-20.09-85.21
                l-17.07,8.53C335.65,328.36,342.19,354.38,342.19,381.95z"/>
              <path d="M399.34,381.95c0,36.76-8.69,71.48-24.1,102.25l17.04,8.52c16.7-33.33,26.11-70.95,26.11-110.77s-9.41-77.44-26.11-110.77
                l-17.04,8.52C390.65,310.47,399.34,345.19,399.34,381.95z"/>
              <path d="M189.79,348.99V229.55v-76.2c0-10.52-8.53-19.05-19.05-19.05c-10.52,0-19.05,8.53-19.05,19.05v76.2v119.44
                c-11.38,6.59-19.05,18.87-19.05,32.96c0,21.04,17.06,38.1,38.1,38.1s38.1-17.06,38.1-38.1
                C208.84,367.86,201.17,355.58,189.79,348.99z"/>
            </svg>
            </span>
            Temp2IoT
          </h2>
        </div>
        <div class="card-body">
          <h1><span id="temperature">%T%</span><sup class="units">&deg;C</sup></h1>
          <p class="description">Water Temperature</p>
        </div>
        <div class="card-footer">
          <p>Secure Counter: <span id="secure-counter">%SC%</span><br><span class="icon" style="line-height: 2; position: relative; bottom: -2px;"><svg width="16" height="16" viewBox="0 0 20 20" xmlns="http://www.w3.org/2000/svg" data-svg="github"><path d="M10,1 C5.03,1 1,5.03 1,10 C1,13.98 3.58,17.35 7.16,18.54 C7.61,18.62 7.77,18.34 7.77,18.11 C7.77,17.9 7.76,17.33 7.76,16.58 C5.26,17.12 4.73,15.37 4.73,15.37 C4.32,14.33 3.73,14.05 3.73,14.05 C2.91,13.5 3.79,13.5 3.79,13.5 C4.69,13.56 5.17,14.43 5.17,14.43 C5.97,15.8 7.28,15.41 7.79,15.18 C7.87,14.6 8.1,14.2 8.36,13.98 C6.36,13.75 4.26,12.98 4.26,9.53 C4.26,8.55 4.61,7.74 5.19,7.11 C5.1,6.88 4.79,5.97 5.28,4.73 C5.28,4.73 6.04,4.49 7.75,5.65 C8.47,5.45 9.24,5.35 10,5.35 C10.76,5.35 11.53,5.45 12.25,5.65 C13.97,4.48 14.72,4.73 14.72,4.73 C15.21,5.97 14.9,6.88 14.81,7.11 C15.39,7.74 15.73,8.54 15.73,9.53 C15.73,12.99 13.63,13.75 11.62,13.97 C11.94,14.25 12.23,14.8 12.23,15.64 C12.23,16.84 12.22,17.81 12.22,18.11 C12.22,18.35 12.38,18.63 12.84,18.54 C16.42,17.35 19,13.98 19,10 C19,5.03 14.97,1 10,1 L10,1 Z"></path></svg></span> Find the <a href="https://github.com/100prznt/Temp2IoT" target="_blank">Temp2IoT</a> project on GitHub.</p>
      </div>
    </div>
  </div>
  </body>

  <script>
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature").innerHTML = JSON.parse(this.responseText).temperature;
        document.getElementById("secure-counter").innerHTML = JSON.parse(this.responseText).secure_counter;
      }
    };
    xhttp.open("GET", "/api", true);
    xhttp.send();
  }, 10000 ) ;
  </script>
</html>
)rawliteral";
