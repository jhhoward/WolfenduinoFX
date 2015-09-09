#include "Engine.h"

Renderer Engine::renderer;
Map Engine::map;
Player Engine::player;

void Engine::init()
{
	player.x = player.z = 48;
	renderer.init();
}

void Engine::update()
{
	// TODO put this in player class
    bool strafe = Platform.readInput() & Input_Btn_A;
    
	int16_t movement = MOVEMENT;
    int16_t turn = TURN;
	int16_t cos_dir = FixedMath::Cos(player.direction);
	int16_t sin_dir = FixedMath::Sin(player.direction);
    
	if (Platform.readInput() & Input_Btn_B)
    {
      movement *= 2;
      turn *= 2;
    }
    
    if (Platform.readInput() & Input_Dpad_Down)
    {
      player.x -= (movement * sin_dir) >> (FIXED_SHIFT * 2);
      player.z -= (movement * cos_dir) >> (FIXED_SHIFT * 2);
    }
    
    if (Platform.readInput() & Input_Dpad_Up)
    {
      player.x += (movement * sin_dir) >> (FIXED_SHIFT * 2);
      player.z += (movement * cos_dir) >> (FIXED_SHIFT * 2);
    }
    
    if (Platform.readInput() & Input_Dpad_Left)
    {
      if (strafe)
      {
        player.x -= (movement * cos_dir) >> (FIXED_SHIFT * 2);
        player.z += (movement * sin_dir) >> (FIXED_SHIFT * 2);
      }
      else
        player.direction -= turn;
    }
    
    if (Platform.readInput() & Input_Dpad_Right)
    {
      if (strafe)
      {
        player.x += (movement * cos_dir) >> (FIXED_SHIFT * 2);
        player.x -= (movement * sin_dir) >> (FIXED_SHIFT * 2);
      }
      else
        player.direction += turn;
    }
  
    player.x = max(player.x, 0);
    player.z = max(player.z, 0);
    player.x = min(player.x, MAP_SIZE * CELL_SIZE);
    player.z = min(player.z, MAP_SIZE * CELL_SIZE);
	//

	renderer.drawFrame();
}
