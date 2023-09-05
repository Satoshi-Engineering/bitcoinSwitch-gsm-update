/*
  Source BAsed on 

  https://github.com/arduino-libraries/ArduinoHttpClient/blob/master/examples/SimpleWebSocket/SimpleWebSocket.ino
  
  Rui Santos Complete project details at https://RandomNerdTutorials.com/esp32-sim800l-publish-data-to-cloud/
*/

// Bitcoin Swtich
#define PORTAL_PIN 4

#include "secrets.h"

// Your GPRS credentials (leave empty, if not needed)
const char apn[]      = GSM_APN; // APN (example: internet.vodafone.pt) use https://wiki.apnchanger.org
const char gprsUser[] = GSM_GPRS_USER; // GPRS User
const char gprsPass[] = GSM_GPRS_PASS; // GPRS Password
const char simPIN[]   = GSM_PIN;  // SIM card PIN (leave empty, if not defined)

// Server details
const char server[] = LNBITS_SERVER;  // server address
const char resource[] = LNBITS_PATH;

const int  port = LNBITS_PORT; // server port number
const char lnurl[] = LNBITS_LNURL;

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Display
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

#define SCLK_PIN 18
#define MOSI_PIN 14
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
WebSocketClient client = WebSocketClient(gsmclient, server, port);

#define uS_TO_S_FACTOR 1000000UL   /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* Time ESP32 will go to sleep (in seconds) 3600 seconds = 1 hour */

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

Config config = Config(PORTAL_PIN);
Config::Data configData;

void setup() {
  // Set serial monitor debugging window baud rate to 115200
  SerialMon.begin(115200);

  // Display
  display.setup();

  // Start Setup
  display.clear(SATE_BACKGROUND);

  display.drawLine("Setup");
  delay(3000);

  // read config
  config.init();
  configData = config.getData();
  display.drawLine("Config Mode?");
  //config.checkForConfigMode(2000);
  display.drawLine("Read Config");

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
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }

  // Configure the wake up source as timer wake up  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

int count = 0;
bool connectionClosed = false;

void loop() {
  display.clear(SATE_BACKGROUND);
  display.drawLine("Start");

  SerialMon.print("Connecting to APN: ");
  SerialMon.print(apn);
  display.drawLine("Connect to APN:");

  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" FAIL");
    display.drawLine("FAIL");
    delay(1000);
    return;
  } 

  SerialMon.println(" OK");
  display.drawLine(apn);

  // Wired code from Example
  if (modem.isGprsConnected()) { 
    SerialMon.println("GPRS connected"); 
  }

  //while (modem.isGprsConnected()) {
  while (modem.isGprsConnected()) {
    SerialMon.print("Connected to: ");
    SerialMon.println(modem.getOperator());

    SerialMon.print("Starting WebSocket Connection: ");
    display.drawLine("WebSocket:");

    int succ = client.begin(resource); // 0 if successful, else error
    SerialMon.println((succ == 0 ? "OK" : "ERROR"));
    if (succ != 0) {
      client.stop();
      SerialMon.println(("Retry in 1 sec"));
      display.drawLine("FAIL");
      delay(1000);
      continue;
    }

    display.drawLine("OK");
    SerialMon.println("Sending Connected");
    display.drawLine("Sending Connected");

    client.beginMessage(TYPE_TEXT);
    client.print("Connected");
    client.endMessage();

    display.qrcode(lnurl);

    while (client.connected() && !connectionClosed) {
      int messageSize = client.parseMessage();
      int messageType = client.messageType();
      bool isFinal = client.isFinal();

      if (messageType == TYPE_CONNECTION_CLOSE) {
        SerialMon.println("Connection Closed");
        connectionClosed = true;
        break;
      }

      if (messageSize > 0) {
        
        if (messageType == TYPE_TEXT) {
          SerialMon.println("Received Text:");
          SerialMon.println(client.readString());

          // TODO: Trigger Pin
          display.clear();
          display.payed();

          // Refactore for looping and state
          delay(5000);    
          display.qrcode(lnurl);          
        }
      }

      // Update GSM Connection Bars
      int16_t strength = modem.getSignalQuality();
      SerialMon.print("Signal Strengh: ");
      SerialMon.println(strength);

      // wait 2 seconds
      delay(250);    
    }

    // Close client and disconnect
    client.stop();

    SerialMon.println(F("Server disconnected"));
  }

  //modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));

  // Put ESP32 into deep sleep mode (with timer wake up)
  esp_deep_sleep_start();
}