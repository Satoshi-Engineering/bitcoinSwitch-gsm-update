#include "Display.h"

#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1351.h>
#include "Adafruit_ILI9341.h"
#include <SPI.h>
#include <QRCodeGFX.h>

#include "bitmaps/Logo48x48.h"
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
  tft.setRotation(2);
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
  int w = 36;
  int h = 64;
  int x = (tft.width() - w) * 0.5;
  int y = tft.height() * 0.5 - h - 10;

  if (part == 0) {
    tft.fillScreen(BLACK);
    tft.setTextSize(DEFAULT_TEXT_SIZE * 4);
    tft.setCursor(30, tft.height() * 0.5);
    tft.setTextColor(WHITE);
    tft.print("PAID");
    tft.setTextSize(DEFAULT_TEXT_SIZE);
    tft.drawRGBBitmap(x, y, LightningBoltBitmap, w, h);
  }

  if (part == 1) {
    // Flash  36x64px
    for (int i = 0; i < 4; i++) {
      delay(250);
      tft.fillRect(x, y, w, h, BLACK);
      delay(250);
    tft.drawRGBBitmap(x, y, LightningBoltBitmap, w, h);
    }
  }
}

void Display::qrcode(String text) {
  tft.fillScreen(WHITE);
  QRCodeGFX qrcode(tft);
  qrcode.setScale(4);
  qrcode.draw(text, 10, 50);

  tft.drawRGBBitmap((tft.width() - 48) / 2, 2, Logo48_WHITE, 48, 48);
  tft.setTextColor(BLACK);

  tft.setCursor(5, tft.height() - LINE_HEIGHT * 2);
  tft.println("Satoshi Engineering");
  tft.setCursor(5, tft.height() - LINE_HEIGHT);
  tft.println("We make IT easy!");
}

void Display::hardwareWaitingScreen() {
  clear(-1);
  tft.setTextColor(WHITE);

  tft.setCursor(5, tft.height() - LINE_HEIGHT * 4);
  tft.println("Satoshi Engineering");
  tft.setCursor(5, tft.height() - LINE_HEIGHT * 3);
  tft.println("We make IT easy!");
}

void Display::progressBar(float percent) {
  percent = max(percent, float(0.0));
  percent = min(percent, float(1.0));

  int gap = 2;
  int border = 2;

  int wBorder = tft.width() - 2 * gap;
  int hBorder = LINE_HEIGHT;
  int xBorder = gap;
  int yBorder = tft.height() - 1 - hBorder - gap;

  int wBar = wBorder - 2 * border;
  int hBar = hBorder - 2 * border;
  int xBar = gap + border;
  int yBar = yBorder + border;

  if (percent <= 0) {
    tft.fillRect(xBorder, yBorder, wBorder, hBorder, BLACK);
    tft.drawRect(xBorder, yBorder, wBorder, hBorder, WHITE);
    tft.drawRect(xBorder + 1, yBorder + 1, wBorder - 2, hBorder - 2, WHITE);

  } else {
    if (percent < 1.0) tft.fillRect(xBar + wBar * percent, yBar, wBar * (1.0 - percent), hBar, BLACK);
    tft.fillRect(xBar, yBar, wBar * percent, hBar, SATE_PURPLE);
  }
}

void Display::warning(String text) {
  int w = 64;
  int h = 64;
  int x = (tft.width() - w) * 0.5;
  int y = tft.height() * 0.5 - h - 10;

  tft.fillScreen(BLACK);
  tft.setTextSize(DEFAULT_TEXT_SIZE * 3);
  tft.setTextColor(WHITE);
  tft.setCursor(12, tft.height() * 0.5 + 30);
  tft.print((char *)text.c_str());
  tft.setTextSize(DEFAULT_TEXT_SIZE);
  tft.drawRGBBitmap(x, y, WarningBitmap, w, h);
}

void Display::updateSignalStrength(int16_t strength) {
  int barValue = SATE_PURPLE;
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

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}