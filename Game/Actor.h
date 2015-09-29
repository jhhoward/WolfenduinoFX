#ifndef ACTOR_H_
#define ACTOR_H_

#include <stdint.h>

enum ActorType
{
	ActorType_Empty,
	ActorType_Guard
};

class Actor
{
public:
	void init(uint8_t spawnId, uint8_t actorType, uint8_t cellX, uint8_t cellZ);
	void update();
	void draw();

	void updateFrozenState();

	uint8_t spawnId;
	uint8_t type;
	int16_t x, z;
	uint8_t state;
	uint8_t frame;
	int16_t hp;

	struct
	{
		bool persistent : 1;
		bool frozen : 1;
		bool alive : 1;
	} flags;
};

#endif
