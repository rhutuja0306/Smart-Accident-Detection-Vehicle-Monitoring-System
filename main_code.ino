#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

/* =========================
          WIFI
========================= */

const char* ssid = "abc123";
const char* password = "abc12345";

/* =========================
      THINGSPEAK API
========================= */

String apiKey = "BO665ZRKNHMNI8W0";

/* =========================
        WEB SERVER
========================= */

WebServer server(80);

/* =========================
        ADXL345
========================= */

Adafruit_ADXL345_Unified accel(12345);

/* =========================
            LCD
========================= */

LiquidCrystal_I2C lcd(0x27, 16, 2);

/* =========================
            GPS
========================= */

TinyGPSPlus gps;

HardwareSerial gpsSerial(1);

/* =========================
            PINS
========================= */

const int vibrationPin = 19;

const int buzzerPin = 18;

/* =========================
        THRESHOLD
========================= */

float accelThreshold = 10.10;

/* =========================
      DASHBOARD VARIABLES
========================= */

bool accidentDetected = false;

double currentLat = 0;

double currentLng = 0;

/* =========================
      ALERT TIMER
========================= */

unsigned long alertStartTime = 0;

bool alertActive = false;

const unsigned long alertDuration = 60000;

/* =========================
          HTML PAGE
========================= */

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<title>ESP32 Accident Dashboard</title>

<meta name="viewport"
content="width=device-width, initial-scale=1.0">

<style>

body{
font-family:Arial;
background:#0f172a;
color:white;
display:flex;
justify-content:center;
align-items:center;
height:100vh;
margin:0;
}

.container{
background:#1e293b;
padding:30px;
border-radius:20px;
text-align:center;
width:90%;
max-width:400px;
box-shadow:0 0 20px rgba(0,0,0,0.5);
}

h1{
margin-bottom:20px;
color:#38bdf8;
}

.status{
padding:25px;
font-size:28px;
font-weight:bold;
border-radius:15px;
margin-bottom:20px;
}

.safe{
background:#16a34a;
}

.danger{
background:#dc2626;
animation:blink 1s infinite;
}

@keyframes blink{
50%{
opacity:0.5;
}
}

button{
width:100%;
padding:15px;
border:none;
border-radius:12px;
font-size:18px;
font-weight:bold;
cursor:pointer;
background:#22c55e;
color:white;
}

</style>

</head>

<body>

<div class="container">

<h1>IoT Accident Detection</h1>

<div id="statusBox"
class="status safe">

 VEHICLE IS SAFE

</div>

<a id="mapLink"
href="#"
target="_blank"
style="display:none;">

<button>

 View Location

</button>

</a>

</div>

<script>

async function fetchData(){

try{

const response =
await fetch('/data');

const data =
await response.json();

let statusBox =
document.getElementById('statusBox');

let mapLink =
document.getElementById('mapLink');

if(data.accident){

statusBox.className =
'status danger';

statusBox.innerHTML =
' ACCIDENT DETECTED';

mapLink.style.display =
'block';

mapLink.href =
'https://maps.google.com/?q='
+ data.lat + ','
+ data.lng;

}

else{

statusBox.className =
'status safe';

statusBox.innerHTML =
' VEHICLE IS SAFE';

mapLink.style.display =
'none';

}

}
catch(error){

console.log(error);

}

}

/* REFRESH EVERY 5 SEC */

setInterval(fetchData,5000);

fetchData();

</script>

</body>
</html>

)rawliteral";

/* =========================
        HOME PAGE
========================= */

void handleRoot(){

server.send_P(
200,
"text/html",
webpage
);

}

/* =========================
          JSON DATA
========================= */

void handleData(){

String json = "{";

json += "\"accident\":";

if(accidentDetected)
json += "true";
else
json += "false";

json += ",";

json += "\"lat\":";
json += String(currentLat,6);

json += ",";

json += "\"lng\":";
json += String(currentLng,6);

json += "}";

server.send(
200,
"application/json",
json
);

}

/* =========================
      THINGSPEAK FUNCTION
========================= */

void sendToThingSpeak(
double lat,
double lng,
float accelValue,
int vibrationValue
) {

if (WiFi.status() == WL_CONNECTED) {

HTTPClient http;

String url =
"http://api.thingspeak.com/update?api_key="
+ apiKey +
"&field1=" + String(lat,6) +
"&field2=" + String(lng,6) +
"&field3=1" +
"&field4=1" +
"&field5=" + String(accelValue) +
"&field6=" + String(vibrationValue);

http.begin(url);

int response = http.GET();

http.end();

}

}

/* =========================
            SETUP
========================= */

void setup() {

Serial.begin(115200);

Serial.setTxBufferSize(1024);

/* GPS */

gpsSerial.begin(
9600,
SERIAL_8N1,
16,
17
);

/* PIN MODES */

pinMode(vibrationPin, INPUT);

pinMode(buzzerPin, OUTPUT);

/* LCD */

lcd.begin();

lcd.backlight();

lcd.setCursor(0,0);

lcd.print("Connecting WiFi");

/* WIFI */

WiFi.begin(ssid, password);

WiFi.setSleep(false);

while (
WiFi.status() != WL_CONNECTED
) {

delay(500);

Serial.print(".");

}

lcd.clear();

lcd.setCursor(0,0);

lcd.print("WiFi Connected");

delay(2000);

lcd.clear();

/* ADXL345 */

if (!accel.begin()) {

while (1);

}

accel.setRange(
ADXL345_RANGE_16_G
);

/* WEB SERVER */

server.on("/", handleRoot);

server.on("/data", handleData);

server.begin();

Serial.println("");

Serial.println("Web Server Started");

Serial.print("Open Browser: ");

Serial.println(WiFi.localIP());

}

/* =========================
              LOOP
========================= */

void loop() {

/* HANDLE WEB CLIENT */

server.handleClient();

/* READ GPS */

while (gpsSerial.available()) {

gps.encode(
gpsSerial.read()
);

}

/* READ ACCELEROMETER */

sensors_event_t event;

accel.getEvent(&event);

float totalAccel = sqrt(

event.acceleration.x *
event.acceleration.x +

event.acceleration.y *
event.acceleration.y +

event.acceleration.z *
event.acceleration.z

);

/* READ VIBRATION */

int vibration =
digitalRead(vibrationPin);

/* =========================
      ACCIDENT DETECTION
========================= */

if (
totalAccel > accelThreshold
&& vibration == HIGH
&& !alertActive
) {

alertActive = true;

accidentDetected = true;

alertStartTime = millis();

digitalWrite(
buzzerPin,
HIGH
);

/* LCD */

lcd.setCursor(0,0);

lcd.print("ACCIDENT!      ");

/* GPS */

currentLat = 0;

currentLng = 0;

if (gps.location.isValid()) {

currentLat =
gps.location.lat();

currentLng =
gps.location.lng();

}

/* SEND TO THINGSPEAK */

sendToThingSpeak(
currentLat,
currentLng,
totalAccel,
vibration
);

}

/* =========================
      KEEP ALERT ACTIVE
========================= */

if(alertActive){

server.handleClient();

if(millis() - alertStartTime
>= alertDuration){

alertActive = false;

accidentDetected = false;

digitalWrite(
buzzerPin,
LOW
);

/* LCD */

lcd.setCursor(0,0);

lcd.print("Vehicle Safe   ");

}

}

/* =========================
        SAFE MODE
========================= */

if(!alertActive){

lcd.setCursor(0,0);

lcd.print("Vehicle Safe   ");

}

/* SMALL DELAY */

delay(10);

yield();

}