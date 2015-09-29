#ifndef MAP_H_
#define MAP_H_

#include "Engine.h"

#ifdef STANDARD_FILE_STREAMING
#include <stdio.h>
#endif

#define MAP_OUT_OF_BOUNDS 0xff

enum MapRead_Orientation
{
	MapRead_Horizontal,
	MapRead_Vertical
};

enum DoorType
{
	DoorType_None,
	DoorType_StandardHorizontal,
	DoorType_StandardVertical
};

enum DoorState
{
	DoorState_Idle = 0,
	DoorState_Opening,
	DoorState_Closing
};

#define DOOR_MAX_OPEN 63

class Door
{
public:
	Door() : type(DoorType_None) {}

	void update();

	DoorType type;
	int8_t x, z;
	uint8_t open : 6;
	uint8_t state : 2;
};

class Map
{
public:
	void init();
	bool isValid(int cellX, int cellZ);
	bool isBlocked(int cellX, int cellZ);
	bool isSolid(int cellX, int cellZ);
	bool isDoor(int cellX, int cellZ);
	uint8_t getTextureId(int cellX, int cellZ);
	uint8_t getTile(int cellX, int cellZ);
	uint8_t getTileFast(int cellX, int cellZ)
	{
		cellX &= 0xf;
		cellZ &= 0xf;
		return m_mapBuffer[cellZ * MAP_BUFFER_SIZE + cellX];
	}

	int bufferX;
	int bufferZ;
	Door doors[MAX_DOORS];

	uint8_t m_mapBuffer[MAP_BUFFER_SIZE * MAP_BUFFER_SIZE];

	void updateBufferPosition(int newX, int newZ);

	void update();
	void openDoorsAt(int x, int z);

	bool isItemCollected(uint8_t spawnId)
	{
		int index = spawnId / 8;
		int mask = 1 << (spawnId - (index * 8));
		return (m_itemState[index] & mask) != 0;
	}
	void markItemCollected(uint8_t spawnId)
	{
		int index = spawnId / 8;
		int mask = 1 << (spawnId - (index * 8));
		m_itemState[index] |= mask;
	}
	bool isActorKilled(uint8_t spawnId)
	{
		int index = spawnId / 8;
		int mask = 1 << (spawnId - (index * 8));
		return (m_actorState[index] & mask) != 0;
	}
	void markActorKilled(uint8_t spawnId)
	{
		int index = spawnId / 8;
		int mask = 1 << (spawnId - (index * 8));
		m_actorState[index] |= mask;
	}

private:
	void streamData(uint8_t* buffer, MapRead_Orientation orientation, int x, int z, int length);
	void updateHorizontalSlice(int offsetZ);
	void updateVerticalSlice(int offsetX);
	void updateEntireBuffer();
	void updateDoors();
	uint8_t streamIn(uint8_t tile, uint8_t metadata, int x, int z);
	void streamInDoor(DoorType type, int x, int z);
	
	uint8_t m_itemState[256 / 8];
	uint8_t m_actorState[256 / 8];

	uint8_t m_streamBuffer[MAP_BUFFER_SIZE * 2];
#ifdef STANDARD_FILE_STREAMING
	FILE* m_mapStream;
#endif
};

#endif
