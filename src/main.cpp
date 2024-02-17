#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#define TUNED_PIN 10
#define HOLD_PIN 11
#define RESET_PIN 12

bool tuned = false;
bool hold = false;
bool reseet = false;

// Web server running on port 80
WebServer server(80);

StaticJsonDocument<1024> jsonDocument;

char buffer[1024];

void createJson(char *name, float value, char *unit) {  
  jsonDocument.clear();
  jsonDocument["name"] = name;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);  
}
 
void addJsonObject(char *name, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["name"] = name;
  obj["value"] = value;
  obj["unit"] = unit; 
}


void getTuned() {
  Serial.println("Get all values");
  jsonDocument.clear(); // Clear json buffer
  addJsonObject("tuned", tuned, "boolean");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void handleReset() {
  digitalWrite(RESET_PIN, HIGH);

  getTuned();
}

void handleHold() {
  digitalWrite(HOLD_PIN, HIGH);

  getTuned();
}

void setupApi() {
  server.on("/is_tuned", getTuned);
  server.on("/reset", HTTP_POST, handleReset);
  server.on("/hold", HTTP_POST, handleHold);
 
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

  setupApi();

  pinMode(RESET_PIN, OUTPUT);
  pinMode(HOLD_PIN, OUTPUT);
  pinMode(TUNED_PIN, INPUT_PULLDOWN);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

}