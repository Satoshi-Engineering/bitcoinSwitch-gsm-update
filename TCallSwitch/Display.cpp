#include "Display.h"

#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1351.h>
#include "Adafruit_ILI9341.h"
#include <SPI.h>
#include "QRCode_SPITFT.h"

#include "bitmaps/Logo240x240.h"
#include "bitmaps/LightningBolt.h"
#include "bitmaps/Warning.h"

#define LINE_HEIGHT 20
#define DEFAULT_TEXT_SIZE 2

Display::Display(uint16_t width_, uint16_t height_, int8_t cs_pin_, int8_t dc_pin_, int8_t mosi_pin_, int8_t sclk_pin_, int8_t rst_pin_)
    : tft(cs_pin_, dc_pin_, mosi_pin_, sclk_pin_, rst_pin_) {
//  : tft(width_, height_, cs_pin_, dc_pin_, mosi_pin_, sclk_pin_, rst_pin_) {
  this->width = width_;
  this->height = height_;

  this->barColor = tft.color565(32, 128, 0);
  this->barGap = 2;
  this->barWidth = 5;
  this->barHeight = 5;  //(this->height - barGap) / 4 - barGap;
}

void Display::setup() {
  tft.begin();
  tft.setTextSize(DEFAULT_TEXT_SIZE);
}

void Display::clear(int background) {
  switch (background) {
    case -1:
      tft.fillScreen(BLACK);
      tft.drawRGBBitmap(0, 0, LogoBitmap, 240, 240);
      break;

    default:
      tft.fillScreen(background);
      break;
  }

  line = 0;
}

void Display::drawLine(String text, uint16_t color) {
  tft.setCursor(5, 2 + line * LINE_HEIGHT);
  tft.setTextColor(color);
  tft.print((char *)text.c_str());

  line++;

  // Display Height reached
  if (line >= lineMax) {
    clear(-1);
  }
}

void Display::payed(int part) {
  if (part == 0) {
    tft.fillScreen(BLACK);
    tft.setTextSize(DEFAULT_TEXT_SIZE * 4);
    tft.setCursor(15, tft.height() * 0.5 + 30);
    tft.print("PAID");
    tft.setTextSize(DEFAULT_TEXT_SIZE);
    tft.drawRGBBitmap((tft.width() - 36) / 2 , 10, LightningBoltBitmap, 36, 64);
  }

  if (part == 1) {
    // Flash  36x64px
    for (int i = 0; i < 4; i++) {
      delay(250);
      tft.fillRect(46, 10, 36, 64, BLACK);
      delay(250);
      tft.drawRGBBitmap((tft.width() - 36), 10, LightningBoltBitmap, 36, 64);
    }
  }
}

void Display::hardwareWaitingScreen() {
  clear(-1);
  tft.setCursor(5, tft.height() - LINE_HEIGHT * 2);
  tft.setTextColor(WHITE);
  tft.println("Satoshi Engineering");
  tft.println("We make IT easy!");
}

void Display::warning(String text) {
  tft.fillScreen(BLACK);
  tft.setTextSize(DEFAULT_TEXT_SIZE * 3);
  tft.setCursor(8, this->height * 0.5 + 30);
  tft.print((char *)text.c_str());
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.drawRGBBitmap(32, 5, WarningBitmap, 64, 64);
}

void Display::updateSignalStrength(int16_t strength) {
  int barValue = 0;
  if (strength >= 2) barValue = 1;
  if (strength >= 10) barValue = 2;
  if (strength >= 15) barValue = 3;
  if (strength >= 20) barValue = 3;

  for (int i = 0; i < 4; i++) {
    if (barValue > i) {
      tft.fillRect(this->width - this->barWidth - 1, this->height - 1 - (i + 1) * (this->barHeight + this->barGap), this->barWidth, this->barHeight, this->barColor);
    } else {
      tft.fillRect(this->width - this->barWidth - 1, this->height - 1 - (i + 1) * (this->barHeight + this->barGap), this->barWidth, this->barHeight, WHITE);
      tft.drawRect(this->width - this->barWidth - 1, this->height - 1 - (i + 1) * (this->barHeight + this->barGap), this->barWidth, this->barHeight, this->barColor);
    }
  }
}

void Display::qrcode(String text) {
  QRCode_SPITFT qrcode(&tft);
  qrcode.init(tft.width(), tft.height());
  qrcode.create(text);

  tft.setCursor(5, tft.height() - LINE_HEIGHT * 2);
  tft.setTextColor(BLACK);
  tft.println("Satoshi Engineering");
  tft.println("We make IT easy!");
}

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}