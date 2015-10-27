#ifndef GAMEBUINO_PLATFORM_H_
#define GAMEBUINO_PLATFORM_H_

#include <SPI.h>
#include <Gamebuino.h>
//#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "Platform.h"
#include "Arduino.h"

extern int y_lut[];
inline void setPixel(uint8_t x, uint8_t y)
{
	_displayBuffer[y_lut[y] + x] |= (0x01 << (y & 7));
}
inline void clearPixel(uint8_t x, uint8_t y)
{
	_displayBuffer[y_lut[y] + x] &= ~(0x01 << (y & 7));
}

inline void drawPixel(uint8_t x, uint8_t y, uint8_t colour)
{
	if(colour)
	{
		_displayBuffer[y_lut[y] + x] &= ~(0x01 << (y & 7));
	}
	else
	{
		_displayBuffer[y_lut[y] + x] |= (0x01 << (y & 7));
	}
}

inline void clearDisplay(uint8_t colour)
{
	uint8_t data = colour ? 0 : 0xff;
	memset(_displayBuffer, data, LCDWIDTH * LCDHEIGHT / 8);
}

class GamebuinoPlatform : public PlatformBase
{
public:
	void playSound(uint8_t id);

	void update();
	
	bool isMuted();
	void setMuted(bool muted);
};

void ERROR(const char* msg);


extern GamebuinoPlatform Platform;

#endif
