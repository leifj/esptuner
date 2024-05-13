#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>


#define PWR_PIN 25
#define TUNED_PIN 33
#define HOLD_PIN 21
#define RESET_PIN 18

bool tuned = false;
bool hold = false;
bool on = false;

// Web server running on port 80
//WebServer server(80);
AsyncWebServer server(80);

void addBool(JsonDocument *doc, String name, bool value) {
  if (value) {
    (*doc)[name].set("true");
  } else {
    (*doc)[name].set("false");
  }
}

void getStatus(AsyncWebServerRequest *request) {
  StaticJsonDocument<1024> doc;
  Serial.println("Processing request...");
  tuned = digitalRead(TUNED_PIN);
  doc.clear(); // Clear json buffer
  addBool(&doc, String("tuned"), tuned);
  addBool(&doc, String("on"), on);
  addBool(&doc, String("hold"),hold);
  char buffer[1024];
  serializeJson(doc, buffer);
  
  request->send(200, "application/json", buffer);
}

void setupApi() {
  server.on("/api/is_tuned", HTTP_GET, [](AsyncWebServerRequest *request){
    getStatus(request);
  });
  server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String());
  });
  server.on("/style.css",HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", String());
  });
  server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(RESET_PIN, HIGH);
    sleep(1);
    digitalWrite(RESET_PIN, LOW);
    hold = false;
    getStatus(request);
  });
  server.on("/api/hold", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(HOLD_PIN, HIGH);
    hold = true;
    getStatus(request);
  });
  server.on("/api/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(PWR_PIN, HIGH);
    on = true;
    getStatus(request);
  });
  server.on("/api/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(PWR_PIN, LOW);
    on = false;
    getStatus(request);
  });
  // start server
  server.begin();
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 


  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // it is a good practice to make sure your code sets wifi mode how you want it.

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  // wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
      Serial.println("Failed to connect");
      ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("Connected...yeey :)");
  }

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  setupApi();

  pinMode(PWR_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(HOLD_PIN, OUTPUT);
  pinMode(TUNED_PIN, INPUT);
}

unsigned int previousMillis = 0;
unsigned const int interval = 30*1000;
void loop() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  delay(1000);
}