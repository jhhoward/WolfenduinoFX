#include <Arduboy2.h>       
#include <ArduboyFX.h>      

#include "Engine.h"
#include "Generated/fxdata.h"

Arduboy2Base arduboy;
constexpr int TARGET_FRAMERATE = 30;

void setup() {
  arduboy.boot();
  arduboy.flashlight();
  arduboy.systemButtons();
  //arduboy.bootLogo();
  arduboy.setFrameRate(TARGET_FRAMERATE);

  FX::begin(FX_DATA_PAGE);    
  engine.init();
}

void loop() {
  if (!arduboy.nextFrame()) return; 
 
  Platform.update();
  engine.update();
  engine.draw();
  
  FX::display();
  //FX::display(CLEAR_BUFFER); // Using CLEAR_BUFFER will clear the display buffer after it is displayed
}
