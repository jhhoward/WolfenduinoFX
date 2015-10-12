#include "Engine.h"

Engine engine;

void Engine::init()
{
	menu.init();
	difficulty = Difficulty_Medium;
	map.initStreaming();
	gameState = GameState_Menu;
}

void Engine::startLevel()
{
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		actors[n].type = ActorType_Empty;
		actors[n].spawnId = 0xff;
	}

	map.init();
	renderer.init();
	player.init();

	frameCount = 0;
	gameState = GameState_Playing;
}

void Engine::update()
{
	if(gameState == GameState_Playing)
	{
		player.update();

		if(player.hp > 0)
		{
			map.update();
			for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
			{
				actors[n].update();
			}
		}
	}
	else if(gameState == GameState_Menu)
	{
		menu.update();
	}

	frameCount ++;
}

void Engine::draw()
{
	if(gameState == GameState_Menu)
	{
		menu.draw();
	}
	else if(gameState == GameState_Playing)
	{
		renderer.drawFrame();
	}
	else if(gameState == GameState_Dead)
	{
		if(frameCount < 30)
		{
			for(int n = 0; n < DISPLAYWIDTH * (DISPLAYHEIGHT / 15); n++)
			{
				uint8_t x = (n + getRandomNumber16() + getRandomNumber16()) % DISPLAYWIDTH;
				uint8_t y = (n + getRandomNumber16() + getRandomNumber16()) % DISPLAYHEIGHT;

				setPixel(x, y);
			}
		}
		else
		{
			for(uint8_t x = 0; x < DISPLAYWIDTH; x++)
			{
				for(uint8_t y = 0; y < DISPLAYHEIGHT; y++)
				{
					setPixel(x, y);
				}
			}
			startLevel();
		}
	}
}

Actor* Engine::spawnActor(uint8_t spawnId, uint8_t actorType, int8_t cellX, int8_t cellZ)
{
#if 1
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

	WARNING("Could not find a slot for new actor\n");
#endif
	return NULL;
}
