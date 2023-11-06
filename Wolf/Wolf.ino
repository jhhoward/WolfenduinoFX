#include <Arduboy2.h>       
#include <ArduboyFX.h>      
#include <ArduboyTones.h>

#include "Engine.h"
#include "Generated/fxdata.h"
#include "ArduboyPlatform.h"
#include "Generated/Data_Audio.h"

Arduboy2Base arduboy;
ArduboyTones sound(arduboy.audio.enabled);

unsigned long lastTimingSample;

void ArduboyPlatform::playSound(uint8_t id)
{
	sound.tones((const uint16_t*)pgm_read_word(&Data_AudioPatterns[id]));
}

void setup() {
  arduboy.boot();
  arduboy.flashlight();
  arduboy.systemButtons();
  //arduboy.bootLogo();
  arduboy.setFrameRate(TARGET_FRAMERATE);
  arduboy.audio.begin();
  arduboy.audio.on();

  FX::begin(FX_DATA_PAGE);    
  engine.init();
}

void loop() {
  static int16_t tickAccum = 0;
  unsigned long timingSample = millis();
  tickAccum += (timingSample - lastTimingSample);
  lastTimingSample = timingSample;
  
  if (!arduboy.nextFrame()) return; 
 
  Platform.update();

  constexpr int16_t frameDuration = 1000 / TARGET_FRAMERATE;
  while(tickAccum > frameDuration)
  {
    engine.update();
	tickAccum -= frameDuration;
  }
	
  engine.draw();
  
  FX::display();
  //FX::display(CLEAR_BUFFER); // Using CLEAR_BUFFER will clear the display buffer after it is displayed
}
