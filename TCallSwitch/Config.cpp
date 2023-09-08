#include "Config.h"

/* What happend to SPIFFS?
  SPIFFS is currently deprecated and may be removed in future releases of the core. Please consider moving your code to LittleFS.
  SPIFFS is not actively supported anymore by the upstream developer, while LittleFS is under active development,
  supports real directories, and is many times faster for most operations.
  https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
*/

#include <ArduinoJson.h>

#include "FS.h"
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

fs::LittleFSFS &FlashFS = LittleFS;

#define PARAM_FILE "/elements.json"

Config::Config(int _portalPin) : portalPin(_portalPin) {
}

void Config::init() {
  if(!FlashFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
    Serial.println("An Error has occurred while mounting LittleFS");
  }
}

bool Config::checkForConfigMode(int m_sec) {
  int timer = 0;
  bool triggerConfig = false;

  Serial.print("Waiting ");
  Serial.print(m_sec);
  Serial.println("msec for config mode trigger");
  Serial.print("Portal Pin (<40): ");

  while (timer < m_sec) {
    Serial.println(touchRead(portalPin));
    if (touchRead(portalPin) < 40) {
      Serial.println("Launch portal");
      triggerConfig = true;
      timer = m_sec;
    }
    timer = timer + 250;
    delay(250);
  }
  return triggerConfig;
}

void Config::configOverSerialPort() {
  while (true) {
    if (Serial.available() == 0) continue;
    String data = Serial.readStringUntil('\n');
    Serial.println("received: " + data);
    KeyValue kv = extractKeyValue(data);
    String commandName = kv.key;
    if (commandName == "/config-done") {
      Serial.println("/config-done");
      return;
    }
    executeCommand(commandName, kv.value);
  }
}

void Config::executeCommand(String commandName, String commandData) {
  Serial.println("executeCommand: " + commandName + " > " + commandData);
  KeyValue kv = extractKeyValue(commandData);
  String path = kv.key;
  String data = kv.value;

  if (commandName == "/file-remove") {
    return deleteFile("/" + path);
  }
  if (commandName == "/file-append") {
    return appendOrCreateFile("/" + path, data);
  }

  if (commandName == "/file-read") {
    Serial.println("prepare to read");
    readFile("/" + path);
    Serial.println("readFile done");
    return;
  }

  Serial.println("command unknown");
}

void Config::deleteFile(String path) {
  Serial.printf("Deleting file: %s\r\n", path);
    if (FlashFS.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

void Config::appendOrCreateFile(String path, String data) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = FlashFS.open(path, FILE_APPEND);
  if (!file) {
    file = FlashFS.open(path, FILE_WRITE);
    Serial.printf("- failed to open file for appending - File created");
  }
  if (file.print(data)) {
      Serial.println("- data appended");
  } else {
      Serial.println("- append failed");
  }
  file.close();
}

void Config::readFile(String path) {
  //Serial.printf("Reading file: %s\r\n", path);
  Serial.println("readFile: " + path);

  File file = FlashFS.open(path);
  if (!file || file.isDirectory()){
      // Serial.println("- failed to open file for reading");
      Serial.println("");
      Serial.println("/file-done");  
      return;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println("/file-read " + line);
  }
  file.close();
  Serial.println("");
  Serial.println("/file-done");  
}  

Config::KeyValue Config::extractKeyValue(String s) {
  int spacePos = s.indexOf(" ");
  String key = s.substring(0, spacePos);
  if (spacePos == -1) {
    return { key, "" };
  }
  String value = s.substring(spacePos + 1, s.length());
  return { key, value };
}

Config::Data Config::getData() {
  Config::Data data;

  File paramFile = FlashFS.open(PARAM_FILE, "r");
  if (paramFile) {
    StaticJsonDocument<2500> doc;
    DeserializationError error = deserializeJson(doc, paramFile.readString());
    
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return data;
    }

    if (!doc.is<JsonArray>()) {
      Serial.print("JSON reading ERROR. Root element is not a array!");
      return data;
    }

    JsonArray configArray = doc.as<JsonArray>();

    for (unsigned short i = 0; i < configArray.size(); ++i) {
      const JsonObject configObject = configArray[i];
      String name = String((const char *)configObject["name"]);
      String value = String((const char *)configObject["value"]);

      // Debug only
      // Serial.print(name + " | ");
      // Serial.println(value);

      // Password
      if (name == "password") data.devicePassword = value;
      
      // Wifi
      if (name == "ssid") data.ssid = value;
      if (name == "wifipassword") data.wifiPassword = value;

      // LNBits
      if (name == "socket") {
        data.serverFull = value;
        data.lnbitsServer = data.serverFull.substring(5, data.serverFull.length() - 33);
        data.deviceId = data.serverFull.substring(data.serverFull.length() - 22); 
        data.serverPort = 443; // Default to wss
        int indexOfColon = data.lnbitsServer.indexOf(":");
        if (indexOfColon >= 0) {
          data.serverPort = data.lnbitsServer.substring(indexOfColon + 1).toInt();
          data.lnbitsServer = data.lnbitsServer.substring(0, indexOfColon);
        }
      }
      if (name == "lnurl") data.lnurl = value;
      if (name == "lnurlWaitingTime") data.lnurlWaitingTime = value.toInt();
 
      // GSM
      if (name == "gsmPIN") data.gsmPIN = value;
      if (name == "gsmAPN") data.gsmAPN = value;
      if (name == "gsmGPRSUser") data.gsmGPRSUser = value;
      if (name == "gsmGPRSPassword") data.gsmGPRSPass = value;
    }

    Serial.println("Device Password: " + data.devicePassword);
    Serial.println("SSID: " + data.ssid);
    Serial.println("Wifi Password: " + data.wifiPassword);

    Serial.println("Server Full: " + data.serverFull);
    Serial.println("Server: " + data.lnbitsServer);
    Serial.print("Port: ");
    Serial.println(data.serverPort);
    Serial.println("DeviceId (or Resource): " + data.deviceId);
    Serial.println("LNURL: " + data.lnurl);
    Serial.print("Waitning time: ");
    Serial.print(data.lnurlWaitingTime);
    Serial.println("ms");

    Serial.println("GSM PIN: " + data.gsmPIN);
    Serial.println("GSM APN: " + data.gsmAPN);
    Serial.println("GSM GPRS User: " + data.gsmGPRSUser);
    Serial.println("GSM GPRS Password: " + data.gsmGPRSPass);

  } else {
    Serial.println("No Configfile");
  }

  paramFile.close();
  return data;
}