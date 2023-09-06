#ifndef Display_h
#define Display_h

#include <Adafruit_SSD1351.h>

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

class Display {
  private:
    Adafruit_SSD1351 tft;
    int line = 0;
    uint16_t width = 0;
    uint16_t height = 0;

    uint16_t barColor = 0;
    int barGap = 2;
    int barWidth = 5;
    int barHeight = 5;      

  public:
    Display(uint16_t width_, uint16_t height_, int8_t cs_pin_, int8_t dc_pin_, int8_t mosi_pin_, int8_t sclk_pin_, int8_t rst_pin_);

    void setup();
    void clear(int background = 0);
    void drawLine(String text, uint16_t color = WHITE);
    void qrcode(String text);
    void payed();
    void updateSignalStrength(int16_t strength);

};

#endif