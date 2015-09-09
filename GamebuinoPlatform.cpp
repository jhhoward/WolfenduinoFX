#include "GamebuinoPlatform.h"

GamebuinoPlatform Platform;
Gamebuino gb;

// look-up table to speed up calculation of the line address
int y_lut[48] = {0,0,0,0,0,0,0,0,84,84,84,84,84,84,84,84,168,168,168,168,168,168,168,168,252,252,252,252,252,252,252,252,336,336,336,336,336,336,336,336,420,420,420,420,420,420,420,420};

void GamebuinoPlatform::drawPixel(uint8_t x, uint8_t y, uint8_t colour)
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

void GambuinoPlatform::update()
{
	inputState = 0;
	if(gb.buttons.pressed(BTN_A))
		inputState |= Input_Btn_A;
	if(gb.buttons.pressed(BTN_B))
		inputState |= Input_Btn_B;
	if(gb.buttons.pressed(BTN_C))
		inputState |= Input_Btn_C;
	if(gb.buttons.pressed(BTN_UP))
		inputState |= Input_Dpad_Up;
	if(gb.buttons.pressed(BTN_RIGHT))
		inputState |= Input_Dpad_Right;
	if(gb.buttons.pressed(BTN_DOWN))
		inputState |= Input_Dpad_Down;
	if(gb.buttons.pressed(BTN_LEFT))
		inputState |= Input_Dpad_Left;
		
}

/** Arduino interface routines **/
void setup(void) 
{
	gb.begin();
	gb.titleScreen(F("    3D DEMO\n\nControls:\n \25 strafe\n \26 run "));
	gb.display.persistence = false;
	gb.battery.show = false;
	gb.setFrameRate(30);
	
	Engine::init();
}

void loop()
{
	if(gb.update())
	{
		Platform.update();
		Engine::update();
	}
}
	