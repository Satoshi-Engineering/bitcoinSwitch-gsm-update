#define LED_BUILTIN 13

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  Serial.println("Minimal Sketch");

  bitcoinSwitchRelayTest();

  delay(15000);
}

void blink() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(250);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(5000);                      // wait for a second
}

void bitcoinSwitchRelayTest() {
  String payloadStr = "12-1000";

  pinMode(getValue(payloadStr, '-', 0).toInt(), OUTPUT);
  digitalWrite(getValue(payloadStr, '-', 0).toInt(), HIGH);
  delay(getValue(payloadStr, '-', 1).toInt());
  digitalWrite(getValue(payloadStr, '-', 0).toInt(), LOW);  
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