#include "Display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#include "SateLogo.h"

Display::Display(uint16_t width_, uint16_t height_, int8_t cs_pin_, int8_t dc_pin_, int8_t mosi_pin_, int8_t sclk_pin_, int8_t rst_pin_) : tft(width_, height_, cs_pin_, dc_pin_, mosi_pin_, sclk_pin_, rst_pin_){

}

void Display::setup() {
  tft.begin();
  tft.setTextSize(1);
}

void Display::clear() {
  tft.fillScreen(BLACK);
  tft.drawRGBBitmap(0, 0, SateLogo, 128, 128);

  line = 2;
}

void Display::drawLine(String text, uint16_t color) {
  tft.setCursor(5, line);
  tft.setTextColor(color);
  tft.print((char *) text.c_str());

  line += 10;
}