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
	case ActorState_Injured:
		if(updateFrame)
		{
			switchState(ActorState_Idle);
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

	engine.renderer.queueSprite((uint8_t*) Data_guardSprite + TEXTURE_STRIDE * TEXTURE_SIZE * offset, x, z);
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
