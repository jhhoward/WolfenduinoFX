#include "GamebuinoPlatform.h"

GamebuinoPlatform Platform;
Gamebuino gb;

void GamebuinoPlatform::drawPixel(uint8_t x, uint8_t y, uint8_t colour)
{
	// TODO
}

void GambuinoPlatform::update()
{
	// TODO: input updating
}

/** Arduino interface routines **/
void setup(void) 
{
	gb.begin();
	gb.titleScreen(F("    3D DEMO\n\nControls:\n \25 strafe\n \26 run "));
	gb.display.persistence = false;
	gb.battery.show = false;
	
	Engine.init();
}

void loop()
{
	if(gb.update())
	{
		Platform.update();
		Engine.update();
	}
}
	