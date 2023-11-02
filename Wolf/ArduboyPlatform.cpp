#include "ArduboyPlatform.h"
//#include "Generated/Data_Audio.h"

ArduboyPlatform Platform;

void ArduboyPlatform::update()
{
	inputState = 0;
	  
	if(arduboy.pressed(A_BUTTON))
	{
		inputState |= Input_Btn_A;  
	}
	if(arduboy.pressed(B_BUTTON))
	{
		inputState |= Input_Btn_B;  
	}
	if(arduboy.pressed(UP_BUTTON))
	{
		inputState |= Input_Dpad_Up;  
	}
	if(arduboy.pressed(DOWN_BUTTON))
	{
		inputState |= Input_Dpad_Down;  
	}
	if(arduboy.pressed(LEFT_BUTTON))
	{
		inputState |= Input_Dpad_Left;  
	}
	if(arduboy.pressed(RIGHT_BUTTON))
	{
		inputState |= Input_Dpad_Right;  
	}
}

void ArduboyPlatform::playSound(uint8_t id)
{
//	if(!m_isMuted)
//		gb.sound.playPattern((const uint16_t*)pgm_read_word(&Data_AudioPatterns[id]), 0);
}

