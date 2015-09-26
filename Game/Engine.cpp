#include "Engine.h"

Renderer Engine::renderer;
Map Engine::map;
Player Engine::player;

void Engine::init()
{
//	player.x = player.z = 48;
	player.z = CELL_SIZE * (MAP_SIZE - 2);
	player.x = CELL_SIZE * MAP_SIZE / 2;
	renderer.init();
	map.init();
	player.init();
}

#include <math.h> // temp

void Engine::update()
{
	player.update();
	map.update();
	renderer.drawFrame();
}
