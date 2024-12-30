#include "hardware.h"

SdFat SD;
File fp;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_MCP23X08 mcp;

bool keys[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool oldKeys[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setupHardware()
{
    tft.init(TFT_WIDTH, TFT_HEIGHT); // Init ST7789 display
    tft.setRotation(1);
    tft.setSPISpeed(80000000);
    tft.fillScreen(ST77XX_BLACK);

    if (!SD.begin(SD_CS, SD_SCK_MHZ(4)))
    { /// SD max 10MHz!!!
        Serial.println("SD card initialization failed!");
        while (1)
        {
        }; // FREEZE
    }

    if (!mcp.begin_I2C())
    {
        Serial.println("Error initilizing MCP23008.");
        while (1)
            ;
    }
    mcp.pinMode(GPIO_UP, INPUT_MODE);
    mcp.pinMode(GPIO_DOWN, INPUT_MODE);
    mcp.pinMode(GPIO_LEFT, INPUT_MODE);
    mcp.pinMode(GPIO_RIGHT, INPUT_MODE);
    mcp.pinMode(GPIO_A, INPUT_MODE);
    mcp.pinMode(GPIO_B, INPUT_MODE);
}

bool get_keys()
{
    // Clear keys
    bool changed = false;
    for (int i = 0; i < 8; i++)
    {
        if (keys[i] != oldKeys[i])
        {
            changed = true;
        }
        oldKeys[i] = keys[i];
        keys[i] = 0;
    }

    uint8_t inputs[6] = {0, 0, 0, 0, 0, 0};

    inputs[KEY_UP] = !mcp.digitalRead(GPIO_UP);
    inputs[KEY_DOWN] = !mcp.digitalRead(GPIO_DOWN);
    inputs[KEY_LEFT] = !mcp.digitalRead(GPIO_LEFT);
    inputs[KEY_RIGHT] = !mcp.digitalRead(GPIO_RIGHT);
    inputs[KEY_A] = !mcp.digitalRead(GPIO_A);
    inputs[KEY_B] = !mcp.digitalRead(GPIO_B);

    if (inputs[KEY_A])
    {
        // Alternate mode
        if (inputs[KEY_UP])
        {
            // Select
            keys[KEY_SELECT] = 1;
        }
        else if (inputs[KEY_DOWN])
        {
            // Start
            keys[KEY_START] = 1;
        }
        else
        {
            // A
            keys[KEY_A] = 1;
        }
    }
    else
    {
        // Normal mode
        for (int i = 0; i < 6; i++)
        {

            keys[i] = inputs[i];
        }
    }

    return changed;
}