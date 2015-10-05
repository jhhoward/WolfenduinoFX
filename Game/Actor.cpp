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
		if(tryMove())
		{
			frame = (engine.frameCount >> 2) & 0x3;
			if(frame == 3)
				frame = 1;
		}
		else
		{
			frame = 1;
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

bool Actor::isPlayerColliding()
{
	if(x >= engine.player.x - MIN_ACTOR_DISTANCE && x <= engine.player.x + MIN_ACTOR_DISTANCE
	&& z >= engine.player.z - MIN_ACTOR_DISTANCE && z <= engine.player.z + MIN_ACTOR_DISTANCE)
	{
		return true;
	}
	return false;
}

bool Actor::tryMove()
{
	int movement = 1;

	if(engine.map.isBlocked(targetCellX, targetCellZ))
	{
		engine.map.openDoorsAt(targetCellX, targetCellZ);
		return false;
	}

	int16_t targetX = targetCellX * CELL_SIZE + CELL_SIZE / 2;
	int16_t targetZ = targetCellZ * CELL_SIZE + CELL_SIZE / 2;

	int8_t deltaX = clamp(targetX - x, -movement, movement);
	int8_t deltaZ = clamp(targetZ - z, -movement, movement);

	x += deltaX;
	z += deltaZ;

	if(isPlayerColliding())
	{
		x -= deltaX;
		z -= deltaZ;
		return false;
	}

	if(x == targetX && z == targetZ)
	{
		pickNewTargetCell();
	}
	return true;
}

bool Actor::tryPickCell(int8_t newX, int8_t newZ)
{
	if(engine.map.isBlocked(newX, newZ) && !engine.map.isDoor(newX, newZ))
		return false;
	if(engine.map.isBlocked(targetCellX, newZ) && !engine.map.isDoor(targetCellX, newZ))
		return false;
	if(engine.map.isBlocked(newX, targetCellZ) && !engine.map.isDoor(newX, targetCellZ))
		return false;

	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(this != &engine.actors[n] && engine.actors[n].type != ActorType_Empty && engine.actors[n].hp > 0)
		{
			if(engine.actors[n].targetCellX == newX && engine.actors[n].targetCellZ == newZ)
				return false;
		}
	}

	targetCellX = newX;
	targetCellZ = newZ;

	return true;
}

bool Actor::tryPickCells(int8_t deltaX, int8_t deltaZ)
{
	return tryPickCell(targetCellX + deltaX, targetCellZ + deltaZ)
		|| tryPickCell(targetCellX + deltaX, targetCellZ) 
		|| tryPickCell(targetCellX, targetCellZ + deltaZ) 
		|| tryPickCell(targetCellX - deltaX, targetCellZ + deltaZ)
		|| tryPickCell(targetCellX + deltaX, targetCellZ - deltaZ);
}

void Actor::pickNewTargetCell()
{
	int8_t deltaX = clamp(engine.player.x / CELL_SIZE - targetCellX, -1, 1);
	int8_t deltaZ = clamp(engine.player.z / CELL_SIZE - targetCellZ, -1, 1);
	uint8_t dodgeChance = random() & 0xff;

	if(deltaX == 0)
	{
		if(dodgeChance < 64)
		{
			deltaX = -1;
		}
		else if(dodgeChance < 128)
		{
			deltaX = 1;
		}
	}
	else if(deltaZ == 0)
	{
		if(dodgeChance < 64)
		{
			deltaZ = -1;
		}
		else if(dodgeChance < 128)
		{
			deltaZ = 1;
		}
	}

	tryPickCells(deltaX, deltaZ);
}
