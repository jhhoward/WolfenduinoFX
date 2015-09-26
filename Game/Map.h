#ifndef MAP_H_
#define MAP_H_

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

	void init();
	void update();
	void openDoorsAt(int x, int z);

private:
	void streamData(uint8_t* buffer, MapRead_Orientation orientation, int x, int z, int length);
	void updateHorizontalSlice(int offsetZ);
	void updateVerticalSlice(int offsetX);
	void updateEntireBuffer();
	void updateDoors();
	uint8_t streamIn(uint8_t tile, int x, int z);
	void streamInDoor(DoorType type, int x, int z);
	
};

#endif
