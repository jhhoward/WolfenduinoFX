#include "Engine.h"
#include "TileTypes.h"

Engine engine;

void Engine::init()
{
	menu.init();
	difficulty = Difficulty_Medium;
	map.initStreaming();
	gameState = GameState_Menu;
	map.currentLevel = 0;

	//map.currentLevel = 1;
	//gameState = GameState_StartingLevel;
	// hacks
	//map.currentLevel = 2;
	//difficulty = Difficulty_Baby;
	//startLevel();
	//player.x = 650;
	//player.z = 1514;
	/*player.x = 1121;
	player.z = 730;
	player.x = 1106;
	player.z = 835;
	player.direction = DEGREES_180;*/
}

void Engine::startNewGame()
{
	map.currentLevel = 0;
	player.lives = 3;
	startLevel(false);
}

void Engine::startLevel(bool resetPlayer)
{
	if(resetPlayer)
	{
		player.hp = 0;
	}
	player.inventory.hasKey1 = false;
	player.inventory.hasKey2 = false;
	gameState = GameState_StartingLevel;
	engine.frameCount = 0;
}

void Engine::startingLevel()
{
	gameState = GameState_Loading;
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		actors[n].type = ActorType_Empty;
		actors[n].spawnId = 0xff;
	}

	map.init();
	renderer.init();
	player.init();
	player.update(); // To update streaming position for first frame

	frameCount = 0;
	gameState = GameState_Playing;
	map.updateEntireBuffer();	// Do this to spawn enemies immediately around the player
}

void Engine::update()
{
	switch(gameState)
	{
	case GameState_Playing:
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

			if(Platform.readInput() & Input_Btn_C)
			{
				gameState = GameState_PauseMenu;
				engine.menu.switchMenu((MenuData*) Menu_Paused);
			}
		}
		break;
	case GameState_Menu:
	case GameState_PauseMenu:
		{
			menu.update();
		}
		break;
	case GameState_FinishedLevel:
		{
			if (map.getTile(WORLD_TO_CELL(engine.player.x), WORLD_TO_CELL(engine.player.z)) == Tile_SecretExit)
			{
				map.currentLevel = 9;
			}
			else if (map.currentLevel < 8)
			{
				map.currentLevel++;
			}
			else if (map.currentLevel == 9)
			{
				map.currentLevel = 1;
			}
			startLevel(false);
		}
		break;
	case GameState_StartingLevel:
		if (frameCount > TARGET_FRAMERATE * 2)
		{
			startingLevel();
		}
		break;
	case GameState_Dead:
		if(frameCount >= 30)
		{
			if (engine.player.lives == 0)
			{
				gameState = GameState_Menu;
				engine.menu.init();
			}
			else
			{
				startLevel();
			}
		}
		break;
	}

	frameCount ++;
}

void Engine::draw()
{
	switch(gameState)
	{
	case GameState_Menu:
	case GameState_PauseMenu:
		{
			menu.draw();
		}
		break;
	case GameState_Playing:
		{
			renderer.drawFrame();
		}
		break;
	case GameState_StartingLevel:
	case GameState_Loading:
		{
			renderer.drawLevelLoadScreen();
		}
		break;
	case GameState_Dead:
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
				clearDisplay(0);
			}
		}
		break;
	}
}

Actor* Engine::spawnActor(uint8_t spawnId, uint8_t actorType, int8_t cellX, int8_t cellZ)
{
	// Check for existing actor
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(actors[n].spawnId == spawnId)
		{
			return &actors[n];;
		}
	}

	static uint8_t firstIndex = 0;
	int8_t freeIndex = -1;

	// First try to find an empty slot
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		uint8_t index = (firstIndex + n) % MAX_ACTIVE_ACTORS;
		if(actors[index].type == ActorType_Empty)
		{
			freeIndex = index;
			break;
		}
	}

	if (freeIndex == -1)
	{
		// Take over an existing slot that is currently frozen and dead
		for (int n = 0; n < MAX_ACTIVE_ACTORS; n++)
		{
			uint8_t index = (firstIndex + n) % MAX_ACTIVE_ACTORS;
			if (actors[index].flags.frozen && actors[index].hp == 0)
			{
				freeIndex = index;
				break;
			}
		}
	}

	if (freeIndex == -1)
	{
		// Take over an existing slot that is currently frozen
		for (int n = 0; n < MAX_ACTIVE_ACTORS; n++)
		{
			uint8_t index = (firstIndex + n) % MAX_ACTIVE_ACTORS;
			if (actors[index].flags.frozen)
			{
				freeIndex = index;
				break;
			}
		}
	}

	if (freeIndex == -1)
	{
		// Take over an existing slot that is currently dead
		for (int n = 0; n < MAX_ACTIVE_ACTORS; n++)
		{
			uint8_t index = (firstIndex + n) % MAX_ACTIVE_ACTORS;
			if (actors[index].hp == 0)
			{
				freeIndex = index;
				break;
			}
		}
	}

	if (freeIndex != -1)
	{
		actors[freeIndex].init(spawnId, actorType, cellX, cellZ);
		
		// By changing the first index for next time, we can keep corpses around for longer, otherwise the same slot keeps getting reused
		firstIndex = (freeIndex + 1) % MAX_ACTIVE_ACTORS;

		return &actors[freeIndex];
	}

	WARNING("Could not find a slot for new actor\n");
	return nullptr;
}
