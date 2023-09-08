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
      int serverPort;
      String deviceId;
      String lnurl;
      int lnurlWaitingTime;

      String gsmPIN;
      String gsmAPN;
      String gsmGPRSUser;
      String gsmGPRSPass;
    };

    Config(int _portalPin);

    void init();
    bool checkForConfigMode(int m_sec);

    Data getData();
    void configOverSerialPort();
    void executeCommand(String commandName, String commandData);

    void deleteFile(String path);
    void appendOrCreateFile(String path, String data);

    void readFile(String path);

    KeyValue extractKeyValue(String s);
};

#endif