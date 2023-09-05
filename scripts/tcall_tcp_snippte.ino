// Base: https://RandomNerdTutorials.com/esp32-sim800l-publish-data-to-cloud/
TinyGsmClient client(modem);

// ...

  SerialMon.println("starting WebSocket client");
  if (!client.connect(server, port)) {
    SerialMon.println(" fail");
  } else {
    SerialMon.println(" OK");
  }

  client.print(String("GET ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.println("Upgrade: websocket");
  client.println("Connection: Upgrade");
  client.println("Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==");
  client.println("Sec-WebSocket-Protocol: chat, superchat");
  client.println("Sec-WebSocket-Version: 13");
  client.println("Origin: http://example.com");
  client.println();

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data (HTTP response from server)
    while (client.available()) {
      char c = client.read();
      SerialMon.print(c);
      timeout = millis();
    }
  }
  SerialMon.println();

  // Close client and disconnect
  client.stop();
