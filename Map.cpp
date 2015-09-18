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

bool Map::isBlocked(int cellX, int cellZ)
{
//  return mapBuffer[cellZ * MAP_SIZE + cellX] == '#';
	uint8_t tile = getTile(cellX, cellZ);
	return tile > 0 && tile != MAP_OUT_OF_BOUNDS;
  //return pgm_read_byte(mapData + cellZ * MAP_SIZE + cellX) != 0;// == '#';
//  return pgm_read_byte(mapBuffer + cellZ * MAP_SIZE + cellX) == '#';
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

void Map::updateHorizontalSlice(int offsetZ)
{
	uint8_t readBuffer[MAP_BUFFER_SIZE];

	streamData(readBuffer, MapRead_Horizontal, bufferX, bufferZ + offsetZ, MAP_BUFFER_SIZE);

	int targetZ	= (bufferZ + offsetZ) & 0xf;

	for(int x = 0; x < MAP_BUFFER_SIZE; x++)
	{
		int targetX = (bufferX + x) & 0xf;
		m_mapBuffer[targetZ * MAP_BUFFER_SIZE + targetX] = readBuffer[x];
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
		m_mapBuffer[targetZ * MAP_BUFFER_SIZE + targetX] = readBuffer[z];
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
