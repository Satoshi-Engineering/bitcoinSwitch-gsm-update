// Base: https://RandomNerdTutorials.com/esp32-sim800l-publish-dat== a-to-cloud/
// Library: https://github.com/arduino-libraries/ArduinoHttpClient

// TinyGSM Client for Internet connection
TinyGsmClient gsmclient(modem);
WebSocketClient client = WebSocketClient(gsmclient, server, port);

// ... 

 client.begin();
  SerialMon.println("Sending hello ");

  client.beginMessage(TYPE_TEXT);
  client.print("Connected");
  client.endMessage();

  while (client.connected()) {
    // send a hello #
    SerialMon.println("Sending hello ");
    client.beginMessage(TYPE_TEXT);
    client.print("Count: ");
    client.print(count);
    client.endMessage();
  
    // increment count for next message
    count++;

    // check if a message is available to be received
    int messageSize = client.parseMessage();
    int messageType = client.messageType();

    if (messageType == TYPE_TEXT) SerialMon.println("Received Text:");
    if (messageSize > 0) {
      Serial.println(client.readString());
    }

    // wait 5 seconds
    delay(5000);    
  }

  SerialMon.println();

  // Close client and disconnect
  client.stop();
