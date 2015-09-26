/*
 *  Uzebox Bitmap Demo
 *  Copyright (C) 2009 Alec Bourque
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * This program demonstrates video mode 8 (bitmap mode @ 120x96 2bpp)
 */
#include <stdbool.h>
#include <avr/io.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
extern "C" {
#include "uzebox.h"
}
#include "UzeboxPlatform.h"
#include "Engine.h"

UzeboxPlatform Platform;

int main(){

	//Clear the screen (fills the vram with tile zero)
	ClearVram();

	srand(0x365e);
	
	Engine::init();
	
	while(1)
	{
		palette[0] = 0;
		palette[1] = 255;
		palette[2] = 164;
		palette[3] = 173;
		/*for(int n = 0; n < 16; n++)
		{
			PutPixel(n, n, 1);
		}*/
		
		Platform.update();
		Engine::update();
		WaitVsync(1);
		Engine::renderer.drawDeferredFrame();
	}
}

void UzeboxPlatform::update()
{
	int joypad = ReadJoypad(0);
	
	inputState = 0;
	
	if(joypad & BTN_A)
		inputState |= Input_Btn_A;
	if(joypad & BTN_B)
		inputState |= Input_Btn_B;
	if(joypad & BTN_Y)
		inputState |= Input_Btn_C;
	if(joypad & BTN_LEFT)
		inputState |= Input_Dpad_Left;
	if(joypad & BTN_RIGHT)
		inputState |= Input_Dpad_Right;
	if(joypad & BTN_UP)
		inputState |= Input_Dpad_Up;
	if(joypad & BTN_DOWN)
		inputState |= Input_Dpad_Down;
}

void PutPixel2(unsigned char x, unsigned char y,unsigned char color){
	if(x>=120 || y>=96) return;

	unsigned int addr=((SCREEN_WIDTH/4)*y)+(x>>2);
	unsigned char byte=vram[addr];
	color&=3;
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

