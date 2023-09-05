#ifndef QRCODE_SSD1351_H
#define QRCODE_SSD1351_H

#include <Arduino.h>
#include <Adafruit_SSD1351.h>
#include "qrcodedisplay.h"

#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

class QRCode_SSD1351 : public QRcodeDisplay {
	private:
    Adafruit_SSD1351 *display;
    void drawPixel(int x, int y, int color);

	public:
		QRCode_SSD1351(Adafruit_SSD1351 *display);

		void init();
    void init(int width, int height);
		void screenwhite();
		void screenupdate();    
};

#endif