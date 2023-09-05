#include "QRCode_SSD1351.h"

#include "qrencode.h"

QRCode_SSD1351::QRCode_SSD1351(Adafruit_SSD1351 *display) {
  this->display = display;
}

void QRCode_SSD1351::init() {
  this->init(128, 128);
}


void QRCode_SSD1351::init(int width, int height) {
  this->screenwidth = width;
  this->screenheight = height;

  int min = screenwidth;

  if (screenheight < screenwidth)
      min = screenheight;

  multiply = min/WD;
  offsetsX = (screenwidth - (WD*multiply))/2;
  offsetsY = (screenheight - (WD*multiply))/2;
}

void QRCode_SSD1351::drawPixel(int x, int y, int color) {
    uint16_t thecolor = color == 1 ? BLACK : WHITE;

    this->display->drawPixel(x, y, thecolor);
    if (this->multiply>1) {
        this->display->drawPixel(x + 1, y, thecolor);
        this->display->drawPixel(x + 1, y + 1, thecolor);
        this->display->drawPixel(x, y + 1, thecolor);
    }
}

void QRCode_SSD1351::screenwhite() {
 this->display->fillScreen(WHITE);
}

void QRCode_SSD1351::screenupdate() {
  // NOT USED
}