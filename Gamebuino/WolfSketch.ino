#include "Arduino.h"
#include <SPI.h>
#include <Gamebuino.h>
#include "Engine.h"

#ifdef PETIT_FATFS_FILE_STREAMING
#include <petit_fatfs.h>
#endif

Gamebuino gb;
long lastMillis = 0;
uint16_t millisBehind = 0;

/** Arduino interface routines **/
void setup(void) 
{
	//Serial.begin(9600);
	//Serial.print("Hello world!");
	gb.begin();
	//gb.titleScreen(F("WOLF3D DEMO\n\nControls:\n \25 strafe\n \26 run "));
	gb.display.persistence = true; //false;
	gb.display.autoUpdate = false;
	gb.battery.show = false;
	gb.setFrameRate(30);
	
#ifdef PETIT_FATFS_FILE_STREAMING
	PFFS.begin(10, rx, tx);
#endif

	engine.init();
	//gb.sound.setVolume(0, 0);
	lastMillis = millis();
}

#define FRAME_MS (1000 / 20)
#define MAX_BEHIND (FRAME_MS * 5)

void ERROR(const char* msg)
{
	engine.renderer.drawString(msg, 0, 0);
	gb.display.update();
}

void loop()
{
	gb.update();
	//return;
	Platform.update();

	long newMillis = millis();
	millisBehind += newMillis - lastMillis;
	
	if(millisBehind > MAX_BEHIND)
	{
		millisBehind = MAX_BEHIND;
	}
	
	while(millisBehind > FRAME_MS)
	{
		engine.update();
		millisBehind -= FRAME_MS;
		if(engine.gameState != GameState_Playing)
			break;
	}
	lastMillis = newMillis;

	engine.draw();
	gb.display.update();
}
	
#ifdef PETIT_FATFS_FILE_STREAMING
// For SD card access:
byte rx()
{
  // The SPI Data Register is a read/write register 
  // used for data transfer between the Register File and the SPI Shift Register. 
  // Writing to the register initiates data transmission. Reading the register causes the Shift Register receive buffer to be read.
  SPDR = 0xFF; // dummy byte to initiate reading
  // loop_until_bit_is_set is an AVR libc macro
  // SPSR is SPI status register, SPIF is SPI interrupt flag, that is set after transmission is complete
  loop_until_bit_is_set(SPSR, SPIF);
  return SPDR; //return the SPDR (= value back from slave)
}

void tx(byte d)
{
  SPDR = d;
  loop_until_bit_is_set(SPSR, SPIF);
}
#endif