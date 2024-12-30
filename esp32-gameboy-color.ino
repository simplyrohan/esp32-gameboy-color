#include <Arduino.h>
extern "C"
{
#include "src/gnuboy/gnuboy.h"
}
#include "hardware.h"


#define PANIC(x)          \
    {                     \
        Serial.printf(x); \
        while (1)          \
            ;             \
    }


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

void audio_callback(void *buffer, size_t length) {}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    setupHardware();
    Serial.println("Hardware setup");

    char *romfilename = "Super Mario Bros Deluxe.gbc";

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
	// gnuboy_load_rom((const byte *)rom, romSize);

	Serial.println("ROM loaded");

	gnuboy_set_palette(GB_PALETTE_DMG);
	Serial.println("Palette set");

	// Hard reset to have a clean slate
	gnuboy_reset(true);
	Serial.println("Emulator reset");
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
