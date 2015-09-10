#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdint.h>

#include "Defines.h"

#define INPUT_BIT(x) (1 << (x))

enum
{
	Input_Dpad_Up		= INPUT_BIT(0),
	Input_Dpad_Right	= INPUT_BIT(1),
	Input_Dpad_Down		= INPUT_BIT(2),
	Input_Dpad_Left		= INPUT_BIT(3),
	Input_Btn_A			= INPUT_BIT(4),
	Input_Btn_B			= INPUT_BIT(5),
	Input_Btn_C			= INPUT_BIT(6),
};

class PlatformBase
{
public:
	virtual void drawPixel(uint8_t x, uint8_t y, uint8_t colour) = 0;
	uint8_t readInput() { return inputState; }

	uint8_t inputState;
};

#endif
