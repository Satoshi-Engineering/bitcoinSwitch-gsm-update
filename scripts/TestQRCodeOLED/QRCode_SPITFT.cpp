#include "QRCode_SPITFT.h"

#include "qrencode.h"

QRCode_SPITFT::QRCode_SPITFT(Adafruit_SPITFT *display) {
  this->display = display;
}

void QRCode_SPITFT::init() {
  this->init(128, 128);
}


void QRCode_SPITFT::init(int width, int height) {
  this->screenwidth = width;
  this->screenheight = height;

  int min = screenwidth;

  if (screenheight < screenwidth)
      min = screenheight;

  multiply = min/WD;
  offsetsX = (screenwidth - (WD*multiply))/2;
  offsetsY = (screenheight - (WD*multiply))/2;
}

void QRCode_SPITFT::drawPixel(int x, int y, int color) {
    uint16_t thecolor = color == 1 ? BLACK : WHITE;

    this->display->drawPixel(x, y, thecolor);
    if (this->multiply > 1) {
      this->display->fillRect(x, y, this->multiply, this->multiply, thecolor);
    }
}

void QRCode_SPITFT::screenwhite() {
 this->display->fillScreen(WHITE);
}

void QRCode_SPITFT::screenupdate() {
  // NOT USED
}