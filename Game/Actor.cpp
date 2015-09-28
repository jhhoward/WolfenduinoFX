#include "Engine.h"
#include "Actor.h"

void Actor::init(uint8_t id, uint8_t actorType, uint8_t cellX, uint8_t cellZ)
{
	spawnId = id;
	type = actorType;
	x = cellX * CELL_SIZE + CELL_SIZE / 2;
	z = cellZ * CELL_SIZE + CELL_SIZE / 2;
}

void Actor::update()
{
	if(type == ActorType_Empty)
		return;

	int cellX = x / CELL_SIZE;
	int cellZ = z / CELL_SIZE;

	flags.frozen = cellX < engine.map.bufferX || cellZ < engine.map.bufferZ || cellX >= engine.map.bufferX + MAP_BUFFER_SIZE || cellZ >= engine.map.bufferZ + MAP_BUFFER_SIZE;

	if(flags.frozen)
		return;
}

void Actor::draw()
{
}
