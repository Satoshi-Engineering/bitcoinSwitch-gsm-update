/*************************************************** 

 ****************************************************/


// Screen dimensions  1.27" OLED.
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 96

// Screen dimensions  1.8" OLED.
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Screen dimensions 2.5 TFT
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// You can use any (4 or) 5 pins 
/*
#define SCLK_PIN 18
#define MOSI_PIN 23
#define DC_PIN   16
#define CS_PIN   17
#define RST_PIN  5
*/

// T-Call
#define MOSI_PIN 14
#define SCLK_PIN 18
#define DC_PIN   33
#define CS_PIN   32
#define RST_PIN  25

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include "Adafruit_ILI9341.h"

#include <SPI.h>
#include "QRCode_SPITFT.h"

// Option 1: use any pins but a little slower
//Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  
Adafruit_ILI9341 tft = Adafruit_ILI9341(CS_PIN, DC_PIN, MOSI_PIN, SCLK_PIN, RST_PIN);  

// Option 2: must use the hardware SPI pins 
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be 
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
//Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

float p = 3.1415926;

String lnurl = "lightning:LNURL1DPNOTAVALIDLNURLRWVF5HGUEWW35X2UMSD9JKCURV131313NRDAKJ7MRWW4EXCER9WE5KXEF0V9NOTVALIDWFKZ12313123DFYGAMDWEN4ZCTR2F98X4FK8ACXJM3AXYEZVCTDDA6KUAPAXQHRQVP3YEJ82UNPW35K7M3AXYCRQVQLHQ3DP";

QRCode_SPITFT qrcode(&tft);

void setup(void) {
  Serial.begin(115200);
  Serial.print("hello!");

  tft.begin();

  Serial.println("init");

  tft.fillScreen(BLACK);

  delay(1000);

  // WARNING: Sample non valid LNURL
  // WARNING: Library is default on QR Code Version 7 which can be too less for LNURLs
  // WARNING: I had to rewrite the library code (I Know it's so bad) from QR Code version 7 to 10
  qrcode.init(240, 320);
  qrcode.create(lnurl);
  
  Serial.println("done");
  delay(1000);
}

int barGap = 2;
int barWidth = 5;
int barHeight = 5; //(SCREEN_HEIGHT - gap) / 4 - gap;
int barValue = 0;

uint16_t barColor = color565(32, 128, 0);

void loop() {
  
  for (int i = 0; i < 4; i++) {
    if (barValue > i) {
      tft.fillRect(SCREEN_WIDTH - barWidth - 1, SCREEN_HEIGHT - 1 - (i + 1) * (barHeight + barGap), barWidth, barHeight, barColor);
    } else {
      tft.fillRect(SCREEN_WIDTH - barWidth - 1, SCREEN_HEIGHT - 1 - (i + 1) * (barHeight + barGap), barWidth, barHeight, WHITE);
      tft.drawRect(SCREEN_WIDTH - barWidth - 1, SCREEN_HEIGHT - 1 - (i + 1) * (barHeight + barGap), barWidth, barHeight, barColor);
    }
  }

  delay(1000);
  barValue++;
  if (barValue >= 5) barValue = 0;
}

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}
