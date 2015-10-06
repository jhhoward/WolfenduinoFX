#include "Engine.h"
#include "Map.h"
#include "TileTypes.h"

#include <stdio.h>

#ifdef PROGMEM_MAP_STREAMING
#include "Data_Maps.h"
#endif
#ifdef PETIT_FATFS_FILE_STREAMING
#include <petit_fatfs.h>
#endif

bool Map::isValid(int8_t x, int8_t z)
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
#ifdef STANDARD_FILE_STREAMING
	fopen_s(&m_mapStream, "wolf3d.dat", "rb");
#endif
#ifdef PETIT_FATFS_FILE_STREAMING
	m_mapLoaded = false;
	if(pf_mount(&m_fileSystem) == FR_OK)
	{
		if(pf_open("WOLF3D.DAT") == FR_OK)
		{
			m_mapLoaded = true;
		}
	}
#endif
	bufferX = 0;
	bufferZ = 0;
}

void Map::update()
{
	updateDoors();
}

bool Map::isDoor(int8_t cellX, int8_t cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	return tile >= Tile_FirstDoor && tile <= Tile_LastDoor;
}

bool Map::isBlocked(int8_t cellX, int8_t cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);

	// Check if this is a wall
	if((tile >= Tile_FirstWall && tile <= Tile_LastWall))
		return true;

	// Check if this is a blocking decoration
	if((tile >= Tile_FirstBlockingDecoration && tile <= Tile_LastBlockingDecoration))
		return true;

	// Check if the door is closed
	for(int8_t n = 0; n < MAX_DOORS; n++)
	{
		if(doors[n].type != DoorType_None && doors[n].x == cellX && doors[n].z == cellZ && doors[n].open < 16)
		{
			return true;
		}
	}

	return false;
}

bool Map::isSolid(int8_t cellX, int8_t cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	return tile >= Tile_FirstWall && tile <= Tile_LastWall && tile != MAP_OUT_OF_BOUNDS;
}

uint8_t Map::getTextureId(int8_t cellX, int8_t cellZ)
{
	uint8_t tile = getTile(cellX, cellZ);
	if(tile == MAP_OUT_OF_BOUNDS)
		return 0;
	return tile - 1;
}

uint8_t Map::getTile(int8_t x, int8_t z)
{
	if(x < bufferX || z < bufferZ || x >= bufferX + MAP_BUFFER_SIZE || z >= bufferZ + MAP_BUFFER_SIZE)
	{
		return MAP_OUT_OF_BOUNDS;
	}
	
	return getTileFast(x, z);
}

void Map::streamData(uint8_t* buffer, MapRead_Orientation orientation, int8_t x, int8_t z, int8_t length)
{
#ifdef STANDARD_FILE_STREAMING
	if(m_mapStream)
	{
		int32_t offset = orientation == MapRead_Horizontal ? (z * MAP_SIZE + x) * 2 : (MAP_SIZE * MAP_SIZE * 2) + (x * MAP_SIZE + z) * 2;
		fseek(m_mapStream, offset, SEEK_SET);
		fread(buffer, 1, length * 2, m_mapStream);
		return;
	}
#endif
#ifdef PETIT_FATFS_FILE_STREAMING
	if(m_mapLoaded)
	{
		int32_t offset = orientation == MapRead_Horizontal ? (z * MAP_SIZE + x) * 2 : (MAP_SIZE * MAP_SIZE * 2) + (x * MAP_SIZE + z) * 2;
		WORD bytesRead;
		pf_lseek(offset);
		pf_read(buffer, length * 2, &bytesRead);
	}
#endif
	//printf("Streaming %s, %d, %d\n", orientation == MapRead_Horizontal ? "Horizontal" : "Vertical", x, z, length);
	// TODO: make this stream from SD card or decompress from huffman stream in progmem

#ifdef PROGMEM_MAP_STREAMING
	if(orientation == MapRead_Horizontal)
	{
		for(int8_t n = 0; n < length; n++)
		{
			buffer[n * 2] = pgm_read_byte(&mapData[z * MAP_SIZE + x + n]);
			buffer[n * 2 + 1] = 0xff;
		}
	}
	else
	{
		for(int8_t n = 0; n < length; n++)
		{
			buffer[n * 2] = pgm_read_byte(&mapData[(z + n) * MAP_SIZE + x]);
			buffer[n * 2 + 1] = 0xff;
		}
	}
#endif
}

uint8_t Map::streamIn(uint8_t tile, uint8_t metadata, int8_t x, int8_t z)
{
	if(tile >= Tile_FirstDoor && tile <= Tile_LastDoor)
	{
		//if(tile & 0x1)
		if(tile == Tile_Door_Generic_Horizontal)
		{
			streamInDoor(DoorType_StandardHorizontal, x, z);
		}
		else if(tile == Tile_Door_Generic_Vertical)
		{
			streamInDoor(DoorType_StandardVertical, x, z);
		}
	}
	else if(tile >= Tile_FirstItem && tile <= Tile_LastItem)
	{
		if(!isItemCollected(metadata))
		{
			placeItem(tile, x, z, metadata);
		}
		return Tile_Empty;
	}
	else if(tile >= Tile_FirstActor && tile <= Tile_LastActor)
	{
		if(!isActorKilled(metadata))
		{
			engine.spawnActor(metadata, tile, x, z);
		}
		return Tile_Empty;
	}

	return tile;
}

void Map::updateHorizontalSlice(int8_t offsetZ)
{
	streamData(m_streamBuffer, MapRead_Horizontal, bufferX, bufferZ + offsetZ, MAP_BUFFER_SIZE);

	int8_t targetZ	= (bufferZ + offsetZ) & 0xf;

	for(int8_t x = 0; x < MAP_BUFFER_SIZE; x++)
	{
		int8_t targetX = (bufferX + x) & 0xf;
		uint8_t read = streamIn(m_streamBuffer[x * 2], m_streamBuffer[x * 2 + 1], bufferX + x, bufferZ + offsetZ);

		m_mapBuffer[targetZ * MAP_BUFFER_SIZE + targetX] = read;
	}
}

void Map::updateVerticalSlice(int8_t offsetX)
{
	streamData(m_streamBuffer, MapRead_Vertical, bufferX + offsetX, bufferZ, MAP_BUFFER_SIZE);

	int8_t targetX = (bufferX + offsetX) & 0xf;

	for(int8_t z = 0; z < MAP_BUFFER_SIZE; z++)
	{
		int8_t targetZ = (bufferZ + z) & 0xf;
		uint8_t read = streamIn(m_streamBuffer[z * 2], m_streamBuffer[z * 2 + 1], bufferX + offsetX, bufferZ + z);

		m_mapBuffer[targetZ * MAP_BUFFER_SIZE + targetX] = read;
	}
}

void Map::updateEntireBuffer()
{
	for(int8_t n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(engine.actors[n].type != ActorType_Empty)
		{
			engine.actors[n].updateFrozenState();
		}
	}

	for(int8_t n = 0; n < MAP_BUFFER_SIZE; n++)
	{
		updateHorizontalSlice(n);
	}
}

void Map::updateBufferPosition(int8_t newX, int8_t newZ)
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
	for(int8_t n = 0; n < MAX_DOORS; n++)
	{
		if(doors[n].type != DoorType_None)
		{
			doors[n].update();
		}
	}
}

void Map::streamInDoor(DoorType type, int8_t x, int8_t z)
{
	int8_t freeIndex = -1;

	for(int8_t n = 0; n < MAX_DOORS; n++)
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
		for(int8_t n = 0; n < MAX_DOORS; n++)
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
		WARNING("No room to spawn door!\n");
		return;
	}

	doors[freeIndex].x = x;
	doors[freeIndex].z = z;
	doors[freeIndex].open = 0;
	doors[freeIndex].state = DoorState_Idle;
	doors[freeIndex].type = type;
}

void Map::openDoorsAt(int8_t x, int8_t z)
{
	if(!isDoor(x, z))
		return;

	for(int8_t n = 0; n < MAX_DOORS; n++)
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

bool Map::isClearLine(int16_t x1, int16_t z1, int16_t x2, int16_t z2)
/*{
	int cellX = x1 / CELL_SIZE;
	int cellZ = z1 / CELL_SIZE;
	int targetCellX = x2 / CELL_SIZE;
	int targetCellZ = z2 / CELL_SIZE;
	int16_t x = x1;
	int16_t z = z1;
	int16_t dx = x2 - x1;
	int16_t dz = z2 - z1;
	int16_t tx, tz;

	while(1)
	{
		if(cellX == targetCellX && cellZ == targetCellZ)
			return true;

		if(dx < 0)
		{
			tx = (x - cellX * CELL_SIZE) / -dx;
		}
		else if(dx > 0)
		{
			tx = ((cellX + 1) * CELL_SIZE - x) / dx;
		}
		else tx = 0;

		if(dz < 0)
		{
			tz = (z - cellZ * CELL_SIZE) / -dz;
		}
		else if(dz > 0)
		{
			tz = ((cellZ + 1) * CELL_SIZE - z) / dz;
		}
		else tz = 0;

	}

	return true;
}
*/
{
/*    int         x1,y1,xt1,yt1,x2,y2,xt2,yt2;
    int         x,y;
    int         xdist,ydist,xstep,ystep;
    int         partial,delta;
    int32_t     ltemp;
    int         xfrac,yfrac,deltafrac;
    unsigned    value,intercept;
	*/
	int cellX1 = x1 / CELL_SIZE;
	int cellX2 = x2 / CELL_SIZE;
	int cellZ1 = z1 / CELL_SIZE;
	int cellZ2 = z2 / CELL_SIZE;

    int xdist = mabs(cellX2 - cellX1);

	int partial, delta;
	int deltafrac;
	int xfrac, zfrac;
	int xstep, zstep;
	int32_t ltemp;
	int x, z;

    if (xdist > 0)
    {
        if (cellX2 > cellX1)
        {
            partial = ((cellX1 + 1) * CELL_SIZE - x1);
            xstep = 1;
        }
        else
        {
            partial = (x1 - cellX1 * CELL_SIZE);
            xstep = -1;
        }

        deltafrac = mabs(x2 - x1);
        delta = z2 - z1;
        ltemp = ((int32_t)delta * CELL_SIZE) / deltafrac;
        if (ltemp > 0x7fffl)
            zstep = 0x7fff;
        else if (ltemp < -0x7fffl)
            zstep = -0x7fff;
        else
            zstep = ltemp;
        zfrac = z1 + (((int32_t)zstep*partial) / CELL_SIZE);

        x = cellX1 + xstep;
        cellX2 += xstep;
        do
        {
            z = zfrac / CELL_SIZE;
            zfrac += zstep;

            uint8_t tile = getTile(x, z);
            x += xstep;

            if (!tile)
                continue;

            if (tile >= Tile_FirstWall && tile <= Tile_LastWall)
                return false;

            //
            // see if the door is open enough
            //
            /*value &= ~0x80;
            intercept = yfrac-ystep/2;

            if (intercept>doorposition[value])
                return false;*/

        } while (x != cellX2);
    }

    int zdist = mabs(cellZ2 - cellZ1);

    if (zdist > 0)
    {
        if (cellZ2 > cellZ1)
        {
            partial = ((cellZ1 + 1) * CELL_SIZE - z1);
            zstep = 1;
        }
        else
        {
            partial = (z1 - cellZ1 * CELL_SIZE);
            zstep = -1;
        }

        deltafrac = mabs(z2 - z1);
        delta = x2 - x1;
        ltemp = ((int32_t)delta * CELL_SIZE)/deltafrac;
        if (ltemp > 0x7fffl)
            xstep = 0x7fff;
        else if (ltemp < -0x7fffl)
            xstep = -0x7fff;
        else
            xstep = ltemp;
        xfrac = x1 + (((int32_t)xstep*partial) / CELL_SIZE);

        z = cellZ1 + zstep;
        cellZ2 += zstep;
        do
        {
            x = xfrac / CELL_SIZE;
            xfrac += xstep;

            uint8_t tile = getTile(x, z);
            z += zstep;

            if (!tile)
                continue;

            if (tile >= Tile_FirstWall && tile <= Tile_LastWall)
                return false;

            //
            // see if the door is open enough
            //
            /*value &= ~0x80;
            intercept = xfrac-xstep/2;

            if (intercept>doorposition[value])
                return false;*/
        } while (z != cellZ2);
    }

    return true;
}

bool Map::placeItem(uint8_t type, int8_t x, int8_t z, uint8_t spawnId)
{
	int8_t slot = -1;

	for(int8_t n = 0; n < MAX_ACTIVE_ITEMS; n++)
	{
		if(items[n].spawnId == 0xff)
		{
			slot = n;
		}
		else if(spawnId != DYNAMIC_ITEM_ID && items[n].spawnId == spawnId)
		{
			return false;
		}
	}

	if(slot == -1)
	{
		for(int8_t n = 0; n < MAX_ACTIVE_ITEMS; n++)
		{
			if(!isValid(items[n].x, items[n].z))
			{
				slot = n;
				break;
			}
		}
	}

	if(slot == -1)
	{
		WARNING("No room to spawn item!\n");
		return false;
	}

	items[slot].type = type;
	items[slot].spawnId = spawnId;
	items[slot].x = x;
	items[slot].z = z;

	return true;
}
