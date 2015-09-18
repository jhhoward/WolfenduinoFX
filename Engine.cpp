#include "Engine.h"

Renderer Engine::renderer;
Map Engine::map;
Player Engine::player;

void Engine::init()
{
//	player.x = player.z = 48;
	player.z = CELL_SIZE * (MAP_SIZE - 2);
	player.x = CELL_SIZE * MAP_SIZE / 2;
	renderer.init();
}

#include <math.h> // temp

void Engine::update()
{
	// TODO put this in player class
    bool strafe = Platform.readInput() & Input_Btn_A;
    
	int16_t movement = MOVEMENT;
    int16_t turn = TURN;
	int16_t cos_dir = FixedMath::Cos(player.direction);
	int16_t sin_dir = FixedMath::Sin(player.direction);
	int16_t oldX = player.x;
	int16_t oldZ = player.z;
  cos_dir = (int16_t)((FIXED_ONE * cos(Engine::player.direction * 3.141592 / 128.0f)) + 0.5f);
  sin_dir = (int16_t)((FIXED_ONE * sin(Engine::player.direction * 3.141592 / 128.0f)) + 0.5f);
    
	if (Platform.readInput() & Input_Btn_B)
    {
      movement *= 2;
      turn *= 2;
    }
    
    if (Platform.readInput() & Input_Dpad_Down)
    {
      player.x -= (movement * cos_dir) >> (FIXED_SHIFT);
      player.z -= (movement * sin_dir) >> (FIXED_SHIFT);
    }
    
    if (Platform.readInput() & Input_Dpad_Up)
    {
      player.x += (movement * cos_dir) >> (FIXED_SHIFT);
      player.z += (movement * sin_dir) >> (FIXED_SHIFT);
    }
    
    if (Platform.readInput() & Input_Dpad_Left)
    {
      if (strafe)
      {
        player.x += (movement * sin_dir) >> (FIXED_SHIFT);
        player.z -= (movement * cos_dir) >> (FIXED_SHIFT);
      }
      else
        player.direction -= turn;
    }
    
    if (Platform.readInput() & Input_Dpad_Right)
    {
      if (strafe)
      {
        player.x -= (movement * sin_dir) >> (FIXED_SHIFT);
        player.z += (movement * cos_dir) >> (FIXED_SHIFT);
      }
      else
        player.direction += turn;
    }
  
	if(map.isBlocked(player.x / CELL_SIZE, player.z / CELL_SIZE))
	{
		player.x = oldX;
		player.z = oldZ;
	}

    player.x = max(player.x, 0);
    player.z = max(player.z, 0);
    player.x = min(player.x, MAP_SIZE * CELL_SIZE);
    player.z = min(player.z, MAP_SIZE * CELL_SIZE);
	//

	int16_t projectedX = player.x / CELL_SIZE + cos_dir / 15;
	int16_t projectedZ = player.z / CELL_SIZE + sin_dir / 15;

	map.updateBufferPosition(projectedX - MAP_BUFFER_SIZE / 2, projectedZ - MAP_BUFFER_SIZE / 2);

	renderer.drawFrame();
}
