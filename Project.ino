#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define pin 02
#define buzz 14
#define buton 16
#define DHTTYPE DHT11
DHT dht(pin, DHTTYPE);

#define D_W 128
#define D_H 64

//global variables
int flag = 0;
int swtch = 0;

Adafruit_SSD1306 oled(D_W, D_H, &Wire, -1);

// SSID and Password for network connection
#define SSID "MANI SURYA TEJA's S23"
#define passwd "kkkkkkkk"

float humidity;
float Temp;
float TempF;
float set;
float threshold = 1000; // Default threshold value

ESP8266WebServer server(80);

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Threshold Input</title>
</head>
<body>
  <h1>Set Humidity Threshold</h1>
  
  <form action="/setThreshold" method="post">
    <label for="threshold">Threshold:</label>
    <input type="text" id="threshold" name="threshold">
    <input type="submit" value="Submit">
  </form>

  <h2>Current Threshold: <span id="currentThreshold"></span></h2>
  <h2>Current Humidity: <span id="currentHumidity"></span></h2>
  <h2>Current Temperature: <span id="currentTemperature"></span></h2>

  <script>
    function fetchCurrentData() {
      fetch('/getCurrentData')
      .then(response => response.json())
      .then(data => {
        document.getElementById('currentThreshold').innerText = data.threshold;
        document.getElementById('currentHumidity').innerText = data.humidity;
        document.getElementById('currentTemperature').innerText = data.temperature;
      });
    }
    setInterval(fetchCurrentData, 2000); // Fetch current data every 2 seconds
    window.onload = fetchCurrentData;
  </script>
</body>
</html>
)rawliteral";

void readings() {
  humidity = dht.readHumidity();
  Temp = dht.readTemperature();
  TempF = dht.readTemperature(true);

  //clearing the screen for every reading
  oled.clearDisplay();
  oled.setCursor(25, 20);
  oled.printf("Temp : %.1fC\n", Temp);
  oled.setCursor(20, 30);
  oled.printf("Humidity : %.1f", humidity);
  oled.display();
}

void handleRoot() {
  //Home Page
  server.send(200, "text/html", htmlPage);
}

void handleSetThreshold() {
  //Taking input from the form and projecting the given value in next page
  if (server.hasArg("threshold")) {
    threshold = server.arg("threshold").toFloat();
    server.send(200, "text/html", "Threshold set to: " + String(threshold) + "<br><a href='/'>Back</a>");
  } else {
    server.send(200, "text/html", "Invalid Input<br><a href='/'>Back</a>");
  }
}

void handleGetCurrentData() {
  //Streams the Live Data
  String json = "{\"threshold\":" + String(threshold) + ",\"humidity\":" + String(humidity) + ",\"temperature\":" + String(Temp) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  // Serialport for monitoring the data
  Serial.begin(115200);
  pinMode(pin, INPUT);
  pinMode(buzz, OUTPUT);
  pinMode(buton, INPUT);
  

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
  }

  delay(2000);         // wait for initializing
  oled.clearDisplay(); // clear display

  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(32, 10);
  oled.println("Hey Mani!!!");
  oled.display();
  delay(2000);
  oled.println("Check out Today's Weather!!");
  oled.display();
  delay(1000);

  //Commands to connect to a network
  dht.begin();
  WiFi.begin(SSID, passwd);

  //tries for unlimited times with delay of 500ms
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  
  server.on("/", handleRoot);
  server.on("/setThreshold", HTTP_POST, handleSetThreshold);
  server.on("/getCurrentData", HTTP_GET, handleGetCurrentData);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  readings();
  // here swtch is a variable to decide the buzzer's availability
  // if swtch==1 means it is blocked else buzzer can sound
  if ((humidity > threshold) && (swtch == 0)) {
    tone(buzz, 100);
  }
  flag = digitalRead(buton);
  if (flag) {
    Serial.println("Pressed");
    noTone(buzz);
    swtch = 1;
  }
  if (humidity < threshold) {
    swtch = 0;
    noTone(buzz);
  }
  delay(200);
}
