#include "Engine.h"
#include "Map.h"

#include <stdio.h>

#include "map0.inc.h"

bool Map::isValid(int x, int z)
{
	if(x < bufferX || z < bufferZ || x >= bufferX + MAP_BUFFER_SIZE || z >= bufferZ + MAP_BUFFER_SIZE)
	{
		return false;
	}

	return true;
/*  if (cellX < 0)
    return false;
  if (cellX >= MAP_SIZE)
    return false;
  if (cellZ < 0)
    return false;
  if (cellZ >= MAP_SIZE)
    return false;
  return true;*/
}

void Map::init()
{
	bufferX = 0;
	bufferZ = 0;
}

void Map::update()
{
	updateDoors();
}

bool Map::isDoor(int cellX, int cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	return tile == 0x5a || tile == 0x5b;
}

bool Map::isBlocked(int cellX, int cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	if((tile > 0 && tile < 22) || tile == MAP_OUT_OF_BOUNDS)
		return true;

	for(int n = 0; n < MAX_DOORS; n++)
	{
		if(doors[n].type != DoorType_None && doors[n].x == cellX && doors[n].z == cellZ && doors[n].open < 16)
		{
			return true;
		}
	}

	return false;
}

bool Map::isSolid(int cellX, int cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	return tile > 0 && tile < 22 && tile != MAP_OUT_OF_BOUNDS;
}

uint8_t Map::getTextureId(int cellX, int cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	if(tile == MAP_OUT_OF_BOUNDS)
		return 0;
	return tile - 1;
}

uint8_t Map::getTile(int x, int z)
{
	if(x < bufferX || z < bufferZ || x >= bufferX + MAP_BUFFER_SIZE || z >= bufferZ + MAP_BUFFER_SIZE)
	{
		return MAP_OUT_OF_BOUNDS;
	}
	
	x &= 0xf;
	z &= 0xf;
	return m_mapBuffer[z * MAP_BUFFER_SIZE + x];
}

void Map::streamData(uint8_t* buffer, MapRead_Orientation orientation, int x, int z, int length)
{
	//printf("Streaming %s, %d, %d\n", orientation == MapRead_Horizontal ? "Horizontal" : "Vertical", x, z, length);
	// TODO: make this stream from SD card or decompress from huffman stream in progmem
	if(orientation == MapRead_Horizontal)
	{
		for(int n = 0; n < length; n++)
		{
			buffer[n] = pgm_read_byte(&mapData[z * MAP_SIZE + x + n]);
		}
	}
	else
	{
		for(int n = 0; n < length; n++)
		{
			buffer[n] = pgm_read_byte(&mapData[(z + n) * MAP_SIZE + x]);
		}
	}	
}

uint8_t Map::streamIn(uint8_t tile, int x, int z)
{
	if(tile == 0x5b)
	{
		streamInDoor(DoorType_StandardHorizontal, x, z);
		//return 0;
	}
	if(tile == 0x5a)
	{
		streamInDoor(DoorType_StandardVertical, x, z);
		//return 0;
	}

	return tile;
}

void Map::updateHorizontalSlice(int offsetZ)
{
	uint8_t readBuffer[MAP_BUFFER_SIZE];

	streamData(readBuffer, MapRead_Horizontal, bufferX, bufferZ + offsetZ, MAP_BUFFER_SIZE);

	int targetZ	= (bufferZ + offsetZ) & 0xf;

	for(int x = 0; x < MAP_BUFFER_SIZE; x++)
	{
		int targetX = (bufferX + x) & 0xf;
		uint8_t read = streamIn(readBuffer[x], bufferX + x, bufferZ + offsetZ);

		m_mapBuffer[targetZ * MAP_BUFFER_SIZE + targetX] = read;
	}
}

void Map::updateVerticalSlice(int offsetX)
{
	uint8_t readBuffer[MAP_BUFFER_SIZE];
	
	streamData(readBuffer, MapRead_Vertical, bufferX + offsetX, bufferZ, MAP_BUFFER_SIZE);

	int targetX = (bufferX + offsetX) & 0xf;

	for(int z = 0; z < MAP_BUFFER_SIZE; z++)
	{
		int targetZ = (bufferZ + z) & 0xf;
		uint8_t read = streamIn(readBuffer[z], bufferX + offsetX, bufferZ + z);

		m_mapBuffer[targetZ * MAP_BUFFER_SIZE + targetX] = read;
	}
}

void Map::updateEntireBuffer()
{
	for(int n = 0; n < MAP_BUFFER_SIZE; n++)
	{
		updateHorizontalSlice(n);
	}
}

void Map::updateBufferPosition(int newX, int newZ)
{
	if(newX < 0)
		newX = 0;
	if(newZ < 0)
		newZ = 0;
	if(newX > MAP_SIZE - MAP_BUFFER_SIZE)
		newX = MAP_SIZE - MAP_BUFFER_SIZE;
	if(newZ > MAP_SIZE - MAP_BUFFER_SIZE)
		newZ = MAP_SIZE - MAP_BUFFER_SIZE;
	
	if(bufferX == newX && bufferZ == newZ)
		return;
		
	if(newX <= bufferX - MAP_BUFFER_SIZE || newX >= bufferX + MAP_BUFFER_SIZE
	|| newZ <= bufferZ - MAP_BUFFER_SIZE || newZ >= bufferZ + MAP_BUFFER_SIZE)
	{
		bufferX = newX;
		bufferZ = newZ;
		updateEntireBuffer();
		return;
	}

	while(bufferX < newX)
	{
		bufferX ++;
		updateVerticalSlice(MAP_BUFFER_SIZE - 1);
	}
	while(bufferX > newX)
	{
		bufferX --;
		updateVerticalSlice(0);
	}
	while(bufferZ < newZ)
	{
		bufferZ ++;
		updateHorizontalSlice(MAP_BUFFER_SIZE - 1);
	}
	while(bufferZ > newZ)
	{
		bufferZ --;
		updateHorizontalSlice(0);
	}
}

void Map::updateDoors()
{
	for(int n = 0; n < MAX_DOORS; n++)
	{
		if(doors[n].type != DoorType_None)
		{
			doors[n].update();
		}
	}
}

void Map::streamInDoor(DoorType type, int x, int z)
{
	int freeIndex = -1;

	for(int n = 0; n < MAX_DOORS; n++)
	{
		if(freeIndex == -1 && doors[n].type == DoorType_None)
		{
			freeIndex = n;
		}
		if(doors[n].type != DoorType_None && doors[n].x == x && doors[n].z == z)
		{
			// Already streamed in
			return;
		}
	}

	if(freeIndex == -1)
	{
		for(int n = 0; n < MAX_DOORS; n++)
		{
			if(doors[n].x < bufferX || doors[n].x >= bufferX + MAP_BUFFER_SIZE
			|| doors[n].z < bufferZ || doors[n].z >= bufferZ + MAP_BUFFER_SIZE)
			{
				freeIndex = n;
				break;
			}
		}
	}

	if(freeIndex == -1)
	{
#ifdef _WIN32
		printf("No room to spawn door!\n");
#endif
		return;
	}

	doors[freeIndex].x = x;
	doors[freeIndex].z = z;
	doors[freeIndex].open = 0;
	doors[freeIndex].state = DoorState_Idle;
	doors[freeIndex].type = type;
}

void Map::openDoorsAt(int x, int z)
{
	if(!isDoor(x, z))
		return;

	for(int n = 0; n < MAX_DOORS; n++)
	{
		if(doors[n].type != DoorType_None && doors[n].x == x && doors[n].z == z)
		{
			doors[n].state = DoorState_Opening;
			return;
		}
	}
}


void Door::update()
{
	switch(state)
	{
	case DoorState_Opening:
		if(open < DOOR_MAX_OPEN)
		{
			open ++;
		}
		else state = DoorState_Closing;
		break;
	case DoorState_Closing:
		if(open > 0)
		{
			open --;
		}
		else state = DoorState_Idle;
		break;
	}
}
