#include "Display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include "QRCode_SSD1351.h"

#include "SateLogo.h"
#include "LightningBolt.h"

Display::Display(uint16_t width_, uint16_t height_, int8_t cs_pin_, int8_t dc_pin_, int8_t mosi_pin_, int8_t sclk_pin_, int8_t rst_pin_) : tft(width_, height_, cs_pin_, dc_pin_, mosi_pin_, sclk_pin_, rst_pin_){
  this->width = width_;
  this->height = height_;
}

void Display::setup() {
  tft.begin();
  tft.setTextSize(1);
}

void Display::clear(int background) {
  switch (background) {
    case -1:
      tft.drawRGBBitmap(0, 0, SateLogo, 128, 128);
      break;

    default:
      tft.fillScreen(background);
      break;
  }

  line = 2;
}

void Display::drawLine(String text, uint16_t color) {
  tft.setCursor(5, line);
  tft.setTextColor(color);
  tft.print((char *) text.c_str());

  line += 10;
}

void Display::payed() {
  tft.setTextSize(4);
  tft.setCursor(8, this->height * 0.5 + 30);
  tft.print("PAYED");
  tft.setTextSize(1);

  // Flash  36x64px
  for (int i = 0; i < 4; i++) {
    delay(250);
    tft.fillRect(46, 10, 36, 64, BLACK);
    delay(250);
    tft.drawRGBBitmap(46, 10, LightningBolt, 36, 64);
  }
/*
  for (int y = 0; y < 70; y+=5) {
    tft.drawRGBBitmap(32, y - 60, LightningBolt, 64, 64);
    delay(10);
    tft.fillRect(32, y - 60, 64, 64, BLACK);
  }
  */
}

void Display::updateSignalStrength(int16_t strength) {

}

void Display::qrcode(String text) {
  QRCode_SSD1351 qrcode(&tft);  
  qrcode.init();
  qrcode.create(text);
}