#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define RXPIN 26
#define TXPIN 33

const char* ssid = "Train";
const char* password = "station03";

AsyncWebServer server(80);
AsyncEventSource events("/sse");
String serial2Out;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial2.begin(115200, SERIAL_8N1, RXPIN, TXPIN);
  
  Serial.println("");
  Serial.println("****  7100 ESP32 Example ****");
  Serial.println("****  Piera Systems      ****");
  Serial.println("");
  Serial.println("Warming up 7100...");

  delay(2000);
  
  // Serial2.write("$Wfactory=\r\n");

  Serial2.write("$Wazure=0\r\n");
  delay(100);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html",
      "<html><head>"
      "<link rel='preconnect' href='https://fonts.googleapis.com'>"
      "<link rel='preconnect' href='https://fonts.gstatic.com' crossorigin>"
      "<link href='https://fonts.googleapis.com/css2?family=Roboto:ital,wght@0,100;0,300;0,400;0,500;0,700;0,900;1,100;1,300;1,400;1,500;1,700;1,900&display=swap' rel='stylesheet'>"
      "<style>"
      "body { font-family: 'Roboto', sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; }"
      ".data-container { display: inline-block; text-align: left; }"
      ".label { display: inline-block; width: 80px; text-align: right; margin-right: 10px; }"
      ".data { display: inline-block; text-align: left; }"
      "</style>"
      "</head><body>"
      "<h1>IPS-7100 Sensor Data</h1>"
      "<div class='data-container'>"
      // "  <h2><span class='label'>PC0.1:</span> <span id='pc01' class='data'></span></h2>"
      // "  <h2><span class='label'>PC0.3:</span> <span id='pc03' class='data'></span></h2>"
      // "  <h2><span class='label'>PC0.5:</span> <span id='pc05' class='data'></span></h2>"
      // "  <h2><span class='label'>PC1.0:</span> <span id='pc10' class='data'></span></h2>"
      // "  <h2><span class='label'>PC2.5:</span> <span id='pc25' class='data'></span></h2>"
      // "  <h2><span class='label'>PC5.0:</span> <span id='pc50' class='data'></span></h2>"
      // "  <h2><span class='label'>PC10:</span> <span id='pc100' class='data'></span></h2>"
      "  <h2><span class='label'>PM0.1:</span> <span id='pm01' class='data'></span></h2>"
      // "  <h2><span class='label'>PM0.3:</span> <span id='pm03' class='data'></span></h2>"
      // "  <h2><span class='label'>PM0.5:</span> <span id='pm05' class='data'></span></h2>"
      // "  <h2><span class='label'>PM1.0:</span> <span id='pm10' class='data'></span></h2>"
      "  <h2><span class='label'>PM2.5:</span> <span id='pm25' class='data'></span></h2>"
      // "  <h2><span class='label'>PM5.0:</span> <span id='pm50' class='data'></span></h2>"
      "  <h2><span class='label'>PM10:</span> <span id='pm100' class='data'></span></h2>"
      "</div>"
      "<script>"
      "if (!!window.EventSource) {"
      "  var source = new EventSource('/sse');"
      "  source.onmessage = function(event) {"
      "    var data = event.data.split(',');"
      // "    document.getElementById('pc01').innerHTML = data[1].trim();"
      // "    document.getElementById('pc03').innerHTML = data[3].trim();"
      // "    document.getElementById('pc05').innerHTML = data[5].trim();"
      // "    document.getElementById('pc10').innerHTML = data[7].trim();"
      // "    document.getElementById('pc25').innerHTML = data[9].trim();"
      // "    document.getElementById('pc50').innerHTML = data[11].trim();"
      // "    document.getElementById('pc100').innerHTML = data[13].trim();"
      "    document.getElementById('pm01').innerHTML = data[15].trim();"
      // "    document.getElementById('pm03').innerHTML = data[17].trim();"
      // "    document.getElementById('pm05').innerHTML = data[19].trim();"
      // "    document.getElementById('pm10').innerHTML = data[21].trim();"
      "    document.getElementById('pm25').innerHTML = data[23].trim();"
      // "    document.getElementById('pm50').innerHTML = data[25].trim();"
      "    document.getElementById('pm100').innerHTML = data[27].trim();"
      "  };"
      "}"
      "</script>"
      "</body></html>");
  });

  server.addHandler(&events);

  server.begin();
}

void loop() {
  if (Serial2.available()) {
    serial2Out = Serial2.readStringUntil('\n');

    Serial.println(serial2Out);

    events.send(serial2Out.c_str(), "message", millis());
  }
}