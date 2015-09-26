#include "Engine.h"
#include "Renderer.h"
#include "FixedMath.h"
#include "TileTypes.h"

#include <math.h> // temp
#include "Data_Walls.h"
#include "Data_Guard.h"
#include "Data_Pistol.h"
#include "Data_Decorations.h"
#include "Data_BlockingDecorations.h"

#include <stdio.h>
int overdraw = 0;

void Renderer::init()
{
}

void Renderer::drawWeapon()
{
	BitPairReader reader((uint8_t*)Data_pistolSprite + TEXTURE_STRIDE * TEXTURE_SIZE * Engine::player.weapon.frame);

	for(int i = 0; i < 16; i++)
	{
		for(int j = 0; j < 16; j++)
		{
			uint8_t pixel = reader.read();
			if(pixel)
			{
				Platform.drawPixel(i + HALF_DISPLAYWIDTH - 8, DISPLAYHEIGHT - 16 + j, (pixel - 1) ? 0 : 1);
			}
		}
	}
}

void Renderer::drawFrame()
{
	renderQueueHead = NULL_QUEUE_ITEM;
	for(int n = 0; n < RENDER_QUEUE_CAPACITY; n++)
	{
		renderQueue[n].data = NULL;
	}

	xpos = Engine::player.x;
	zpos = Engine::player.z;
	cos_dir = FixedMath::Cos(-Engine::player.direction);
	sin_dir = FixedMath::Sin(-Engine::player.direction);
	overdraw = 0;
	xcell = Engine::player.x / CELL_SIZE;
	zcell = Engine::player.z / CELL_SIZE;
	initWBuffer();
	drawFloorAndCeiling();

	drawBufferedCells();
	drawDoors();

	/*queueSprite((uint8_t*)Data_guardSprite, CELL_SIZE * (MAP_SIZE / 2 + 2), CELL_SIZE * (MAP_SIZE - 2));
	queueSprite((uint8_t*)Data_guardSprite, CELL_SIZE * (MAP_SIZE / 2 + 2), CELL_SIZE * (MAP_SIZE - 3));
	queueSprite((uint8_t*)Data_guardSprite, CELL_SIZE * (MAP_SIZE / 2 + 2), CELL_SIZE * (MAP_SIZE - 4));
	*/
	for(uint8_t item = renderQueueHead; item != NULL_QUEUE_ITEM; item = renderQueue[item].next)
	{
		drawQueuedSprite(item);
	}
	if(0)
	{
		static int time = 0;
		time++;
		int frame = 0 + ((time / 4) % 9);
		int offset = TEXTURE_STRIDE * TEXTURE_SIZE * frame;

		drawSprite((uint8_t*)Data_guardSprite + offset, CELL_SIZE * MAP_SIZE / 2, CELL_SIZE * (MAP_SIZE - 2));
		drawSprite((uint8_t*)Data_guardSprite + offset, CELL_SIZE * MAP_SIZE / 2, CELL_SIZE * (MAP_SIZE - 32));
	}

	drawWeapon();
}

void Renderer::drawBufferedCells()
{
	int xd, zd;
	int x1, z1, x2, z2;

	if(cos_dir > 0)
	{
		x1 = Engine::map.bufferX;
		x2 = x1 + MAP_BUFFER_SIZE;
		xd = 1;
	}
	else
	{
		x2 = Engine::map.bufferX - 1;
		x1 = x2 + MAP_BUFFER_SIZE;
		xd = -1;
	}
	if(sin_dir < 0)
	{
		z1 = Engine::map.bufferZ;
		z2 = z1 + MAP_BUFFER_SIZE;
		zd = 1;
	}
	else
	{
		z2 = Engine::map.bufferZ - 1;
		z1 = z2 + MAP_BUFFER_SIZE;
		zd = -1;
	}

	if(mabs(cos_dir) < mabs(sin_dir))
	{
		for(int z = z1; z != z2; z += zd)
		{
			for(int x = x1; x != x2; x+= xd)
			{
				drawCell(x, z);
			}
		}
	}
	else
	{
		for(int x = x1; x != x2; x+= xd)
		{
			for(int z = z1; z != z2; z += zd)
			{
				drawCell(x, z);
			}
		}
	}
}

void Renderer::initWBuffer()
{
	for (int i=0; i<DISPLAYWIDTH; i++)
		wbuffer[i] = 0;
	numColumns = 0;
}

#if defined(PLATFORM_GAMEBUINO)
extern uint8_t _displayBuffer[];
void Renderer::drawFloorAndCeiling()
{
	memset(_displayBuffer, 0x00, 3*84);
	for (int y=3, ofs=3*84; y<6; y++)
	{
		for (int x=0; x<84; x+=2)
		{
			_displayBuffer[ofs++] = 0x55;
			_displayBuffer[ofs++] = 0x00;
		}
	}
}
#else
void Renderer::drawFloorAndCeiling()
{
	for(int x = 0; x < DISPLAYWIDTH; x++)
	{
		for(int y = 0; y < DISPLAYHEIGHT; y++)
		{
#if defined(EMULATE_UZEBOX)
			if(y < HALF_DISPLAYHEIGHT || ((x & y) & 1) == 0)
			{
				Platform.drawPixel(x, y, 3);
			}
			else
			{
				Platform.drawPixel(x, y, 2);
			}
#elif 1
			if(y < HALF_DISPLAYHEIGHT || ((x & y) & 1) == 0)
			{
				Platform.drawPixel(x, y, 1);
			}
			else
			{
				Platform.drawPixel(x, y, 0);
			}
#else
			if(y < HALF_DISPLAYHEIGHT || ((x ^ y) & 1) == 1)
			{
				Platform.drawPixel(x, y, 0);
			}
			else
			{
				Platform.drawPixel(x, y, 1);
			}
#endif
		}
	}

}
#endif

void Renderer::drawCellWall(uint8_t textureId, int x1, int z1, int x2, int z2)
{
	drawWall(x1 * CELL_SIZE, z1 * CELL_SIZE, x2 * CELL_SIZE, z2 * CELL_SIZE, textureId);
	//drawWall(x2 * CELL_SIZE, z2 * CELL_SIZE, x1 * CELL_SIZE, z1 * CELL_SIZE);
}

void Renderer::drawCell(int cellX, int cellZ)
{
	// clip cells behind us
	if((cos_dir * (cellX - xcell) - sin_dir * (cellZ - zcell)) <= 0)
		return;

	uint8_t tile = Engine::map.getTileFast(cellX, cellZ);
	if (tile == 0)
		return;

	if(tile >= Tile_FirstDecoration && tile <= Tile_LastDecoration)
	{
		queueSprite((uint8_t*)Data_decorations + (tile - Tile_FirstDecoration) * TEXTURE_SIZE * TEXTURE_STRIDE, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}
	if(tile >= Tile_FirstBlockingDecoration && tile <= Tile_LastBlockingDecoration)
	{
		queueSprite((uint8_t*)Data_blockingDecorations + (tile - Tile_FirstBlockingDecoration) * TEXTURE_SIZE * TEXTURE_STRIDE, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}
	if(tile >= Tile_FirstActor && tile <= Tile_LastActor)
	{
		queueSprite((uint8_t*)Data_guardSprite, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}
	if(tile >= Tile_FirstItem && tile <= Tile_LastItem)
	{
		queueSprite((uint8_t*)Data_decorations + (1) * TEXTURE_SIZE * TEXTURE_STRIDE, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
		//queueSprite((uint8_t*)Data_guardSprite, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
	}

	if(tile >= Tile_FirstWall && tile <= Tile_LastWall)
	{
		uint8_t textureId = tile - Tile_FirstWall; //Engine::map.getTextureId(cellX, cellZ);

		if (zpos < cellZ * CELL_SIZE)
		{
			if (xpos > cellX * CELL_SIZE)
			{
				// north west quadrant
				if (zpos < cellZ * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX, cellZ - 1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
					else if(!Engine::map.isSolid(cellX, cellZ - 1))
					{
						drawCellWall(textureId, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
				}
				if (xpos > (cellX+1) * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX + 1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
					else if(!Engine::map.isSolid(cellX+1, cellZ))
					{
						drawCellWall(textureId, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
				}
			}
			else
			{
				// north east quadrant
				if (zpos < cellZ * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX, cellZ-1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
					else if(!Engine::map.isSolid(cellX, cellZ-1))
					{
						drawCellWall(textureId, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
				}
				if (xpos< cellX * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX-1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
					else if(!Engine::map.isSolid(cellX-1, cellZ))
					{
						drawCellWall(textureId, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
				}
			}
		}
		else
		{
			if (xpos > cellX * CELL_SIZE)
			{
				// south west quadrant
				if (zpos > (cellZ+1) * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX, cellZ+1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
					else if(!Engine::map.isSolid(cellX, cellZ+1))
					{
						drawCellWall(textureId, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
				}
				if (xpos > (cellX+1) * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX+1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
					else if(!Engine::map.isSolid(cellX+1, cellZ))
					{
						drawCellWall(textureId, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
				}
			}
			else
			{
				// south east quadrant
				if (zpos > (cellZ+1) * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX, cellZ+1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
					else if(!Engine::map.isSolid(cellX, cellZ+1))
					{
						drawCellWall(textureId, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
				}
				if (xpos< cellX * CELL_SIZE)
				{
					if(Engine::map.isDoor(cellX-1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
					else if(!Engine::map.isSolid(cellX-1, cellZ))
					{
						drawCellWall(textureId, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
				}
			}
		}
	}
}

#define FORCE_WALL_STRIP_EDGES 1

/*inline*/ void Renderer::drawStrip(int16_t x, int16_t w, int8_t u, uint8_t textureId)
{
	int halfW = w >> 1;
	int y1 = (HALF_DISPLAYHEIGHT) - halfW;
	int y2 = (HALF_DISPLAYHEIGHT) + halfW;
	int verror = halfW;

	BitPairReader textureReader((uint8_t*) Data_wallTextures + u * TEXTURE_STRIDE + textureId * (TEXTURE_STRIDE * TEXTURE_SIZE));
	uint8_t texData = textureReader.read();

#if FORCE_WALL_STRIP_EDGES
	for(int y = y1; y < y2; y++)
#else
	for(int y = y1; y <= y2; y++)
#endif
	{
		if(y >= 0 && y < DISPLAYHEIGHT)
		{
			switch(texData)
			{
			case 1:
				Platform.drawPixel(x, y, 1);
				break;
			case 2:
				Platform.drawPixel(x, y, 0);
				break;
			case 0:
#if defined(EMULATE_UZEBOX)
				Platform.drawPixel(x, y, 2);
#else
				if((x ^ y) & 1)
				{
					Platform.drawPixel(x, y, 1);
				}
				else
				{
					Platform.drawPixel(x, y, 0);
				}
#endif
				break;
			case 3:
#if defined(EMULATE_UZEBOX)
				Platform.drawPixel(x, y, 3);
#else
				if((x & y) & 1)
				{
					Platform.drawPixel(x, y, 0);
				}
				else
				{
					Platform.drawPixel(x, y, 1);
				}
#endif
				break;
			}

		}

		verror -= 15;

		while(verror < 0)
		{
			texData = textureReader.read();
			verror += w;
		}
	}

#if FORCE_WALL_STRIP_EDGES
	if(y2 < DISPLAYHEIGHT)
		Platform.drawPixel(x, y2, 0);
#endif

}

// draws one side of a cell
void Renderer::drawWall(int16_t _x1, int16_t _z1, int16_t _x2, int16_t _z2, uint8_t textureId, int8_t _u1, int8_t _u2)
{
	// find position of wall edges relative to eye

	int16_t z2 = (int16_t)(FIXED_TO_INT(cos_dir * (int32_t)(_x1-xpos)) - FIXED_TO_INT(sin_dir * (int32_t)(_z1-zpos)));
	int16_t x2 = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_x1-xpos)) + FIXED_TO_INT(cos_dir * (int32_t)(_z1-zpos)));
	int16_t z1 = (int16_t)(FIXED_TO_INT(cos_dir * (int32_t)(_x2-xpos)) - FIXED_TO_INT(sin_dir * (int32_t)(_z2-zpos)));
	int16_t x1 = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_x2-xpos)) + FIXED_TO_INT(cos_dir * (int32_t)(_z2-zpos)));

	// clip to the front pane
	if ((z1<CLIP_PLANE) && (z2<CLIP_PLANE))
		return;
	if (z1 < CLIP_PLANE)
	{
		x1 += (CLIP_PLANE-z1) * (x2-x1) / (z2-z1);
		z1 = CLIP_PLANE;
	}
	else if (z2 < CLIP_PLANE)
	{
		x2 += (CLIP_PLANE-z2) * (x1-x2) / (z1-z2);
		z2 = CLIP_PLANE;
	}

	// apply perspective projection
	int16_t vx1 = (int16_t)(x1 * NEAR_PLANE * CAMERA_SCALE / z1);  
	int16_t vx2 = (int16_t)(x2 * NEAR_PLANE * CAMERA_SCALE / z2); 

	// transform the end points into screen space
	int16_t sx1 = (int16_t)((DISPLAYWIDTH / 2) + vx1);
	int16_t sx2 = (int16_t)((DISPLAYWIDTH / 2) + vx2) - 1;

	// clamp to the visible portion of the screen
	int16_t firstx = max(sx1, 0);
	int16_t lastx = min(sx2, DISPLAYWIDTH-1);
	if (lastx < firstx)
		return;

	int16_t w1 = (int16_t)((CELL_SIZE * NEAR_PLANE * CAMERA_SCALE) / z1);
	int16_t w2 = (int16_t)((CELL_SIZE * NEAR_PLANE * CAMERA_SCALE) / z2);
	int16_t dx = sx2 - sx1;
	int16_t werror = dx >> 1;
	int16_t uerror = werror;
	int16_t w = w1;
	int8_t u = _u1;
	int8_t du, ustep;
	int16_t dw, wstep;

	if(w1 < w2)
	{
		dw = w2 - w1;
		wstep = 1;
	}
	else
	{
		dw = w1 - w2;
		wstep = -1;
	}

	if(_u1 < _u2)
	{
		du = _u2 - _u1;
		ustep = 1;
	}
	else
	{
		du = _u1 - _u2;
		ustep = -1;
	}

	for (int x=sx1; x<=sx2; x++)
	{
		if (x >= 0 && x < DISPLAYWIDTH && w > wbuffer[x])
		{        
			if(wbuffer[x] != 0)
			{
				overdraw++;
			}
			if(w <= 255)
			{
				wbuffer[x] = (uint8_t) w;
			}
			else
			{
				wbuffer[x] = 255;
			}

			numColumns++;
			drawStrip(x, w, u, textureId);
		}

		werror -= dw;
		uerror -= du;

		if(dx > 0)
		{
			while(werror < 0)
			{
				w += wstep;
				werror += dx;
			}
			while(uerror < 0)
			{
				u += ustep;
				uerror += dx;
			}
		}
	}
}

void Renderer::drawDoors()
{
	for(int n = 0; n < MAX_DOORS; n++)
	{
		Door& door = Engine::map.doors[n];
		uint8_t textureId = 18;
		int offset = door.open;
		if(offset >= 16)
		{
			continue;
		}

		if(door.x < Engine::map.bufferX || door.z < Engine::map.bufferZ
			|| door.x >= Engine::map.bufferX + MAP_BUFFER_SIZE || door.z >= Engine::map.bufferZ + MAP_BUFFER_SIZE)
		{
			continue;
		}

		if(door.type == DoorType_StandardVertical)
		{
			if(xpos < door.x * CELL_SIZE + CELL_SIZE / 2)
			{
				drawWall(door.x * CELL_SIZE + CELL_SIZE / 2, door.z * CELL_SIZE + CELL_SIZE, 
					door.x * CELL_SIZE + CELL_SIZE / 2, door.z * CELL_SIZE + offset * 2, textureId, 0, 15 - offset);
			}
			else
			{
				drawWall(door.x * CELL_SIZE + CELL_SIZE / 2, door.z * CELL_SIZE + offset * 2, 
					door.x * CELL_SIZE + CELL_SIZE / 2, door.z * CELL_SIZE + CELL_SIZE, textureId, 15 - offset, 0);
			}
		}
		else if(door.type == DoorType_StandardHorizontal)
		{
			if(zpos > door.z * CELL_SIZE + CELL_SIZE / 2)
			{
				drawWall(door.x * CELL_SIZE + CELL_SIZE, door.z * CELL_SIZE + CELL_SIZE / 2, 
					door.x * CELL_SIZE + offset * 2, door.z * CELL_SIZE + CELL_SIZE / 2, textureId, 0, 15 - offset);
			}
			else
			{
				drawWall(door.x * CELL_SIZE + offset * 2, door.z * CELL_SIZE + CELL_SIZE / 2, 
					door.x * CELL_SIZE + CELL_SIZE, door.z * CELL_SIZE + CELL_SIZE / 2, textureId, 15 - offset, 0);
			}
		}
	}
}

void Renderer::drawSprite(uint8_t* sprite, int16_t _x, int16_t _z)
{
	int16_t zt = (int16_t)(FIXED_TO_INT(cos_dir * (int32_t)(_x-xpos)) - FIXED_TO_INT(sin_dir * (int32_t)(_z-zpos)));
	int16_t xt = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_x-xpos)) + FIXED_TO_INT(cos_dir * (int32_t)(_z-zpos)));

	// clip to the front pane
	if (zt < CLIP_PLANE)
		return;

	// apply perspective projection
	int16_t vx = (int16_t)(xt * NEAR_PLANE * CAMERA_SCALE / zt);  
	int16_t w = (int16_t)((CELL_SIZE * NEAR_PLANE * CAMERA_SCALE) / zt);
	int16_t halfW = w >> 1;
	int y1 = (HALF_DISPLAYHEIGHT) - halfW;
	int y2 = (HALF_DISPLAYHEIGHT) + halfW;

	// transform the end points into screen space
	int16_t sx1 = (int16_t)((DISPLAYWIDTH / 2) + vx - halfW);
	int16_t sx2 = sx1 + w;


	int16_t dx = w;
	int16_t uerror = dx >> 1;
	int8_t u = 0;
	int8_t du = 16, ustep = 1;

	for (int x = sx1; x <= sx2; x++)
	{
		if (x >= 0 && x < DISPLAYWIDTH && w > wbuffer[x])
		{        
			int verror = halfW;

			BitPairReader textureReader((uint8_t*) sprite + u * TEXTURE_STRIDE);
			uint8_t texData = textureReader.read();

			for(int y = y1; y <= y2; y++)
			{
				if(y >= 0 && y < DISPLAYHEIGHT)
				{
					switch(texData)
					{
					case 0:
						break;
					case 1:
						Platform.drawPixel(x, y, 1);
						break;
					case 2:
						Platform.drawPixel(x, y, 0);
						break;
					case 3:
#if defined(EMULATE_UZEBOX)
						Platform.drawPixel(x, y, 2);
#else
						if((x ^ y) & 1)
						{
							Platform.drawPixel(x, y, 1);
						}
						else
						{
							Platform.drawPixel(x, y, 0);
						}
#endif
						break;
					}

				}

				verror -= 15;

				while(verror < 0)
				{
					texData = textureReader.read();
					verror += w;
				}
			}
		}

		uerror -= du;

		if(dx > 0)
		{
			while(uerror < 0)
			{
				u += ustep;
				uerror += dx;
			}
		}
	}
}

void Renderer::queueSprite(uint8_t* sprite, int16_t _x, int16_t _z)
{
	int16_t zt = (int16_t)(FIXED_TO_INT(cos_dir * (int32_t)(_x-xpos)) - FIXED_TO_INT(sin_dir * (int32_t)(_z-zpos)));
	int16_t xt = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_x-xpos)) + FIXED_TO_INT(cos_dir * (int32_t)(_z-zpos)));

	// clip to the front plane
	if (zt < CLIP_PLANE)
		return;

	// apply perspective projection
	int16_t vx = (int16_t)(xt * NEAR_PLANE * CAMERA_SCALE / zt);  

	if(vx <= -DISPLAYWIDTH || vx >= DISPLAYWIDTH)
		return;

	int16_t w = (int16_t)((CELL_SIZE * NEAR_PLANE * CAMERA_SCALE) / zt);
	int16_t x = vx + HALF_DISPLAYWIDTH;

	if(w > 255)
		w = 255;

	// TODO: cull if off screen (x)
	uint8_t newItem = NULL_QUEUE_ITEM;
	for(int n = 0; n < RENDER_QUEUE_CAPACITY; n++)
	{
		if(renderQueue[n].data == NULL)
		{
			newItem = n;
			break;
		}
	}

	if(newItem == NULL_QUEUE_ITEM)
	{
		if(w > renderQueue[renderQueueHead].w)
		{
			newItem = renderQueueHead;
			renderQueueHead = renderQueue[renderQueueHead].next;
		}
		else
		{
			WARNING("Out of queue space!\n");
			return;
		}
	}

	renderQueue[newItem].x = x;
	renderQueue[newItem].w = w;
	renderQueue[newItem].data = sprite;

	if(renderQueueHead == NULL_QUEUE_ITEM)
	{
		renderQueueHead = newItem;
		renderQueue[newItem].next = NULL_QUEUE_ITEM;
		return;
	}
	else
	{
		if(w < renderQueue[renderQueueHead].w)
		{
			renderQueue[newItem].next = renderQueueHead;
			renderQueueHead = newItem;
		}
		else
		{
			for(uint8_t item = renderQueueHead; item != NULL_QUEUE_ITEM; item = renderQueue[item].next)
			{
				if(renderQueue[item].next == NULL_QUEUE_ITEM)
				{
					renderQueue[item].next = newItem;
					renderQueue[newItem].next = NULL_QUEUE_ITEM;
					break;
				}
				else if(w < renderQueue[renderQueue[item].next].w)
				{
					renderQueue[newItem].next = renderQueue[item].next;
					renderQueue[item].next = newItem;
					break;
				}
			}
		}
	}
}

void Renderer::drawQueuedSprite(uint8_t id)
{
	int16_t halfW = renderQueue[id].w >> 1;
	int y1 = (HALF_DISPLAYHEIGHT) - halfW;
	int y2 = (HALF_DISPLAYHEIGHT) + halfW;

	// transform the end points into screen space
	int16_t w = renderQueue[id].w;
	int16_t sx1 = renderQueue[id].x - halfW;
	int16_t sx2 = sx1 + w;

	int16_t dx = w;
	int16_t uerror = dx >> 1;
	int8_t u = 0;
	int8_t du = 16, ustep = 1;

	for (int x = sx1; x <= sx2; x++)
	{
		if (x >= 0 && x < DISPLAYWIDTH && w > wbuffer[x])
		{        
			int verror = halfW;

			BitPairReader textureReader((uint8_t*) renderQueue[id].data + u * TEXTURE_STRIDE);
			uint8_t texData = textureReader.read();

			for(int y = y1; y <= y2; y++)
			{
				if(y >= 0 && y < DISPLAYHEIGHT)
				{
					switch(texData)
					{
					case 0:
						break;
					case 1:
						Platform.drawPixel(x, y, 1);
						break;
					case 2:
						Platform.drawPixel(x, y, 0);
						break;
					case 3:
#if defined(EMULATE_UZEBOX)
						Platform.drawPixel(x, y, 2);
#else
						if((x ^ y) & 1)
						{
							Platform.drawPixel(x, y, 1);
						}
						else
						{
							Platform.drawPixel(x, y, 0);
						}
#endif
						break;
					}

				}

				verror -= 15;

				while(verror < 0)
				{
					texData = textureReader.read();
					verror += w;
				}
			}
		}

		uerror -= du;

		if(dx > 0)
		{
			while(uerror < 0)
			{
				u += ustep;
				uerror += dx;
			}
		}
	}
}
