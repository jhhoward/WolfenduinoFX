#include "Arduino.h"
#include <SPI.h>
#include <Gamebuino.h>
#include "GamebuinoPlatform.h"
#include "Engine.h"

GamebuinoPlatform Platform;
Gamebuino gb;

extern uint8_t _displayBuffer[];

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

void GamebuinoPlatform::update()
{
	inputState = 0;
        if(digitalRead(BTN_A_PIN)==LOW)
		inputState |= Input_Btn_A;
        if(digitalRead(BTN_B_PIN)==LOW)
		inputState |= Input_Btn_B;
        if(digitalRead(BTN_C_PIN)==LOW)
		inputState |= Input_Btn_C;
        if(digitalRead(BTN_UP_PIN)==LOW)
		inputState |= Input_Dpad_Up;
        if(digitalRead(BTN_RIGHT_PIN)==LOW)
		inputState |= Input_Dpad_Right;
        if(digitalRead(BTN_DOWN_PIN)==LOW)
		inputState |= Input_Dpad_Down;
        if(digitalRead(BTN_LEFT_PIN)==LOW)
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
	
