#include "Engine.h"

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

void Engine::startLevel(bool resetPlayer)
{
	if(resetPlayer)
	{
		player.hp = 0;
	}
	gameState = GameState_StartingLevel;
}

void Engine::startingLevel()
{
#ifdef _WIN32
	SDL_Delay(1000);
#endif
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
		map.currentLevel++;
		startLevel(false);
		break;
	case GameState_StartingLevel:
		startingLevel();
		break;
	case GameState_Dead:
		if(frameCount >= 30)
		{
			startLevel();
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
			clearDisplay(1);
			renderer.drawString(PSTR("GET PSYCHED!"), 20, 21);
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
#if 1
	// Check for existing actor
	for(int n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(actors[n].spawnId == spawnId)
		{
			return nullptr;
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
	return nullptr;
}
