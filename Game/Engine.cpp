#include "Engine.h"

Engine engine;

void Engine::init()
{
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		actors[n].type = ActorType_Empty;
		actors[n].spawnId = 0xff;
	}

	renderer.init();
	map.init();
	player.init();

	frameCount = 0;
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

	frameCount ++;
}

Actor* Engine::spawnActor(uint8_t spawnId, uint8_t actorType, int8_t cellX, int8_t cellZ)
{
#if 1
	// Check for existing actor
	/*for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(actors[n].spawnId == spawnId)
		{
			return NULL;
		}
	}*/

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

	WARNING("Could not find a slot for new actor\n");
#endif
	return NULL;
}
