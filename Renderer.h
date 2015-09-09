#ifndef RENDERER_H_
#define RENDERER_H_

#include <stdint.h>
#include "Defines.h"

class Renderer
{
public:
	void init();
	void drawFrame();

private:
	void initWBuffer();
	void drawFloorAndCeiling();  
	void drawLayer(int layer);
	void drawCellWall(int x1, int z1, int x2, int z2);
	void drawCell(int cellX, int cellZ);
/*inline*/ void drawStrip(int16_t x, int16_t w, int8_t u);
	void drawWall(int16_t _x1, int16_t _z1, int16_t _x2, int16_t _z2, int8_t _u1 = 0, int8_t _u2 = 15);

	int16_t xpos, zpos;
	int16_t cos_dir;
	int16_t sin_dir;
	int8_t xcell, zcell;
	int8_t numColumns;
	uint8_t wbuffer[DISPLAYWIDTH];
};

#endif
