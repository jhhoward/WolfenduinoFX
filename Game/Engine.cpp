#include "Engine.h"

Engine engine;

void Engine::init()
{
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		actors[n].type = ActorType_Empty;
		actors[n].spawnId = 0xff;
	}
//	player.x = player.z = 48;
	player.z = CELL_SIZE * (MAP_SIZE - 2);
	player.x = CELL_SIZE * MAP_SIZE / 2;
	renderer.init();
	map.init();
	player.init();
}

void Engine::update()
{
	player.update();
	map.update();

	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		actors[n].update();
	}

	renderer.drawFrame();
}

Actor* Engine::spawnActor(uint8_t spawnId, uint8_t actorType, int8_t cellX, int8_t cellZ)
{
	// Check if actor was already killed
	if(map.isActorKilled(spawnId))
		return NULL;

	// Check for existing actor
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(actors[n].spawnId == spawnId)
		{
			return NULL;
		}
	}

	// Find an empty slot
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(actors[n].type == ActorType_Empty)
		{
			actors[n].init(spawnId, actorType, cellX, cellZ);
			return &actors[n];
		}
	}

	// Take over an existing slot that is currently frozen
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(actors[n].flags.frozen && !actors[n].flags.persistent)
		{
			actors[n].init(spawnId, actorType, cellX, cellZ);
			return &actors[n];
		}
	}
	return NULL;
}
