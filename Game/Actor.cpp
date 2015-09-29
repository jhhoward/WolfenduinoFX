#include "Engine.h"
#include "Actor.h"

#include "Data_Guard.h"

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

	updateFrozenState();

	if(flags.frozen)
		return;

	if((engine.frameCount & 0x3) == 0)
		frame = (frame + 1) % 4;
}

void Actor::draw()
{
	int offset = frame == 3 ? 1 : frame;

	engine.renderer.queueSprite((uint8_t*) Data_guardSprite + TEXTURE_STRIDE * TEXTURE_SIZE * offset, x, z);
}

void Actor::updateFrozenState()
{
	int cellX = x / CELL_SIZE;
	int cellZ = z / CELL_SIZE;

	flags.frozen = cellX < engine.map.bufferX || cellZ < engine.map.bufferZ || cellX >= engine.map.bufferX + MAP_BUFFER_SIZE || cellZ >= engine.map.bufferZ + MAP_BUFFER_SIZE;
}