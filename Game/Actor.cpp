#include "Engine.h"
#include "Actor.h"

#include "Data_Guard.h"

void Actor::init(uint8_t id, uint8_t actorType, uint8_t cellX, uint8_t cellZ)
{
	spawnId = id;
	type = actorType;
	state = ActorState_Idle;
	x = cellX * CELL_SIZE + CELL_SIZE / 2;
	z = cellZ * CELL_SIZE + CELL_SIZE / 2;
	targetCellX = cellX;
	targetCellZ = cellZ;
	hp = 20;
}

void Actor::update()
{
	if(type == ActorType_Empty)
		return;

	updateFrozenState();

	if(flags.frozen)
		return;

	bool updateFrame = (engine.frameCount & 0x3) == 0;

	switch(state)
	{
	case ActorState_Idle:
		if(engine.map.isClearLine(x, z, engine.player.x, engine.player.z))
		{
			switchState(ActorState_Active);
		}
		break;
	case ActorState_Active:
	{
		frame = 0;
		if(tryMove())
		{
			pickNewTargetCell();
		}
	}
		break;
	case ActorState_Injured:
		if(updateFrame)
		{
			switchState(ActorState_Active);
		}
		break;
	case ActorState_Dying:
		if(updateFrame)
		{
			frame++;
			if(frame == 8)
				switchState(ActorState_Dead);
		}
		break;
	case ActorState_Dead:
		break;
	default:
		frame = 0;
		break;
	}
//	if((engine.frameCount & 0x3) == 0)
	//	frame = (frame + 1) % 4;
}

void Actor::draw()
{
	int offset = frame; // == 3 ? 1 : frame;

	engine.renderer.queueSprite((SpriteFrame*)&Data_guardSprite_frames[frame], (uint8_t*) Data_guardSprite, x, z);
}

void Actor::updateFrozenState()
{
	int cellX = x / CELL_SIZE;
	int cellZ = z / CELL_SIZE;

	flags.frozen = cellX < engine.map.bufferX || cellZ < engine.map.bufferZ || cellX >= engine.map.bufferX + MAP_BUFFER_SIZE || cellZ >= engine.map.bufferZ + MAP_BUFFER_SIZE;
}

void Actor::damage(int amount)
{
	if(hp == 0)
		return;

	if(amount > hp)
	{
		hp = 0;
	}
	else hp -= amount;

	if(hp == 0)
	{
		switchState(ActorState_Dying);
		engine.map.markActorKilled(spawnId);
	}
	else
	{
		switchState(ActorState_Injured);
	}
}

void Actor::switchState(uint8_t newState)
{
	state = newState;

	switch(newState)
	{
	case ActorState_Injured:
		frame = 4;
		break;
	case ActorState_Dying:
		frame = 4;
		break;
	case ActorState_Dead:
		frame = 8;
		break;
	default:
		break;
	}
}

void Actor::dropItem(uint8_t itemType)
{
	int cellX = x / CELL_SIZE;
	int cellZ = z / CELL_SIZE;

	if(tryDropItem(itemType, cellX, cellZ))
		return;

	for(int i = cellX - 1; i < cellX + 1; i++)
	{
		for(int j = cellZ - 1; j < cellZ + 1; j++)
		{
			if(tryDropItem(itemType, i, j))
				return;
		}
	}
}

bool Actor::tryDropItem(uint8_t itemType, int cellX, int cellZ)
{
	uint8_t tile = engine.map.getTile(cellX, cellZ);
	if(tile == 0)
	{
		// drop here
		return true;
	}
	return false;
}

bool Actor::tryMove()
{
	int16_t newX = x;
	int16_t newZ = z;
	int movement = 1;

	int16_t targetX = targetCellX * CELL_SIZE + CELL_SIZE / 2;
	int16_t targetZ = targetCellZ * CELL_SIZE + CELL_SIZE / 2;

	if(x < targetX)
	{
		if(targetX - x < movement)
		{
			newX = targetX;
		}
		else newX = x + movement;
	}
	else if(x > targetX)
	{
		if(x - targetX < movement)
		{
			newX = targetX;
		}
		else newX = x - movement;
	}
	if(z < targetZ)
	{
		if(targetZ - z < movement)
		{
			newZ = targetZ;
		}
		else newZ = z + movement;
	}
	else if(z > targetZ)
	{
		if(z - targetZ < movement)
		{
			newZ = targetZ;
		}
		else newZ = z - movement;
	}

	if(newX >= engine.player.x - MIN_ACTOR_DISTANCE && newX <= engine.player.x + MIN_ACTOR_DISTANCE
	&& newZ >= engine.player.z - MIN_ACTOR_DISTANCE && newZ <= engine.player.z + MIN_ACTOR_DISTANCE)
	{
		return false;
	}

	x = newX;
	z = newZ;

	return (x == targetX && z == targetZ);
}

void Actor::pickNewTargetCell()
{
	uint8_t newTargetCellX = targetCellX;
	uint8_t newTargetCellZ = targetCellZ;

	// TODO: better way of picking next tile
	if(engine.player.x < x)
	{
		newTargetCellX --;
	}
	else
	{
		newTargetCellX ++;
	}
	if(engine.player.z < z)
	{
		newTargetCellZ --;
	}
	else
	{
		newTargetCellZ ++;
	}

	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(this != &engine.actors[n] && engine.actors[n].type != ActorType_Empty && engine.actors[n].hp > 0)
		{
			if(engine.actors[n].targetCellX == newTargetCellX && engine.actors[n].targetCellZ == newTargetCellZ)
				return;
		}
	}

	if(engine.map.getTile(newTargetCellX, newTargetCellZ) == 0)
	{
		targetCellX = newTargetCellX;
		targetCellZ = newTargetCellZ;
	}

}
