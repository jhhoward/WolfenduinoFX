#ifndef ENGINE_H_
#define ENGINE_H_

#ifdef _WIN32
#include "../Windows/SDLPlatform.h"
#elif !defined(PLATFORM_UZEBOX)
#define PLATFORM_GAMEBUINO 1
#endif

#ifdef PLATFORM_GAMEBUINO
#include "GamebuinoPlatform.h"
#endif

#ifdef PLATFORM_UZEBOX
#include "UzeboxPlatform.h"
#endif

#include "Platform.h"
#include "Renderer.h"
#include "Player.h"
#include "Map.h"
#include "Actor.h"

class Engine
{
public:
	void init();
	void update();
	Actor* spawnActor(uint8_t spawnId, uint8_t actorType, int8_t cellX, int8_t cellZ);
	
	Renderer renderer;
	Player player;
	Map map;
	Actor actors[MAX_ACTIVE_ACTORS];
};

extern Engine engine;

#endif
