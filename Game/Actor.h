#ifndef ACTOR_H_
#define ACTOR_H_

#include <stdint.h>

enum ActorType
{
	ActorType_Empty,
	ActorType_Guard
};

enum ActorState
{
	ActorState_Idle,
	ActorState_Active,
	ActorState_Injured,
	ActorState_Dying,
	ActorState_Dead
};

class Actor
{
public:
	void init(uint8_t spawnId, uint8_t actorType, uint8_t cellX, uint8_t cellZ);
	void update();
	void draw();
	void damage(int amount);

	void switchState(uint8_t newState);
	void updateFrozenState();

	bool tryMove();
	void pickNewTargetCell();

	void dropItem(uint8_t itemType);
	bool tryDropItem(uint8_t itemType, int cellX, int cellZ);

	uint8_t spawnId;
	uint8_t type;
	int16_t x, z;
	uint8_t state;
	uint8_t frame;
	int16_t hp;

	uint8_t targetCellX, targetCellZ;

	struct
	{
		bool persistent : 1;
		bool frozen : 1;
		bool alive : 1;
	} flags;
};

#endif
