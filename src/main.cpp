#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "ota.h"
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "si7006.h"


#include "index.h" //Our HTML webpage contents
#include "setup.h"
#include "reset.h"
#include "clear.h"
#include "reboot.h"
#include "setting.h"

void launchWeb(int webtype);
void handleRoot(void);
void handleSetup(void);
void handleWifiSsids(void);
void handleReset(void);
void handleReboot(void);
void handleClearMem(void);
void handleSetting(void);
void handleData(void);

//===============================================================
//  Global Variables
//===============================================================
const char* ssid = "MC_S1"; //AP SSID
const char* passphrase = "12345678"; //AP Password

String wifiScanResult = "";
double currentTemperature = 0;
double currentRH = 0;
int measureInterval = 20 ; //*100 ms;
int timer1 = 0;
ESP8266WebServer server(80);

//===============================================================
//  Initial startup functions - used to connect to known wifi or 
//  create its own AP
//===============================================================
bool testWifi(void) {
  int c = 0;
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; }
    delay(500);
    c++;
  }
  return false;
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  String SSISstr = "\"SSID\":[";
  String RSSIstr = "\"RSSI\":[";
  String Protected = "\"Protected\":[";
  for (int i = 0; i < n; ++i)
    {
      if(i != 0) {
        RSSIstr += ", ";
        SSISstr += ", ";
        Protected += ", ";
      }
      RSSIstr += (WiFi.RSSI(i));
      SSISstr += "\""+ WiFi.SSID(i) + "\"";
      Protected += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?"0":"1";
    }
  RSSIstr += "]";
  SSISstr += "]";
  Protected += "]";
  wifiScanResult = "{ " + RSSIstr +", "+ SSISstr + ", " + Protected +" }";
  delay(100);
  WiFi.softAP(ssid, passphrase, 6, 0);
  launchWeb(1);
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    server.on("/", handleSetup);
    server.on("/wifissids", handleWifiSsids);
    server.on("/direct", handleRoot);
    server.on("/reset", handleReset);
    server.on("/reboot", handleReboot);
    server.on("/clearmem", handleClearMem);
    server.on("/setting", handleSetting);
    server.on("/data", handleData);
  } else if (webtype == 0) {
    server.on("/", handleRoot);
    server.on("/reset", handleReset);
    server.on("/clearmem", handleClearMem);
    server.on("/reboot", handleReboot);
    server.on("/data", handleData);
  }
}

void launchWeb(int webtype) {
  createWebServer(webtype);
  server.begin();
}

void init_first_boot()
{
  EEPROM.begin(512);
  delay(10);
  String esid;
  for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  String epass = "";
  for (int i = 32; i < 96; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  if ( esid.length() > 1 ) {
      WiFi.mode(WIFI_STA);
      delay(200);
      WiFi.begin(esid.c_str(), epass.c_str());
      if (testWifi()) {
        launchWeb(0);
        return;
      }
  }
  
  if (!testWifi()) {
    setupAP();
  }
}

//===============================================================
//  Routines are executed when you open its IP in a browser
//===============================================================
void handleSetting(){
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");
  if (qsid.length() > 0 && qpass.length() > 0) {
    for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
    for (int i = 0; i < qsid.length(); ++i){
      EEPROM.write(i, qsid[i]);
    }
    for (int i = 0; i < qpass.length(); ++i){
      EEPROM.write(32+i, qpass[i]);
    }
    EEPROM.commit();
    String s = SETTING_page;  
    server.send(200, "text/html", s);
  } else {
    server.send(404, "application/json", "{\"Error\":\"404 not found\"}");
  }
  delay(2000);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
}
void handleReboot(){
  String s = REBOOT_page;             //Read HTML contents
  server.send(200, "text/html", s);
  delay(2000);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
}

void handleClearMem(){
  String s = CLEAR_page;             //Read HTML contents from reboot.h file
  server.send(200, "text/html", s);
  for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
  EEPROM.commit();
  delay(2000);
  WiFi.disconnect();
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
}

void handleReset(){
  String s = RESET_page;             //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

String getSsidScan(){
  return wifiScanResult;
}

void handleWifiSsids(void){
  server.send(200, "text/json", getSsidScan());
}

void handleSetup(){
  String s = SETUP_page;             //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleRoot()
{
  String s = MAIN_page;             //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleData(){
  server.send(200, "text/json", "{\"temp\":" + String(currentTemperature,1) + ",\"rh\":" + String(currentRH,1) + "}");
}


//==============================================================
//                  SETUP
//==============================================================
void setup()
{

  Serial.begin(9600); 
  init_first_boot();
  OtaSetup();

  delay(500);
  Wire.begin(4,5); // (sda,scl)
  Wire.setClock(100000);

  //server.begin();

  currentRH = TemperatureHumiditySensor.measureRelativeHumidity();
  currentTemperature = TemperatureHumiditySensor.readTemperature();
  
}

//==============================================================
//                     LOOP
//==============================================================
void loop()
{
  OtaHandleRequests();

  server.handleClient();
  delay(100);
  timer1++;
  if(timer1 >= measureInterval)
  {
    timer1 = 0;
    currentRH = TemperatureHumiditySensor.measureRelativeHumidity();
    currentTemperature = TemperatureHumiditySensor.readTemperature();
  }
}
