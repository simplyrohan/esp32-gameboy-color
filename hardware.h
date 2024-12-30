#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "Adafruit_GFX.h"
#include "Adafruit_ST7789.h"
#include <Adafruit_MCP23X08.h>
#include "SdFat.h"

#define KEY_UP 0
#define KEY_DOWN 1
#define KEY_LEFT 2
#define KEY_RIGHT 3
#define KEY_A 4
#define KEY_B 5
#define KEY_START 6
#define KEY_SELECT 7

#define GPIO_UP 6
#define GPIO_DOWN 4
#define GPIO_LEFT 1
#define GPIO_RIGHT 7
#define GPIO_A 2
#define GPIO_B 5

#define INPUT_MODE INPUT_PULLUP

#define TFT_CS RX
#define TFT_RST A3
#define TFT_DC A2
#define TFT_BL TX

#define TFT_HEIGHT 240
#define TFT_WIDTH 135

#define SD_CS A1

extern SdFat SD;
extern File fp;

extern Adafruit_ST7789 tft;
extern Adafruit_MCP23X08 mcp;

extern bool keys[8];
extern bool oldKeys[8];

extern void setupHardware();

extern bool get_keys();

#endif