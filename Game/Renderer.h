#ifndef RENDERER_H_
#define RENDERER_H_

#include <stdint.h>
#include "Engine.h"
#include "Defines.h"

#define NULL_QUEUE_ITEM 0xff
#define RENDER_QUEUE_CAPACITY 10

struct RenderQueueItem
{
	uint8_t* data;
	uint8_t x, w;
	uint8_t next;
};

class Renderer
{
public:
	void init();
	void drawFrame();

#ifdef DEFER_RENDER
	void drawDeferredFrame();
#endif

private:
	void initWBuffer();
	void drawFloorAndCeiling();  
	void drawCellWall(uint8_t textureId, int x1, int z1, int x2, int z2);
	void drawCell(int cellX, int cellZ);
/*inline*/ void drawStrip(int16_t x, int16_t w, int8_t u, uint8_t textureId);
	void drawWall(int16_t _x1, int16_t _z1, int16_t _x2, int16_t _z2, uint8_t textureId = 0, int8_t _u1 = 0, int8_t _u2 = 15);
	void drawFrustumCells();
	void drawBufferedCells();
	void drawDoors();

	void queueSprite(uint8_t* sprite, int16_t x, int16_t z);
	void drawQueuedSprite(uint8_t id);
	void drawWeapon();

	int16_t xpos, zpos;
	int16_t cos_dir;
	int16_t sin_dir;
	int8_t xcell, zcell;
	int8_t numColumns;
	uint8_t wbuffer[DISPLAYWIDTH];

#ifdef DEFER_RENDER
	uint8_t texbuffer[DISPLAYWIDTH];
	uint8_t ubuffer[DISPLAYWIDTH];
#endif

	uint8_t renderQueueHead;
	RenderQueueItem renderQueue[RENDER_QUEUE_CAPACITY];
};

class BitPairReader
{
public:
	BitPairReader(uint8_t* ptr, uint8_t offset = 0) : m_ptr(ptr)
	{
		m_readOffset = offset;
		m_lastRead = pgm_read_byte(ptr);
	}
	
	uint8_t read()
	{
		uint8_t result = (m_lastRead & (3 << m_readOffset)) >> m_readOffset;
		m_readOffset += 2;
		if(m_readOffset == 8)
		{
			m_ptr++;
			m_lastRead = pgm_read_byte(m_ptr);
			m_readOffset = 0;
		}

		return result;
	}
private:
	uint8_t* m_ptr;
	uint8_t m_lastRead;
	uint8_t m_readOffset;
};


#endif
