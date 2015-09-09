#ifndef GAMEBUINO_PLATFORM_H_
#define GAMEBUINO_PLATFORM_H_

#include <SPI.h>
#include <Gamebuino.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

class GamebuinoPlatform : PlatformBase
{
public:
	virtual void drawPixel(uint8_t x, uint8_t y, uint8_t colour);
	
	void update();
};

extern GamebuinoPlatform Platform;

#endif
