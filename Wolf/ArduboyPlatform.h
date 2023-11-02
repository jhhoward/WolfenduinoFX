#pragma once

#include <avr/pgmspace.h>
#include <ArduboyFX.h>      

#include "Platform.h"

extern Arduboy2Base arduboy;

extern int y_lut[];

inline uint8_t GetScreenBuffer() { return arduboy.getBuffer(); }

inline void setPixel(uint8_t x, uint8_t y)
{
	 arduboy.drawPixel(x, y, BLACK);
	//_displayBuffer[y_lut[y] + x] |= (0x01 << (y & 7));
}
inline void clearPixel(uint8_t x, uint8_t y)
{
	 arduboy.drawPixel(x, y, WHITE);
	//_displayBuffer[y_lut[y] + x] &= ~(0x01 << (y & 7));
}

inline void drawPixel(uint8_t x, uint8_t y, uint8_t colour)
{
	arduboy.drawPixel(x, y, colour);

	//if(colour)
	//{
	//	_displayBuffer[y_lut[y] + x] &= ~(0x01 << (y & 7));
	//}
	//else
	//{
	//	_displayBuffer[y_lut[y] + x] |= (0x01 << (y & 7));
	//}
}

inline void clearDisplay(uint8_t colour)
{
	//uint8_t data = colour ? 0 : 0xff;
	//memset(_displayBuffer, data, LCDWIDTH * LCDHEIGHT / 8);
}

class ArduboyPlatform : public PlatformBase
{
public:
	void playSound(uint8_t id);

	void update();
};

void ERROR(const char* msg);

extern ArduboyPlatform Platform;

inline void diskSeek(uint24_t address) 				{ FX::seekData(address); }
inline uint8_t diskReadByte() 						
{ 
	//return FX::readByte();
	//uint8_t buffer;
	//FX::readBytes(&buffer, 1);
	//return buffer;
	return FX::readPendingUInt8(); 
}
inline void diskRead(uint8_t* buffer, int length) 	{ FX::readBytes(buffer, length); }
inline void diskFinishRead() 						{ FX::readEnd(); }
