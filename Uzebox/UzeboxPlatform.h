#ifndef UZEBOX_PLATFORM_H_
#define UZEBOX_PLATFORM_H_

#include <avr/pgmspace.h>
extern "C" {
#include "Uzebox.h"
}
#include "Platform.h"

inline void drawPixel(unsigned char x, unsigned char y,unsigned char color)
{
	unsigned int addr=((SCREEN_WIDTH/4)*y)+(x>>2);
	unsigned char byte=vram[addr];
	switch(x&3){
		case 3:						
			byte=(byte&~(3))|color;
			break;
		case 2:						
			byte=(byte&~(3<<2))|(color<<2);
			break;
		case 1:						
			byte=(byte&~(3<<4))|(color<<4);
			break;
		default:
			byte=(byte&~(3<<6))|(color<<6);
	}
	vram[addr]=byte;
}

inline void setPixel(uint8_t x, uint8_t y)
{
	drawPixel(x, y, 0);
//	_displayBuffer[y_lut[y] + x] |= (0x01 << (y & 7));
}
inline void clearPixel(uint8_t x, uint8_t y)
{
	drawPixel(x, y, 1);
	//_displayBuffer[y_lut[y] + x] &= ~(0x01 << (y & 7));
}

class UzeboxPlatform : public PlatformBase
{
public:
	
	void update();
};

extern UzeboxPlatform Platform;

#endif
