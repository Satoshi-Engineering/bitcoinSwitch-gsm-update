#ifndef Config_h
#define Config_h

#include <Arduino.h>

class Config {
  private:
    int portalPin;

  public:
    struct KeyValue {
      String key;
      String value;
    };

    struct Data {
      String devicePassword;
      String ssid;
      String wifiPassword;
      String serverFull;
      String lnbitsServer;
      String deviceId;
      String lnurl;
    };

    Config(int _portalPin);

    void init();
    void checkForConfigMode(int m_sec);

    Data getData();
    void configOverSerialPort();
    void executeCommand(String commandName, String commandData);
    void removeFile(String path);
    void appendToFile(String path, String data);
    void readFile(String path);

    KeyValue extractKeyValue(String s);
};

#endif