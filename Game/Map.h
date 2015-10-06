#ifndef MAP_H_
#define MAP_H_

#include "Engine.h"
#include "Defines.h"

#ifdef STANDARD_FILE_STREAMING
#include <stdio.h>
#endif
#ifdef PETIT_FATFS_FILE_STREAMING
#include <petit_fatfs.h>
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

	uint8_t type;
	int8_t x, z;
	uint8_t open : 6;
	uint8_t state : 2;
};

class Item
{
public:
	uint8_t type;
	int8_t x, z;
	uint8_t spawnId;
};

class Map
{
public:
	void init();
	bool isValid(int8_t cellX, int8_t cellZ);
	bool isBlocked(int8_t cellX, int8_t cellZ);
	bool isSolid(int8_t cellX, int8_t cellZ);
	bool isDoor(int8_t cellX, int8_t cellZ);
	uint8_t getTextureId(int8_t cellX, int8_t cellZ);
	uint8_t getTile(int8_t cellX, int8_t cellZ);
	uint8_t getTileFast(int8_t cellX, int8_t cellZ)
	{
		cellX &= 0xf;
		cellZ &= 0xf;
		return m_mapBuffer[cellZ * MAP_BUFFER_SIZE + cellX];
	}

	int8_t bufferX;
	int8_t bufferZ;
	Door doors[MAX_DOORS];
	Item items[MAX_ACTIVE_ITEMS];

	uint8_t m_mapBuffer[MAP_BUFFER_SIZE * MAP_BUFFER_SIZE];

	void updateBufferPosition(int8_t newX, int8_t newZ);

	void update();
	void openDoorsAt(int8_t x, int8_t z);
	bool placeItem(uint8_t type, int8_t x, int8_t z, uint8_t spawnId);

	bool isItemCollected(uint8_t spawnId)
	{
		uint8_t index = spawnId / 8;
		uint8_t mask = 1 << (spawnId - (index * 8));
		return (m_itemState[index] & mask) != 0;
	}
	void markItemCollected(uint8_t spawnId)
	{
		uint8_t index = spawnId / 8;
		uint8_t mask = 1 << (spawnId - (index * 8));
		m_itemState[index] |= mask;
	}
	bool isActorKilled(uint8_t spawnId)
	{
		uint8_t index = spawnId / 8;
		uint8_t mask = 1 << (spawnId - (index * 8));
		return (m_actorState[index] & mask) != 0;
	}
	void markActorKilled(uint8_t spawnId)
	{
		uint8_t index = spawnId / 8;
		uint8_t mask = 1 << (spawnId - (index * 8));
		m_actorState[index] |= mask;
	}

	bool isClearLine(int16_t x1, int16_t z1, int16_t x2, int16_t z2);

private:
	void streamData(uint8_t* buffer, MapRead_Orientation orientation, int8_t x, int8_t z, int8_t length);
	void updateHorizontalSlice(int8_t offsetZ);
	void updateVerticalSlice(int8_t offsetX);
	void updateEntireBuffer();
	void updateDoors();
	uint8_t streamIn(uint8_t tile, uint8_t metadata, int8_t x, int8_t z);
	void streamInDoor(DoorType type, int8_t x, int8_t z);
	
	uint8_t m_itemState[256 / 8];
	uint8_t m_actorState[256 / 8];

	uint8_t m_streamBuffer[MAP_BUFFER_SIZE * 2];
#ifdef STANDARD_FILE_STREAMING
	FILE* m_mapStream;
#endif
#ifdef PETIT_FATFS_FILE_STREAMING
	FATFS m_fileSystem;
	bool m_mapLoaded;
#endif

};

#endif
