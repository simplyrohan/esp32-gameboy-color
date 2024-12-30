#include <Arduino.h>
extern "C"
{
#include "src/gnuboy/gnuboy.h"
}
#include "hardware.h"

#define PANIC(x)          \
    {                     \
        Serial.printf(x); \
        while (1)         \
            ;             \
    }

// Scan files for selection menu
File file;
File dir;

char filenames[255][100];
int filenum = 0;

// Video
uint16_t *videoBuffer = NULL;
uint16_t *oldVideoBuffer = NULL;

int draw = 0;

void video_callback(void *buffer)
{
    tft.startWrite();
    for (int x = 0; x < 160; x++)
    {
        for (int y = 0; y < 144; y++)
        {
            if (videoBuffer[x + y * 160] != oldVideoBuffer[x + y * 160])
            {
                tft.writePixel(x + 40, y, videoBuffer[x + y * 160]);
            }
        }
    }
    tft.endWrite();
}

void audio_callback(void *buffer, size_t length) {} // Audio not supported yet

void findFiles()
{
    // Open root directory
    dir = SD.open("/");
    if (!dir)
    {
        PANIC("Failed to open root directory");
    }
    dir.rewind();

    filenum = 0;

    char fileName[100];

    while (file.openNext(&dir, O_READ))
    {
        if (!file.isHidden())
        {
            file.getName(fileName, sizeof(fileName));
            Serial.println(fileName);

            // check if file is a ROM
            if (strstr(fileName, ".gbc") || strstr(fileName, ".gb"))
            {
                strcpy(filenames[filenum], fileName);
                filenum++;
            }
        }
        file.close();
    }
}

String selectROM()
{
    String filename = "";

    findFiles();
    for (int i = 0; i < filenum; i++)
    {
        tft.setCursor(20, 20 + (i * 20));
        tft.println(filenames[i]);
    }

    uint8_t selected = -1;
    uint8_t cursor = 0;

    // Draw cursor
    tft.setTextColor(0xFFFF, 0x0000);
    tft.setCursor(0, 20);
    tft.println(">");
    while (1)
    {
        get_keys();

        if (keys[KEY_UP])
        {
            tft.setCursor(0, 20 + (cursor * 20));
            tft.println(" ");
            cursor--;
            if (cursor < 0)
                cursor = filenum - 1;

            tft.setCursor(0, 20 + (cursor * 20));
            tft.println(">");
            delay(200);
        }
        if (keys[KEY_DOWN])
        {
            tft.setCursor(0, 20 + (cursor * 20));
            tft.println(" ");
            cursor++;
            if (cursor >= filenum)
                cursor = 0;

            tft.setCursor(0, 20 + (cursor * 20));
            tft.println(">");
            delay(200);
        }

        if (keys[KEY_A])
        {
            filename = filenames[cursor];
            return filename;
        }

        delay(5);
    }
}

void printCentered(char *str)
{
    tft.setTextSize(1);

    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(TFT_WIDTH / 2 - w / 2, TFT_HEIGHT / 2 - h / 2);
    tft.println(str);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    setupHardware();
    Serial.println("Hardware setup");

    // char *romfilename = "Super Mario Bros Deluxe.gbc";
    String romfilename = selectROM();

    tft.fillScreen(ST77XX_BLACK);
    printCentered("Loading Game");

    // Initialize the emulator
    if (gnuboy_init(16000, GB_AUDIO_STEREO_S16, GB_PIXEL_565_LE, &video_callback, &audio_callback) < 0)
        PANIC("Emulator init failed!");

    Serial.println("Emulator initialized");

    // Allocate video and audio buffers
    videoBuffer = (uint16_t *)malloc(160 * 144 * sizeof(uint16_t));
    oldVideoBuffer = (uint16_t *)malloc(160 * 144 * sizeof(uint16_t));
    if (!videoBuffer)
        PANIC("Video buffer allocation failed!");
    Serial.println("Video buffer allocated");
    if (!oldVideoBuffer)
        PANIC("Old video buffer allocation failed!");
    Serial.println("Old video buffer allocated");

    // Allocate audio buffer
    gnuboy_set_framebuffer((void *)videoBuffer);
    Serial.println("Frame buffer set");

    // Load ROM
    fp = SD.open(romfilename, FILE_READ);
    if (!fp)
    {
        PANIC("ROM Loading failed!");
    }
    Serial.println("ROM opened");

    size_t size = fp.size();
    Serial.println(fp.size());
    byte *data;
    Serial.println("ROM size read");
    data = (byte *)ps_malloc(size);
    Serial.println(fp.read(data, size));

    gnuboy_load_rom((const byte *)data, size);

    Serial.println("ROM loaded");

    gnuboy_set_palette(GB_PALETTE_DMG);
    Serial.println("Palette set");

    // Hard reset to have a clean slate
    gnuboy_reset(true);
    Serial.println("Emulator reset");

    tft.fillScreen(ST77XX_BLACK);
    printCentered("Loaded!");
    delay(1000);
    tft.fillScreen(ST77XX_BLACK);
}

void loop()
{
    if (get_keys())
    {
        int pad = 0;
        if (keys[KEY_UP])
            pad |= GB_PAD_UP;
        if (keys[KEY_RIGHT])
            pad |= GB_PAD_RIGHT;
        if (keys[KEY_DOWN])
            pad |= GB_PAD_DOWN;
        if (keys[KEY_LEFT])
            pad |= GB_PAD_LEFT;
        if (keys[KEY_A])
            pad |= GB_PAD_A;
        if (keys[KEY_B])
            pad |= GB_PAD_B;
        if (keys[KEY_SELECT])
            pad |= GB_PAD_SELECT;
        if (keys[KEY_START])
            pad |= GB_PAD_START;

        gnuboy_set_pad(pad);

        Serial.println("Updating pad");
    }

    if (draw <= 0)
    {
        gnuboy_run(1); // 1 = draw
        draw = 3;
        // copy video buffer to old video buffer
        memcpy(oldVideoBuffer, videoBuffer, 160 * 144 * sizeof(uint16_t));
    }
    else
    {
        draw--;
        gnuboy_run(0); // 0 = no draw
    }
}
