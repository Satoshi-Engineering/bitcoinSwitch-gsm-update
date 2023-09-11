/*
  Source Based on 
  - Rui Santos Work https://github.com/arduino-libraries/ArduinoHttpClient/blob/master/examples/SimpleWebSocket/SimpleWebSocket.ino
  Rui Santos Complete project details at https://RandomNerdTutorials.com/esp32-sim800l-publish-data-to-cloud/
*/

// Bitcoin Swtich
#define PORTAL_PIN 2

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Display
//#define SCREEN_WIDTH  128
//#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 240  ///< ILI9341 max TFT width
#define SCREEN_HEIGHT 320 ///< ILI9341 max TFT height

#define MOSI_PIN 14
#define SCLK_PIN 18
#define DC_PIN   33
#define CS_PIN   32
#define RST_PIN  25

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT Serial1

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

#include <Wire.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include "Config.h"
#include "Display.h"

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

// I2C for SIM800 (to keep it running when powered from battery)
TwoWire I2CPower = TwoWire(0);

// TinyGSM Client for Internet connection
TinyGsmClient gsmclient(modem);
WebSocketClient wsClient(gsmclient, "", 80);

#define WEBSOCKET_LOOP_DELAY 250

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

bool setPowerBoostKeepOn(int en){
  I2CPower.beginTransmission(IP5306_ADDR);
  I2CPower.write(IP5306_REG_SYS_CTL0);
  if (en) {
    I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    I2CPower.write(0x35); // 0x37 is default reg value
  }
  return I2CPower.endTransmission() == 0;
}

#define SATE_BACKGROUND -1

Display display = Display(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);

Config config(PORTAL_PIN);
Config::Data configData;

void setup() {
  // Set serial monitor debugging window baud rate to 115200
  SerialMon.begin(115200);
  delay(250);
  SerialMon.println("-------------> SETUP <-------------");

  // Display
  display.setup();

  // Start Setup
  display.clear(0);
  display.clear(SATE_BACKGROUND);
  display.drawLine("Setup");
  delay(3000);

  // read config
  display.drawLine("Read Config");
  SerialMon.println("Read Config");

  config.init();
  configData = config.getData();

  bool foundConfig = configData.serverFull.length() > 10;

  // Check if config exists
  if (!foundConfig) {
    SerialMon.println(" - Empty");
    display.drawLine("Empty", RED);

    Serial.println("USB Config triggered");
    display.drawLine("Config Mode");
    display.drawLine("YES", GREEN);
    
    config.configOverSerialPort();    
  } else {
    display.drawLine("Found", GREEN);
  }

  // Config Websocket
  wsClient = WebSocketClient(gsmclient, configData.lnbitsServer.c_str(), configData.serverPort);

  // Check if config mode is triggered
  display.drawLine("Config Mode?");
  bool triggerConfig = config.checkForConfigMode(2000);
  if (triggerConfig) {
    Serial.println("USB Config triggered");
    display.drawLine("YES", GREEN);
    config.configOverSerialPort();
  }
  display.drawLine("NO", YELLOW);

  // Bitcoin Switch - Pre set the possible pins
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW);

  // Start I2C communication
  I2CPower.begin(I2C_SDA, I2C_SCL, 400000);

  // Keep power when running from battery
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  display.drawLine(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);

  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  display.drawLine("Init Modem");
  modem.restart();
  // use modem.init() if you don't need the complete restart
  delay(3000);

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);
  display.drawLine("Modem Info:");
  display.drawLine(modemInfo);

  // Unlock your SIM card with a PIN if needed
  if (configData.gsmPIN.length() > 0 && modem.getSimStatus() != 3 ) {
    modem.simUnlock(configData.gsmPIN.c_str());
  }
}

bool allConnectionsAlive = false;

unsigned long startTime;

bool firstLoopDraw = true;

void loop() {
  SerialMon.println("-------------> LOOP <-------------");
  display.clear(SATE_BACKGROUND);

  if (firstLoopDraw) {
    firstLoopDraw = false;
    display.drawLine("Start");
  } else {
    display.clear(SATE_BACKGROUND);
    display.drawLine("Reconnect ...");
    SerialMon.println("Reconnect ...");
    display.drawLine("Last Run:");
    display.drawLine(millitsToReadable(millis() - startTime), YELLOW);
  }
  startTime = millis();
  display.updateSignalStrength(modem.getSignalQuality());

  // ----------- CHECK: GPRS
  if (modem.isGprsConnected()) { 
      SerialMon.println("GPRS connected"); 
      display.drawLine("GPRS connected");
  } else {
    SerialMon.print("GPRS connecting to ");
    SerialMon.print(configData.gsmAPN);
    display.drawLine("GPRS connecting to");

    if (!modem.gprsConnect(configData.gsmAPN.c_str(), configData.gsmGPRSUser.c_str(), configData.gsmGPRSPass.c_str())) {
      SerialMon.println(" - FAIL");
      display.drawLine("FAIL", RED);

      SerialMon.println(("Retry in 2 sec"));
      delay(2000);
      return;
    } 

    SerialMon.println(" - OK");
    display.drawLine(configData.gsmAPN, GREEN);
  }

  SerialMon.print("Operator: ");
  SerialMon.println(modem.getOperator());

  display.updateSignalStrength(modem.getSignalQuality());

  // ----------- CHECK: Websocket
  if (wsClient.connected()) {
    SerialMon.println("WebSocket connected");
    display.drawLine("WebSocket connected");
  } else {
    SerialMon.print("WebSocket connecting: ");
    display.drawLine("WebSocket connecting");

    bool success = wsClient.begin("/api/v1/ws/" + configData.deviceId) == 0; // 0 if successful, else error
    
    if (!success) {
      display.drawLine("FAIL", RED);
      SerialMon.println("FAIL");

      SerialMon.println("Retry in 2 sec");
      wsClient.stop();
      delay(2000);
      return;
    }

    SerialMon.println("OK");
    display.drawLine("OK", GREEN);
  }

  display.updateSignalStrength(modem.getSignalQuality());

  // Sending Websocket Text connected
  SerialMon.println("Sending Connected");
  display.drawLine("Sending Connected");

  wsClient.beginMessage(TYPE_TEXT);
  wsClient.print("Connected");
  wsClient.endMessage();

  display.qrcode(configData.lnurl);
  display.updateSignalStrength(modem.getSignalQuality());

  startTime = millis();
  allConnectionsAlive = true;

  unsigned long lnurlPauseStart = 0;
  bool lnurlPauseActive = false;

  while (allConnectionsAlive) {
    // Check Websocket
    int messageSize = wsClient.parseMessage();
    int messageType = wsClient.messageType();

    if (messageType == TYPE_CONNECTION_CLOSE) {
      allConnectionsAlive = false;

      SerialMon.println("WebSocket Closed");
      display.warning("WebSoc");
      delay(2000);
      break;
    }

    // Websocket Recieve
    if (messageSize > 0) {
      if (messageType == TYPE_TEXT) {
        String payloadStr = wsClient.readString();
        SerialMon.println("Received Text:");
        SerialMon.println(payloadStr);

        if (!lnurlPauseActive) {
          display.payed(0);
  
          pinMode(getValue(payloadStr, '-', 0).toInt(), OUTPUT);
          digitalWrite(getValue(payloadStr, '-', 0).toInt(), HIGH);
          delay(getValue(payloadStr, '-', 1).toInt());
          digitalWrite(getValue(payloadStr, '-', 0).toInt(), LOW);

          display.payed(1);

          display.hardwareWaitingScreen();

          lnurlPauseStart = millis();
          lnurlPauseActive = true;

        } else {
          display.drawLine("Recieved Payment", RED);
          display.drawLine("Hardware not ready", RED);
        }
      }
    }

    // Application Wainting Time
    if (lnurlPauseActive && (millis() - lnurlPauseStart > configData.lnurlWaitingTime)) {
      lnurlPauseActive = false;
      display.qrcode(configData.lnurl);          
    }

    // Update GSM Connection Bars
    display.updateSignalStrength(modem.getSignalQuality());

    // Check Websocket Connection
    if (!wsClient.connected()) {
      allConnectionsAlive = false;

      SerialMon.println("HTTP Client disconnected");
      display.warning(" HTTP");
      delay(2000);
      break;      
    }

    // Check Internet Connection
    if (!modem.isGprsConnected()) {
      allConnectionsAlive = false;

      SerialMon.println("GPRS disconnected while on Websocket");
      display.warning(" GPRS");
      delay(2000);
      break;
    }

    // wait
    delay(WEBSOCKET_LOOP_DELAY);    
  }

  SerialMon.print("Server disconnected after ");
  SerialMon.println(millitsToReadable(millis() - startTime));
}

//////////////////HELPERS///////////////////
String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;
    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

String millitsToReadable(unsigned long currentMillis) {
  unsigned long seconds = currentMillis / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;

  seconds %= 60;
  minutes %= 60;
  hours %= 24;  

  return (days > 0 ? String(days) + " days " : "") + (hours < 10 ? "0" : "") + String(hours) + ":" + (minutes < 10 ? "0" : "") + String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds); 
}