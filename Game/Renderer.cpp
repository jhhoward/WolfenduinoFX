#include "Engine.h"
#include "Renderer.h"
#include "FixedMath.h"
#include "TileTypes.h"

#include "Data_Walls.h"
#include "Data_Pistol.h"
#include "Data_Decorations.h"
#include "Data_BlockingDecorations.h"
#include "Data_Items.h"

#include <stdio.h>

void Renderer::init()
{
}

void Renderer::drawDamage()
{
	if(damageIndicator > 0)
	{
		damageIndicator --;
		for(int x = 0; x < DISPLAYWIDTH; x++)
		{
			setPixel(x, 0);
			setPixel(x, DISPLAYHEIGHT - 1);
		}
		for(int y = 0; y < DISPLAYHEIGHT; y++)
		{
			setPixel(0, y);
			setPixel(DISPLAYWIDTH - 1, y);
		}
	}
}

void Renderer::drawWeapon()
{
	SpriteFrame* frame = (SpriteFrame*) &Data_pistolSprite_frames[engine.player.weapon.frame];
	BitPairReader reader((uint8_t*)Data_pistolSprite, pgm_read_word(&frame->offset));
	uint8_t frameWidth = pgm_read_byte(&frame->width);
	uint8_t frameHeight = pgm_read_byte(&frame->height);
	uint8_t x = HALF_DISPLAYWIDTH - 8 + pgm_read_byte(&frame->xOffset);

	for(int8_t i = 0; i < frameWidth; i++)
	{
		for(int8_t j = frameHeight - 1; j >= 0; j--)
		{
			uint8_t pixel = reader.read();
			if(pixel)
			{
				drawPixel(i + x, DISPLAYHEIGHT - frameHeight + j, (pixel - 1) ? 0 : 1);
			}
		}
	}
}

void Renderer::drawFrame()
{
	renderQueueHead = NULL_QUEUE_ITEM;
	for(int8_t n = 0; n < RENDER_QUEUE_CAPACITY; n++)
	{
		renderQueue[n].data = NULL;
	}

	view.x = engine.player.x;
	view.z = engine.player.z;
	view.rotCos = FixedMath::Cos(-engine.player.direction);
	view.rotSin = FixedMath::Sin(-engine.player.direction);
	view.clipCos = FixedMath::Cos(-engine.player.direction + DEGREES_90 / 2);
	view.clipSin = FixedMath::Sin(-engine.player.direction + DEGREES_90 / 2);

	view.cellX = engine.player.x / CELL_SIZE;
	view.cellZ = engine.player.z / CELL_SIZE;
	initWBuffer();

#if !defined(DEFER_RENDER)
	drawFloorAndCeiling();
#endif

	drawBufferedCells();
	drawDoors();

	for(int8_t n = 0; n < MAX_ACTIVE_ACTORS; n++)
	{
		if(engine.actors[n].type != ActorType_Empty && !engine.actors[n].flags.frozen)
		{
			engine.actors[n].draw();
		}
	}

	for(int8_t n = 0; n < MAX_ACTIVE_ITEMS; n++)
	{
		if(engine.map.items[n].type != 0)
		{
			int16_t x = engine.map.items[n].x * CELL_SIZE + CELL_SIZE / 2, z = engine.map.items[n].z * CELL_SIZE + CELL_SIZE / 2;
			queueSprite((SpriteFrame*) &Data_itemSprites_frames[(engine.map.items[n].type - Tile_FirstItem)], (uint8_t*)Data_itemSprites, x, z);
		}
	}
	
	/*queueSprite((uint8_t*)Data_guardSprite, CELL_SIZE * (MAP_SIZE / 2 + 2), CELL_SIZE * (MAP_SIZE - 2));
	queueSprite((uint8_t*)Data_guardSprite, CELL_SIZE * (MAP_SIZE / 2 + 2), CELL_SIZE * (MAP_SIZE - 3));
	queueSprite((uint8_t*)Data_guardSprite, CELL_SIZE * (MAP_SIZE / 2 + 2), CELL_SIZE * (MAP_SIZE - 4));
	*/
#if 0
	int fill1 = 0;
	int fill2 = 0;
	for(int i = engine.map.bufferX; i < engine.map.bufferX + MAP_BUFFER_SIZE; i++)
	{
		for(int j = engine.map.bufferZ; j < engine.map.bufferZ + MAP_BUFFER_SIZE; j++)
		{
			uint8_t tile = engine.map.getTile(i, j);
			uint8_t colour = 1;

			if(!((view.clipCos * (i - view.cellX) - view.clipSin * (j - view.cellZ)) <= 0))
				fill1 ++;
			if(!isFrustrumClipped(i, j))
				fill2 ++;
			if(tile >= Tile_FirstWall && tile <= Tile_LastWall)
			{
				colour = 0;
				if((view.clipCos * (i - view.cellX) - view.clipSin * (j - view.cellZ)) <= 0)
					colour = 1;
				colour = isFrustrumClipped(i, j) ? 1 : 0;
			}
			drawPixel(i - engine.map.bufferX, j - engine.map.bufferZ, colour);
		}
	}
	WARNING("Old: %d\tNew: %d\t Diff=%f\n", fill1, fill2, (float)fill2 / (float)fill1);
	drawPixel(view.cellX - engine.map.bufferX, view.cellZ - engine.map.bufferZ, 0);

#endif

#if !defined(DEFER_RENDER)
	for(uint8_t item = renderQueueHead; item != NULL_QUEUE_ITEM; item = renderQueue[item].next)
	{
		drawQueuedSprite(item);
	}

	drawWeapon();
	drawDamage();
#endif
}

#ifdef DEFER_RENDER
void Renderer::drawDeferredFrame()
{
	for(int x = 0; x < DISPLAYWIDTH; x++)
	{
		int16_t w = wbuffer[x];
		int16_t halfW = w >> 1;
		for(int y = 0; y < HALF_DISPLAYHEIGHT - halfW - 1; y++)
		{
			clearPixel(x, y);
			clearPixel(x, DISPLAYHEIGHT - y - 1);
		}
		drawStrip(x, wbuffer[x], ubuffer[x], texbuffer[x]);
	}
	/*for(uint8_t item = renderQueueHead; item != NULL_QUEUE_ITEM; item = renderQueue[item].next)
	{
		drawQueuedSprite(item);
	}

	drawWeapon();*/
}
#endif

void Renderer::drawBufferedCells()
{
	int8_t xd, zd;
	int8_t x1, z1, x2, z2;

	if(view.rotCos > 0)
	{
		x1 = engine.map.bufferX;
		x2 = x1 + MAP_BUFFER_SIZE;
		xd = 1;
	}
	else
	{
		x2 = engine.map.bufferX - 1;
		x1 = x2 + MAP_BUFFER_SIZE;
		xd = -1;
	}
	if(view.rotSin < 0)
	{
		z1 = engine.map.bufferZ;
		z2 = z1 + MAP_BUFFER_SIZE;
		zd = 1;
	}
	else
	{
		z2 = engine.map.bufferZ - 1;
		z1 = z2 + MAP_BUFFER_SIZE;
		zd = -1;
	}

	if(mabs(view.rotCos) < mabs(view.rotSin))
	{
		for(int8_t z = z1; z != z2; z += zd)
		{
			for(int8_t x = x1; x != x2; x+= xd)
			{
				drawCell(x, z);
			}
		}
	}
	else
	{
		for(int8_t x = x1; x != x2; x+= xd)
		{
			for(int8_t z = z1; z != z2; z += zd)
			{
				drawCell(x, z);
			}
		}
	}
}

void Renderer::initWBuffer()
{
	for (int8_t i=0; i<DISPLAYWIDTH; i++)
		wbuffer[i] = 0;
}

#if defined(PLATFORM_GAMEBUINO)
extern uint8_t _displayBuffer[];
void Renderer::drawFloorAndCeiling()
{
	memset(_displayBuffer, 0x00, 3*84);
	for (int y=3, ofs=3*84; y<6; y++)
	{
		for (int8_t x=0; x<84; x+=2)
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
				drawPixel(x, y, 3);
			}
			else
			{
				drawPixel(x, y, 2);
			}
#elif 1
			if(y < HALF_DISPLAYHEIGHT || ((x & y) & 1) == 0)
			{
				clearPixel(x, y);
			}
			else
			{
				setPixel(x, y);
			}
#else
			if(y < HALF_DISPLAYHEIGHT || ((x ^ y) & 1) == 1)
			{
				setPixel(x, y);
			}
			else
			{
				clearPixel(x, y);
			}
#endif
		}
	}

}
#endif

void Renderer::drawCellWall(uint8_t textureId, int8_t x1, int8_t z1, int8_t x2, int8_t z2)
{
	drawWall(x1 * CELL_SIZE, z1 * CELL_SIZE, x2 * CELL_SIZE, z2 * CELL_SIZE, textureId);
	//drawWall(x2 * CELL_SIZE, z2 * CELL_SIZE, x1 * CELL_SIZE, z1 * CELL_SIZE);
}

void Renderer::drawCell(int8_t cellX, int8_t cellZ)
{
	// clip cells out of frustum view
	if(isFrustrumClipped(cellX, cellZ))
		return;

	uint8_t tile = engine.map.getTileFast(cellX, cellZ);
	if (tile == 0)
		return;
	
	if(tile >= Tile_FirstDecoration && tile <= Tile_LastDecoration)
	{
		queueSprite((SpriteFrame*) &Data_decorations_frames[tile - Tile_FirstDecoration], (uint8_t*)Data_decorations, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}
	if(tile >= Tile_FirstBlockingDecoration && tile <= Tile_LastBlockingDecoration)
	{
		queueSprite((SpriteFrame*) &Data_blockingDecorations_frames[tile - Tile_FirstBlockingDecoration], (uint8_t*)Data_blockingDecorations, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}
	/*if(tile >= Tile_FirstActor && tile <= Tile_LastActor)
	{
		queueSprite((uint8_t*)Data_guardSprite, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}*/
	/*if(tile >= Tile_FirstItem && tile <= Tile_LastItem)
	{
		queueSprite((SpriteFrame*) &Data_itemSprites_frames[(tile - Tile_FirstItem)], (uint8_t*)Data_itemSprites, cellX * CELL_SIZE + CELL_SIZE / 2, cellZ * CELL_SIZE + CELL_SIZE / 2);
		return;
	}*/

	if(tile >= Tile_FirstWall && tile <= Tile_LastWall)
	{
		uint8_t textureId = tile - Tile_FirstWall; //engine.map.getTextureId(cellX, cellZ);

		if (view.z < cellZ * CELL_SIZE)
		{
			if (view.x > cellX * CELL_SIZE)
			{
				// north west quadrant
				if (view.z < cellZ * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX, cellZ - 1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
					else if(!engine.map.isSolid(cellX, cellZ - 1))
					{
						drawCellWall(textureId, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
				}
				if (view.x > (cellX+1) * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX + 1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
					else if(!engine.map.isSolid(cellX+1, cellZ))
					{
						drawCellWall(textureId, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
				}
			}
			else
			{
				// north east quadrant
				if (view.z < cellZ * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX, cellZ-1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
					else if(!engine.map.isSolid(cellX, cellZ-1))
					{
						drawCellWall(textureId, cellX, cellZ, cellX+1, cellZ);  // south wall
					}
				}
				if (view.x< cellX * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX-1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
					else if(!engine.map.isSolid(cellX-1, cellZ))
					{
						drawCellWall(textureId, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
				}
			}
		}
		else
		{
			if (view.x > cellX * CELL_SIZE)
			{
				// south west quadrant
				if (view.z > (cellZ+1) * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX, cellZ+1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
					else if(!engine.map.isSolid(cellX, cellZ+1))
					{
						drawCellWall(textureId, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
				}
				if (view.x > (cellX+1) * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX+1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
					else if(!engine.map.isSolid(cellX+1, cellZ))
					{
						drawCellWall(textureId, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
					}
				}
			}
			else
			{
				// south east quadrant
				if (view.z > (cellZ+1) * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX, cellZ+1))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
					else if(!engine.map.isSolid(cellX, cellZ+1))
					{
						drawCellWall(textureId, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
					}
				}
				if (view.x< cellX * CELL_SIZE)
				{
					if(engine.map.isDoor(cellX-1, cellZ))
					{
						drawCellWall(DOOR_FRAME_TEXTURE, cellX, cellZ+1, cellX, cellZ);  // west wall
					}
					else if(!engine.map.isSolid(cellX-1, cellZ))
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
				clearPixel(x, y);
				break;
			case 2:
				setPixel(x, y);
				break;
			case 0:
#if defined(EMULATE_UZEBOX)
				drawPixel(x, y, 2);
#else
				if((x ^ y) & 1)
				{
					clearPixel(x, y);
				}
				else
				{
					setPixel(x, y);
				}
#endif
				break;
			case 3:
#if defined(EMULATE_UZEBOX)
				drawPixel(x, y, 3);
#else
				if((x & y) & 1)
				{
					setPixel(x, y);
				}
				else
				{
					clearPixel(x, y);
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
		setPixel(x, y2);
#endif

}

// draws one side of a cell
void Renderer::drawWall(int16_t _x1, int16_t _z1, int16_t _x2, int16_t _z2, uint8_t textureId, int8_t _u1, int8_t _u2)
{
	// find position of wall edges relative to eye

	int16_t z2 = (int16_t)(FIXED_TO_INT(view.rotCos * (int32_t)(_x1-view.x)) - FIXED_TO_INT(view.rotSin * (int32_t)(_z1-view.z)));
	int16_t x2 = (int16_t)(FIXED_TO_INT(view.rotSin * (int32_t)(_x1-view.x)) + FIXED_TO_INT(view.rotCos * (int32_t)(_z1-view.z)));
	int16_t z1 = (int16_t)(FIXED_TO_INT(view.rotCos * (int32_t)(_x2-view.x)) - FIXED_TO_INT(view.rotSin * (int32_t)(_z2-view.z)));
	int16_t x1 = (int16_t)(FIXED_TO_INT(view.rotSin * (int32_t)(_x2-view.x)) + FIXED_TO_INT(view.rotCos * (int32_t)(_z2-view.z)));

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

//	for (int x=firstx; x<=lastx; x++)
	for (int x=sx1; x<=sx2; x++)
	{
		if (x >= 0 && x < DISPLAYWIDTH && w > wbuffer[x])
		{        
			if(w <= 255)
			{
				wbuffer[x] = (uint8_t) w;
			}
			else
			{
				wbuffer[x] = 255;
			}

#ifdef DEFER_RENDER
			ubuffer[x] = u;
			texbuffer[x] = textureId;
#else
			drawStrip(x, w, u, textureId);
#endif
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
		Door& door = engine.map.doors[n];
		uint8_t textureId = 18;
		int offset = door.open;
		if(offset >= 16)
		{
			continue;
		}

		if(door.x < engine.map.bufferX || door.z < engine.map.bufferZ
			|| door.x >= engine.map.bufferX + MAP_BUFFER_SIZE || door.z >= engine.map.bufferZ + MAP_BUFFER_SIZE)
		{
			continue;
		}

		if(door.type == DoorType_StandardVertical)
		{
			if(view.x < door.x * CELL_SIZE + CELL_SIZE / 2)
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
			if(view.z > door.z * CELL_SIZE + CELL_SIZE / 2)
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

void Renderer::queueSprite(SpriteFrame* frame, uint8_t* spriteData, int16_t _x, int16_t _z)
{
#if 1
	int cellX = _x / CELL_SIZE;
	int cellZ = _z / CELL_SIZE;

	if(isFrustrumClipped(cellX, cellZ))
		return;

	int16_t zt = (int16_t)(FIXED_TO_INT(view.rotCos * (int32_t)(_x-view.x)) - FIXED_TO_INT(view.rotSin * (int32_t)(_z-view.z)));
	int16_t xt = (int16_t)(FIXED_TO_INT(view.rotSin * (int32_t)(_x-view.x)) + FIXED_TO_INT(view.rotCos * (int32_t)(_z-view.z)));

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
			//WARNING("Out of queue space!\n");
			return;
		}
	}

	renderQueue[newItem].x = x;
	renderQueue[newItem].w = w;
	renderQueue[newItem].frame = frame;
	renderQueue[newItem].data = spriteData;

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
#endif
}

void Renderer::drawQueuedSprite(uint8_t id)
{
	uint8_t frameWidth = pgm_read_byte(&renderQueue[id].frame->width);
	uint8_t frameHeight = pgm_read_byte(&renderQueue[id].frame->height);
	int16_t halfW = renderQueue[id].w >> 1;
	int y2 = (HALF_DISPLAYHEIGHT) + halfW;
	int y1 = y2 - (renderQueue[id].w * frameHeight) / (CELL_SIZE / 2);

	int16_t w = renderQueue[id].w;

	int16_t dx = (w * frameWidth) / (CELL_SIZE / 2);

	int16_t sx1 = renderQueue[id].x - halfW + (w * pgm_read_byte(&renderQueue[id].frame->xOffset)) / (CELL_SIZE / 2);
	int16_t sx2 = sx1 + dx;
	int16_t uerror = dx;
	int8_t u = 0;
	int8_t du = frameWidth, ustep = 1;
	int8_t v;

	for (int x = sx1; x <= sx2; x++)
	{
		if (x >= 0 && x < DISPLAYWIDTH && w > wbuffer[x])
		{        
			int verror = halfW;

			BitPairReader textureReader((uint8_t*) renderQueue[id].data, pgm_read_word(&renderQueue[id].frame->offset) + frameHeight * u);
			uint8_t texData = textureReader.read();

			v = 0;

			for(int y = y2; y >= y1 && y >= 0 && v < frameHeight; y--)
			{
				if(y < DISPLAYHEIGHT)
				{
					switch(texData)
					{
					case 0:
						break;
					case 1:
						clearPixel(x, y);
						break;
					case 2:
						setPixel(x, y);
						break;
					case 3:
#if defined(EMULATE_UZEBOX)
						drawPixel(x, y, 2);
#else
						if((x ^ y) & 1)
						{
							clearPixel(x, y);
						}
						else
						{
							setPixel(x, y);
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
					v++;
				}

			}
		}

		uerror -= du;

		if(dx > 0)
		{
			while(u < frameWidth - 1 && uerror < 0)
			{
				u += ustep;
				uerror += dx;
			}
		}
	}
}
