# Bitcoin Switch with Gsm and OLED Display
_by [#sathoshiengineeringcrew](https://satoshiengineering.com/)_

[![MIT License Badge](docs/img/license-badge.svg)](LICENSE)

Adoption of the Bitcoin Switch with the T-Call Go V1.4 which has a GSM800 Module and a OLED display. 

This repo is based on the original source of [bitcoinSwitch](https://github.com/lnbits/bitcoinswitch) of @benarc!

This [Commit](https://github.com/lnbits/bitcoinswitch/commit/9daee009820ccbdf6b7899ee88477fb181fcf457) was used for developing.

### Hardware

- https://www.lilygo.cc/products/t-call-v1-4?variant=42285821460661
  - Schematics: https://github.com/Xinyuan-LilyGO/LilyGo-T-Call-SIM800/blob/master/doc/SIM800L_IP5306.MD 
- https://www.waveshare.com/1.5inch-RGB-OLED-Module.htm
- Relay

### Wireing
```
### OLEAD ### 
5V   3.3V/5V Power input
GND  GND
23   DIN / MOSI Data input
18   SCL Clock input
32   CS Chip select, active low (because 17 was not found)
33   DC Data/command signal selection, low level indicates command, high level indicates data
25   RST (because 5 is used for the modem)

### Relay ### 
12   IN of Relay
```

FYI: GPI: GPIO34-39 can only be set as input mode and do not have software-enabled pullup or pulldown functions.

### Dev Notes

#### Portal Pin change to 2

Pin 4 is used by the modem. Changed Portal PIN to 2.

```
#define PORTAL_PIN 2
```

#### Signal Strength

```cpp
int16_t strength = modem.getSignalQuality()
```

```
99 not known or not detectable
2 ... 30 --> Check Table here: https://m2msupport.net/m2msupport/atcsq-signal-quality/
```

#### Ghost interferance on relay pin

I am using a relay on pin12 and it was triggered, when the moden is dooing something. I needed to set the pin on output in the beginning of the code. (Not wait until the first payment is recived).

## Tip us

If you like this project, please adapt the landingpage to your local stores, that
accept bitcoin or even extend it. Why not [send some tip love?](https://satoshiengineering.com/tipjar/)
