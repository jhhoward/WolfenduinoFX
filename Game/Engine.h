#ifndef ENGINE_H_
#define ENGINE_H_

#ifdef _WIN32
#include "../Windows/SDLPlatform.h"
#else
#define PLATFORM_GAMEBUINO 1
#endif

#ifdef PLATFORM_GAMEBUINO
#include "GamebuinoPlatform.h"
#endif

#include "Platform.h"
#include "Renderer.h"
#include "Player.h"
#include "Map.h"

class Engine
{
public:
	static void init();
	static void update();
	
	static Renderer renderer;
	static Player player;
	static Map map;
};

#endif
