#include "Arduino.h"
#include <SPI.h>
#include <Gamebuino.h>
#include "Engine.h"

Gamebuino gb;

/** Arduino interface routines **/
void setup(void) 
{
	gb.begin();
	gb.titleScreen(F("WOLF3D DEMO\n\nControls:\n \25 strafe\n \26 run "));
	gb.display.persistence = false;
	gb.battery.show = false;
	gb.setFrameRate(30);
	
	engine.init();
}

void loop()
{
	if(gb.update())
	{
		Platform.update();
		engine.update();
	}
}
	
