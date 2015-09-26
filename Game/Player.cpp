#include "Engine.h"
#include "Player.h"

Player::Player()
{
	weapon.type = WeaponType_Pistol;
	weapon.ammo = 8;
	weapon.frame = 0;
	weapon.debounce = false;
}

void Player::update()
{
	bool strafe = Platform.readInput() & Input_Btn_A;
    
	int16_t movement = MOVEMENT;
	int16_t turn = TURN;
	int16_t cos_dir = FixedMath::Cos(direction);
	int16_t sin_dir = FixedMath::Sin(direction);
	int16_t oldX = x;
	int16_t oldZ = z;
	int16_t deltaX = 0, deltaZ = 0;
    
	updateWeapon();
    
	if (Platform.readInput() & Input_Dpad_Down)
	{
		deltaX -= (movement * cos_dir) >> (FIXED_SHIFT);
		deltaZ -= (movement * sin_dir) >> (FIXED_SHIFT);
	}
    
	if (Platform.readInput() & Input_Dpad_Up)
	{
		deltaX += (movement * cos_dir) >> (FIXED_SHIFT);
		deltaZ += (movement * sin_dir) >> (FIXED_SHIFT);
	}
    
	if (Platform.readInput() & Input_Dpad_Left)
	{
		if (strafe)
		{
			deltaX += (movement * sin_dir) >> (FIXED_SHIFT);
			deltaZ -= (movement * cos_dir) >> (FIXED_SHIFT);
		}
		else
			direction -= turn;
	}	
    
	if (Platform.readInput() & Input_Dpad_Right)
	{
		if (strafe)
		{
			deltaX -= (movement * sin_dir) >> (FIXED_SHIFT);
			deltaZ += (movement * cos_dir) >> (FIXED_SHIFT);
		}
		else
			direction += turn;
	}
  
	move(deltaX, deltaZ);

	//int16_t projectedX = x / CELL_SIZE;
	//int16_t projectedZ = z / CELL_SIZE;

	int16_t projectedX = x / CELL_SIZE + cos_dir / 19;
	int16_t projectedZ = z / CELL_SIZE + sin_dir / 19;

	Engine::map.updateBufferPosition(projectedX - MAP_BUFFER_SIZE / 2, projectedZ - MAP_BUFFER_SIZE / 2);

	// Check for doors
	int cellX = x / CELL_SIZE;
	int cellZ = z / CELL_SIZE;

	Engine::map.openDoorsAt(cellX, cellZ);

	if(cos_dir > 0)
	{
		Engine::map.openDoorsAt(cellX + 1, cellZ);
	}
	else
	{
		Engine::map.openDoorsAt(cellX - 1, cellZ);
	}
	if(sin_dir > 0)
	{
		Engine::map.openDoorsAt(cellX, cellZ + 1);
	}
	else
	{
		Engine::map.openDoorsAt(cellX, cellZ - 1);
	}

}

void Player::move(int16_t deltaX, int16_t deltaZ)
{
	int cellX = x / CELL_SIZE;
	int cellZ = z / CELL_SIZE;

	if(deltaX < 0)
	{
		if(Engine::map.isBlocked(cellX - 1, cellZ)
			|| (z < cellZ * CELL_SIZE + MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX - 1, cellZ - 1))
			|| (z > cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX - 1, cellZ + 1)))
		{
			if(x + deltaX < cellX * CELL_SIZE + MIN_WALL_DISTANCE)
			{
				deltaX = (cellX * CELL_SIZE + MIN_WALL_DISTANCE) - x;
			}
		}
	}
	else if(deltaX > 0)
	{
		if(Engine::map.isBlocked(cellX + 1, cellZ)
			|| (z < cellZ * CELL_SIZE + MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX + 1, cellZ - 1))
			|| (z > cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX + 1, cellZ + 1)))
		{
			if(x + deltaX > cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE)
			{
				deltaX = (cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE) - x;
			}
		}
	}

	if(deltaZ < 0)
	{
		if(Engine::map.isBlocked(cellX, cellZ - 1)
			|| (x < cellX * CELL_SIZE + MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX - 1, cellZ - 1))
			|| (x > cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX + 1, cellZ - 1)))
		{
			if(z + deltaZ < cellZ * CELL_SIZE + MIN_WALL_DISTANCE)
			{
				deltaZ = (cellZ * CELL_SIZE + MIN_WALL_DISTANCE) - z;
			}
		}
	}
	else if(deltaZ > 0)
	{
		if(Engine::map.isBlocked(cellX, cellZ + 1)
			|| (x < cellX * CELL_SIZE + MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX - 1, cellZ + 1))
			|| (x > cellX * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE && Engine::map.isBlocked(cellX + 1, cellZ + 1)))
		{
			if(z + deltaZ > cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE)
			{
				deltaZ = (cellZ * CELL_SIZE + CELL_SIZE - MIN_WALL_DISTANCE) - z;
			}
		}
	}

	x += deltaX;
	z += deltaZ;
}

#define NUM_WEAPON_FRAMES 4

void Player::updateWeapon()
{
	if (Platform.readInput() & Input_Btn_B)
	{
		if(!weapon.debounce)
		{
			weapon.debounce = true;
			if(weapon.shooting == false)
			{
				weapon.shooting = true;
				weapon.time = 0;
			}
		}
	}
	else
	{
		weapon.debounce = false;
	}

	if(weapon.shooting)
	{
		weapon.time++;

		switch(weapon.time)
		{
		case 2:
			weapon.frame = 1;
			break;
		case 4:
			weapon.frame = 2;
			break;
		case 6:
			weapon.frame = 3;
			break;
		case 8:
			weapon.frame = 1;
			break;
		case 10:
			weapon.frame = 0;
			weapon.shooting = false;
			break;
		}
	}
}
