#include "Engine.h"
#include "Renderer.h"
#include "FixedMath.h"
#include "TileTypes.h"

#include "Generated/fxdata.h"
//#include "Generated/Data_Walls.h"
#include "Generated/Data_Pistol.h"
#include "Generated/Data_Knife.h"
#include "Generated/Data_Machinegun.h"
#include "Generated/Data_Decorations.h"
#include "Generated/Data_BlockingDecorations.h"
#include "Generated/Data_Items.h"
#include "Generated/Data_Font.h"


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
	SpriteFrame* frame;
	uint24_t address;
	
	switch(engine.player.weapon.type)
	{
	case WeaponType_Knife:
		frame = (SpriteFrame*) &Data_knifeSprite_frames[engine.player.weapon.frame];
		address = Data_knifeSprite;
		break;
	case WeaponType_Pistol:
		frame = (SpriteFrame*) &Data_pistolSprite_frames[engine.player.weapon.frame];
		address = Data_pistolSprite;
		break;
	case WeaponType_MachineGun:
		frame = (SpriteFrame*) &Data_machinegunSprite_frames[engine.player.weapon.frame];
		address = Data_machinegunSprite;
		break;
	default:
		return;
	}
	
	diskSeek(address + frame->offset);
	//BitPairReader reader((uint8_t*)data, pgm_read_word(&frame->offset));
	uint8_t frameWidth = pgm_read_byte(&frame->width);
	uint8_t frameHeight = pgm_read_byte(&frame->height);
	uint8_t x = HALF_DISPLAYWIDTH - 32 + pgm_read_byte(&frame->xOffset);

	for(int8_t i = 0; i < frameWidth; i++)
	{
		for(int8_t j = frameHeight - 1; j >= 0; j--)
		{
			uint8_t pixel = diskReadByte(); // reader.read();
			if(pixel)
			{
				drawPixel(i + x, DISPLAYHEIGHT - frameHeight + j, (pixel - 1) ? 0 : 1);
			}
		}
	}

	diskFinishRead();
}

void Renderer::drawFrame()
{
	renderQueueHead = NULL_QUEUE_ITEM;
	for(int8_t n = 0; n < RENDER_QUEUE_CAPACITY; n++)
	{
		renderQueue[n].spriteAddress = 0;
	}

	view.x = engine.player.x;
	view.z = engine.player.z;
	view.rotCos = FixedMath::Cos(-engine.player.direction);
	view.rotSin = FixedMath::Sin(-engine.player.direction);
	view.clipCos = FixedMath::Cos(-engine.player.direction + DEGREES_90 / 2);
	view.clipSin = FixedMath::Sin(-engine.player.direction + DEGREES_90 / 2);

	view.cellX = WORLD_TO_CELL(engine.player.x);
	view.cellZ = WORLD_TO_CELL(engine.player.z);
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
			int16_t x = CELL_TO_WORLD(engine.map.items[n].x) + CELL_SIZE / 2, z = CELL_TO_WORLD(engine.map.items[n].z) + CELL_SIZE / 2;
			queueSprite((SpriteFrame*) &Data_itemSprites_frames[(engine.map.items[n].type - Tile_FirstItem)], Data_itemSprites, x, z);
		}
	}
	
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

	// Draw HUD
	uint8_t hudHeight = DISPLAYHEIGHT - FONT_HEIGHT;
	drawGlyph('+' - FIRST_FONT_GLYPH, 0, hudHeight);
	drawInt(engine.player.hp, (FONT_WIDTH + 1) * 3, hudHeight);
	drawGlyph('*' - FIRST_FONT_GLYPH, DISPLAYWIDTH - (FONT_WIDTH + 1) * 4, hudHeight);
	drawInt(engine.player.weapon.ammo, DISPLAYWIDTH - (FONT_WIDTH + 1), hudHeight);
	//drawString(PSTR("*99"), DISPLAYWIDTH - (FONT_WIDTH + 1) * 3, DISPLAYHEIGHT - FONT_HEIGHT);
	/*int y = 4;
	drawString(PSTR("* CAN I PLAY, DADDY?"), 0, y); y += FONT_HEIGHT + 1;
	drawString(PSTR("  DON'T HURT ME!"), 0, y); y += FONT_HEIGHT + 1;
	drawString(PSTR("  BRING 'EM ON!"), 0, y); y += FONT_HEIGHT + 1;
	drawString(PSTR("  I AM DEATH"), 0, y); y += FONT_HEIGHT + 1;
	drawString(PSTR("       INCARNATE!"), 0, y); y += FONT_HEIGHT + 1;*/
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
	for (uint8_t i = 0; i < DISPLAYWIDTH; i++)
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
#if 1
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
			if (y < HALF_DISPLAYHEIGHT)
			{
				clearPixel(x, y);
			}
			else
			{
				if ((x & 1))
				{
					if (((x >> 1) & 1) == (y & 1))
					{
						clearPixel(x, y);
					}
					else
					{
						setPixel(x, y);
					}
				}
				else
				{
					clearPixel(x, y);
				}
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
#elif 0
	uint8_t* ptr = GetScreenBuffer();
	uint8_t counter = 128;

	while (counter--)
	{
		*ptr++ = 0x55; *ptr++ = 0x00; *ptr++ = 0xaa; *ptr++ = 0x00;
	}

	counter = 128;
	while (counter--)
	{
		*ptr++ = 0x55; *ptr++ = 0xff; *ptr++ = 0xaa; *ptr++ = 0xff;
	}
#endif
}
#endif

#if defined(PLATFORM_GAMEBUINO)
extern Gamebuino gb;
#endif

void Renderer::drawCell(int8_t cellX, int8_t cellZ)
{
#if defined(PLATFORM_GAMEBUINO)
	// HACK: Keep calling update so that sound doesn't slow down. Ugh..
	gb.update();
#endif
	// clip cells out of frustum view
	if(isFrustrumClipped(cellX, cellZ))
		return;

	uint8_t tile = engine.map.getTileFast(cellX, cellZ);
	if (tile == 0)
		return;

	int16_t worldX = CELL_TO_WORLD(cellX);
	int16_t worldZ = CELL_TO_WORLD(cellZ);
	
	if(tile >= Tile_FirstDecoration && tile <= Tile_LastDecoration)
	{
		queueSprite((SpriteFrame*) &Data_decorations_frames[tile - Tile_FirstDecoration], Data_decorations, worldX + CELL_SIZE / 2, worldZ + CELL_SIZE / 2);
		return;
	}
	if(tile >= Tile_FirstBlockingDecoration && tile <= Tile_LastBlockingDecoration)
	{
		queueSprite((SpriteFrame*) &Data_blockingDecorations_frames[tile - Tile_FirstBlockingDecoration], Data_blockingDecorations, worldX + CELL_SIZE / 2, worldZ + CELL_SIZE / 2);
		return;
	}

	if(tile >= Tile_FirstWall && tile <= Tile_LastWall)
	{
		uint8_t textureId = tile - Tile_FirstWall; //engine.map.getTextureId(cellX, cellZ);

		if (view.z < worldZ)
		{
			if (view.x > worldX)
			{
				// north west quadrant
				if (view.z < worldZ)
				{
					if(engine.map.isDoor(cellX, cellZ - 1))
					{
						drawWall(worldX, worldZ, worldX + CELL_SIZE, worldZ, DOOR_FRAME_TEXTURE);  // south wall
					}
					else if(!engine.map.isSolid(cellX, cellZ - 1))
					{
						drawWall(worldX, worldZ, worldX + CELL_SIZE, worldZ, textureId);  // south wall
					}
				}
				if (view.x > worldX + CELL_SIZE)
				{
					if(engine.map.isDoor(cellX + 1, cellZ))
					{
						drawWall(worldX + CELL_SIZE, worldZ, worldX + CELL_SIZE, worldZ + CELL_SIZE, DOOR_FRAME_TEXTURE);  // east wall
					}
					else if(!engine.map.isSolid(cellX+1, cellZ))
					{
						drawWall(worldX + CELL_SIZE, worldZ, worldX + CELL_SIZE, worldZ + CELL_SIZE, textureId);  // east wall
					}
				}
			}
			else
			{
				// north east quadrant
				if (view.z < worldZ)
				{
					if(engine.map.isDoor(cellX, cellZ-1))
					{
						drawWall(worldX, worldZ, worldX + CELL_SIZE, worldZ, DOOR_FRAME_TEXTURE);  // south wall
					}
					else if(!engine.map.isSolid(cellX, cellZ-1))
					{
						drawWall(worldX, worldZ, worldX + CELL_SIZE, worldZ, textureId);  // south wall
					}
				}
				if (view.x < worldX)
				{
					if(engine.map.isDoor(cellX-1, cellZ))
					{
						drawWall(worldX, worldZ + CELL_SIZE, worldX, worldZ, DOOR_FRAME_TEXTURE);  // west wall
					}
					else if(!engine.map.isSolid(cellX-1, cellZ))
					{
						drawWall(worldX, worldZ + CELL_SIZE, worldX, worldZ, textureId);  // west wall
					}
				}
			}
		}
		else
		{
			if (view.x > worldX)
			{
				// south west quadrant
				if (view.z > worldZ + CELL_SIZE)
				{
					if(engine.map.isDoor(cellX, cellZ+1))
					{
						drawWall(worldX + CELL_SIZE, worldZ + CELL_SIZE, worldX, worldZ + CELL_SIZE, DOOR_FRAME_TEXTURE);  // north wall
					}
					else if(!engine.map.isSolid(cellX, cellZ+1))
					{
						drawWall(worldX + CELL_SIZE, worldZ + CELL_SIZE, worldX, worldZ + CELL_SIZE, textureId);  // north wall
					}
				}
				if (view.x > worldX + CELL_SIZE)
				{
					if(engine.map.isDoor(cellX+1, cellZ))
					{
						drawWall(worldX + CELL_SIZE, worldZ, worldX + CELL_SIZE, worldZ + CELL_SIZE, DOOR_FRAME_TEXTURE);  // east wall
					}
					else if(!engine.map.isSolid(cellX+1, cellZ))
					{
						drawWall(worldX + CELL_SIZE, worldZ, worldX + CELL_SIZE, worldZ + CELL_SIZE, textureId);  // east wall
					}
				}
			}
			else
			{
				// south east quadrant
				if (view.z > worldZ + CELL_SIZE)
				{
					if(engine.map.isDoor(cellX, cellZ+1))
					{
						drawWall(worldX + CELL_SIZE, worldZ + CELL_SIZE, worldX, worldZ + CELL_SIZE, DOOR_FRAME_TEXTURE);  // north wall
					}
					else if(!engine.map.isSolid(cellX, cellZ+1))
					{
						drawWall(worldX + CELL_SIZE, worldZ + CELL_SIZE, worldX, worldZ + CELL_SIZE, textureId);  // north wall
					}
				}
				if (view.x < worldX)
				{
					if(engine.map.isDoor(cellX-1, cellZ))
					{
						drawWall(worldX, worldZ + CELL_SIZE, worldX, worldZ, DOOR_FRAME_TEXTURE);  // west wall
					}
					else if(!engine.map.isSolid(cellX-1, cellZ))
					{
						drawWall(worldX, worldZ + CELL_SIZE, worldX, worldZ, textureId);  // west wall
					}
				}
			}
		}
	}
}

#define FORCE_WALL_STRIP_EDGES 1

bool renderingVerticalWall = false;

/*inline*/ void Renderer::drawStrip(int16_t x, int16_t w, int8_t u, uint8_t textureId)
{
	int halfW = w >> 1;
	int y1 = (HALF_DISPLAYHEIGHT) - halfW;
	int y2 = (HALF_DISPLAYHEIGHT) + halfW;
	int verror = halfW;

	diskSeek(Data_wallTextures + u * TEXTURE_STRIDE + textureId * (TEXTURE_STRIDE * TEXTURE_SIZE));
	//BitPairReader textureReader((uint8_t*) Data_wallTextures + u * TEXTURE_STRIDE + textureId * (TEXTURE_STRIDE * TEXTURE_SIZE));
	uint8_t texData = diskReadByte(); // textureReader.read();

	texData = 2;

#if FORCE_WALL_STRIP_EDGES
	for(int y = y1; y < y2; y++)
#else
	for(int y = y1; y <= y2; y++)
#endif
	{
		if(y >= 0 && y < DISPLAYHEIGHT)
		{
			if (renderingVerticalWall)
			{
				switch (texData)
				{
				case 1:
					if ((x | y) & 1)
					{
						clearPixel(x, y);
					}
					else
					{
						setPixel(x, y);
					}
					break;
				case 2:
					setPixel(x, y);
					break;
				case 0:
					if ((x ^ y) & 1)
					{
						clearPixel(x, y);
					}
					else
					{
						setPixel(x, y);
					}
					break;
				case 3:
					if ((x & y) & 1)
					{
						clearPixel(x, y);
					}
					else
					{
						setPixel(x, y);
					}
					break;
				}
			}
			else
			{
				switch (texData)
				{
				case 1:
					clearPixel(x, y);
					break;
				case 2:
					setPixel(x, y);
					break;
				case 3:
					if ((x ^ y) & 1)
					{
						clearPixel(x, y);
					}
					else
					{
						setPixel(x, y);
					}
					break;
				case 0:
					if ((x & y) & 1)
					{
						setPixel(x, y);
					}
					else
					{
						clearPixel(x, y);
					}
					break;
				}
			}
		}

		//verror -= 15;
		verror -= TEXTURE_SIZE - 1;

		while(verror < 0)
		{
			//texData = textureReader.read();
			texData = diskReadByte();
			verror += w;
		}
	}

#if FORCE_WALL_STRIP_EDGES
	if(y2 < DISPLAYHEIGHT)
		setPixel(x, y2);
#endif

	diskFinishRead();

}

#ifdef PERSPECTIVE_CORRECT_TEXTURE_MAPPING
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
	int16_t du, ustep;
	int16_t dw, wstep;

	int16_t u1 = _u1 * w1;
	int16_t u2 = _u2 * w2;
	int16_t u = _u1;
	int16_t z = z1;
	int8_t dz, zstep;
	int16_t zerror = werror;
	if(z1 < z2)
	{
		dz = z2 - z1;
		zstep = 1;
	}
	else
	{
		dz = z1 - z2;
		zstep = -1;
	}

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

	if(u1 < u2)
	{
		du = u2 - u1;
		ustep = 1;
	}
	else
	{
		du = u1 - u2;
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
			int16_t testOutU = (u * z) / (CELL_SIZE * NEAR_PLANE * CAMERA_SCALE);
			float interpX = (float)(x - sx1) / (float)(sx2 - sx1);
			float u1OverZ = (float)_u1 / (float) z1;
			float u2OverZ = (float)_u2 / (float) z2;
			float interpUOverZ = (interpX * u2OverZ) + (1.0f - interpX) * u1OverZ;
			float interpZ = (interpX * z2) + (1.0f - interpX) * z1;
			float interpU = interpUOverZ * interpZ;
			uint8_t outU = (uint8_t) interpU;
			outU = (uint8_t)testOutU;
			outU = clamp(outU, 0, 15);
			drawStrip(x, w, outU, textureId);
#endif
		}

		werror -= dw;
		uerror -= du;
		zerror -= dz;

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
			while(zerror < 0)
			{
				z += zstep;
				zerror += dx;
			}
		}
	}
}
#else
// draws one side of a cell
void Renderer::drawWall(int16_t _x1, int16_t _z1, int16_t _x2, int16_t _z2, uint8_t textureId, int8_t _u1, int8_t _u2)
{
	renderingVerticalWall = _x1 == _x2;
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
#endif

void Renderer::drawDoors()
{
	for(int n = 0; n < MAX_DOORS; n++)
	{
		Door& door = engine.map.doors[n];
		uint8_t textureId = door.texture;

		if(!engine.map.isValid(door.x, door.z))
		{
			continue;
		}
		
		if(door.type == DoorType_SecretPushWall)
		{
			int16_t doorX = CELL_TO_WORLD(door.x);
			int16_t doorZ = CELL_TO_WORLD(door.z);

			switch(door.state)
			{
			case DoorState_PushNorth:
				doorZ -= door.open;
				break;
			case DoorState_PushEast:
				doorX += door.open;
				break;
			case DoorState_PushSouth:
				doorZ += door.open;
				break;
			case DoorState_PushWest:
				doorX -= door.open;
				break;
			}

			if(view.x < doorX)
			{
				drawWall(doorX, doorZ + CELL_SIZE, doorX, doorZ, door.texture, 0, (TEXTURE_SIZE - 1));
			}
			else if(view.x > doorX)
			{
				drawWall(doorX + CELL_SIZE, doorZ, doorX + CELL_SIZE, doorZ + CELL_SIZE, door.texture, 0, (TEXTURE_SIZE - 1));
			}
			if(view.z > doorZ + CELL_SIZE)
			{
				drawWall(doorX + CELL_SIZE, doorZ + CELL_SIZE, doorX, doorZ + CELL_SIZE, door.texture, 0, (TEXTURE_SIZE - 1));
			}
			else if(view.z < doorZ)
			{
				drawWall(doorX, doorZ, doorX + CELL_SIZE, doorZ, door.texture, 0, (TEXTURE_SIZE - 1));
			}
		}
		else
		{
			int offset = door.open * 2;
			if(offset >= TEXTURE_SIZE)
			{
				continue;
			}

			int16_t worldX = CELL_TO_WORLD(door.x);
			int16_t worldZ = CELL_TO_WORLD(door.z);

			if((door.type & 0x1) == 0)
			{
				worldX += CELL_SIZE / 2;
				if(view.x < worldX)
				{
					drawWall(worldX, worldZ + CELL_SIZE, 
						worldX, worldZ + offset, textureId, 0, (TEXTURE_SIZE - 1) - offset);
				}
				else
				{
					drawWall(worldX, worldZ + offset, 
						worldX, worldZ + CELL_SIZE, textureId, (TEXTURE_SIZE - 1) - offset, 0);
				}
			}
			else
			{
				worldZ += CELL_SIZE / 2;
				if(view.z > worldZ)
				{
					drawWall(worldX + CELL_SIZE, worldZ, 
						worldX + offset, worldZ, textureId, 0, (TEXTURE_SIZE - 1) - offset);
				}
				else
				{
					drawWall(worldX + offset, worldZ, 
						worldX + CELL_SIZE, worldZ, textureId, (TEXTURE_SIZE - 1) - offset, 0);
				}
			}
		}
	}
}

void Renderer::queueSprite(SpriteFrame* frame, uint24_t spriteAddress, int16_t _x, int16_t _z)
{
#if 1
	int cellX = WORLD_TO_CELL(_x);
	int cellZ = WORLD_TO_CELL(_z);

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
		if(renderQueue[n].spriteAddress == 0)
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
	renderQueue[newItem].spriteAddress = spriteAddress;

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
	int y1 = y2 - (renderQueue[id].w * frameHeight) / (CELL_SIZE);

	int16_t w = renderQueue[id].w;

	int16_t dx = (w * frameWidth) / (CELL_SIZE);

	int16_t sx1 = renderQueue[id].x - halfW + (w * pgm_read_byte(&renderQueue[id].frame->xOffset)) / (CELL_SIZE);
	int16_t sx2 = sx1 + dx;
	int16_t uerror = dx;
	int8_t u = 0;
	int8_t lastU = 0;
	int8_t du = frameWidth, ustep = 1;
	int8_t v;
	bool outline = false;

	for (int x = sx1; x <= sx2; x++)
	{
		if (x >= 0 && x < DISPLAYWIDTH && w > wbuffer[x])
		{        
			int verror = halfW;

			uint24_t texAddress = renderQueue[id].spriteAddress + pgm_read_word(&renderQueue[id].frame->offset) + frameHeight * u;
			uint24_t texAddressLeft = renderQueue[id].spriteAddress + pgm_read_word(&renderQueue[id].frame->offset) + frameHeight * lastU;
			//BitPairReader textureReader((uint8_t*) renderQueue[id].data, pgm_read_word(&renderQueue[id].frame->offset) + frameHeight * u);
			//BitPairReader textureReaderLeft((uint8_t*)renderQueue[id].data, pgm_read_word(&renderQueue[id].frame->offset) + frameHeight * lastU);

			diskSeek(texAddress);
			uint8_t texData = diskReadByte();
			diskFinishRead();

			diskSeek(texAddressLeft);
			uint8_t texDataLeft = diskReadByte();
			diskFinishRead();

			v = 0;
			outline = texData != 0;

			for(int y = y2; y >= y1 && y >= 0 && v < frameHeight; y--)
			{
				if(y < DISPLAYHEIGHT)
				{
					if ((!texDataLeft && texData) || (texDataLeft && !texData))
					{
						outline = true;
					}
					if (texData && v == frameHeight - 1)
					{
						outline = true;
					}

					if (outline)
					{
						setPixel(x, y);
						outline = false;
					}
					else
					{
						switch (texData)
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
							if ((x ^ y) & 1)
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
				}
				verror -= 31;

				uint8_t lastTexDataValue = texData;

				if (verror < 0)
				{
					while (verror < 0)
					{
						texAddress++;
						texAddressLeft++;
						//texData = textureReader.read();
						//texDataLeft = textureReaderLeft.read();
						verror += w;
						v++;
					}

					diskSeek(texAddress);
					texData = diskReadByte();
					diskFinishRead();

					diskSeek(texAddressLeft);
					texDataLeft = diskReadByte();
					diskFinishRead();
				}

				if (lastTexDataValue && !texData)
					outline = true;
				if (!lastTexDataValue && texData)
					outline = true;

			}
		}

		uerror -= du;
		lastU = u;

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

void Renderer::drawGlyph(char glyph, uint8_t x, uint8_t y)
{
	uint8_t* ptr = (uint8_t*) (Data_font + glyph * FONT_GLYPH_BYTE_SIZE);
	uint8_t readMask = 1;
	uint8_t read = pgm_read_byte(ptr++);

	for(int i = 0; i < FONT_WIDTH; i++)
	{
		for(int j = 0; j < FONT_HEIGHT; j++)
		{
			uint8_t colour = (read & readMask) ? 0 : 1;
			drawPixel(x + i, y + j, colour);
			readMask <<= 1;
			if(readMask == 0)
			{
				readMask = 1;
				read = pgm_read_byte(ptr++);
			}
		}
//		clearPixel(x + i, y);
	//	clearPixel(x + i, y + FONT_HEIGHT + 1);
	}
	for(int j = 0; j < FONT_HEIGHT; j++)
	{
		clearPixel(x + FONT_WIDTH, y + j);
	}
}

void Renderer::drawString(const char* str, uint8_t x, uint8_t y)
{
	char* ptr = (char*) str;
	char current = 0;
	uint8_t startX = x;

	do
	{
		current = pgm_read_byte(ptr);
		ptr++;

		if(current >= FIRST_FONT_GLYPH && current <= LAST_FONT_GLYPH)
		{
			drawGlyph(current - FIRST_FONT_GLYPH, x, y);
		}

		x += FONT_WIDTH + 1;

		if(current == '\n')
		{
			x = startX;
			y += FONT_HEIGHT + 1;
		}
	} while(current);
}

void Renderer::drawInt(int8_t val, uint8_t x, uint8_t y)
{
	unsigned char c, i;

	for(i = 0; i < 3; i++)
	{
		c = val % 10;
		if(val > 0 || i == 0) 
		{
			drawGlyph(c + '0' - FIRST_FONT_GLYPH, x, y);
		}
		else
		{
			drawGlyph(' ' - FIRST_FONT_GLYPH, x, y);
		}
		x -= FONT_WIDTH + 1;
		val = val / 10;
	}
}
