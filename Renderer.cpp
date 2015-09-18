#include "Engine.h"
#include "Renderer.h"
#include "FixedMath.h"


#include <math.h> // temp
#include "textures.inc.h"


// temporary
const char texture[] PROGMEM =
{
  "#-...#-...#-===#"
  "#-...#-...#-===#"
  "#-...#-...######"
  "######-...#-...#"
  "#-===#-...#-...#"
  "#-===#-...#-...#"
  "#-===#-...#-...#"
  "#-===#-...#-...#"
  "#-===######-...#"
  "#-===#-...#-...#"
  "#-===#-...######"
  "#-===#-...#-===#"
  "######-...#-===#"
  "#-...#-...#-===#"
  "#-...#-...#-===#"
  "#-...######-===#"
};

#include <stdio.h>
int overdraw = 0;

void Renderer::init()
{
}

/*void Renderer::drawFrame()
{
}
*/
void Renderer::drawFrame()
{
	xpos = Engine::player.x;
	zpos = Engine::player.z;
	// TODO: move this into a LUT
 // cos_dir = (int16_t)((FIXED_ONE * cos(-Engine::player.direction * 3.141592 / 128.0f)) + 0.5f);
  //sin_dir = (int16_t)((FIXED_ONE * sin(-Engine::player.direction * 3.141592 / 128.0f)) + 0.5f);
	cos_dir = FixedMath::Cos(-Engine::player.direction);
	sin_dir = FixedMath::Sin(-Engine::player.direction);
	overdraw = 0;
  xcell = Engine::player.x / CELL_SIZE;
  zcell = Engine::player.z / CELL_SIZE;
  initWBuffer();
  drawFloorAndCeiling();

#if 1
  drawBufferedCells();
#elif 0
  for (int layer=1; (layer<MAP_BUFFER_SIZE) && (numColumns<DISPLAYWIDTH); layer++)
	  drawLayer(layer);
#else
  drawFrustumCells();
#endif

  //drawSprite(3.5f, 1.5f, monster, monster_mask);
  //drawSprite(3.5f, 11.5f, key, key_mask);

  if(overdraw > 0)
	printf("Overdraw: %d\n", overdraw);
}

int orient2d(int ax, int ay, int bx, int by, int cx, int cy)
{
    return (bx-ax)*(cy-ay) - (by-ay)*(cx-ax);
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

void Renderer::drawFrustumCells()
{
	angle_t leftSide = Engine::player.direction - CULLING_FOV;
	angle_t rightSide = Engine::player.direction + CULLING_FOV;
	
	#if 1
	int16_t lx = FixedMath::Sin(leftSide);
	int16_t lz = FixedMath::Cos(leftSide);
	int16_t rx = FixedMath::Sin(rightSide);
	int16_t rz = FixedMath::Cos(rightSide);
	
	int xd, zd;
	int x1, x2, z1, z2;

	if(sin_dir > 0)
	{
		xd = 1;		x1 = 0;		x2 = MAP_SIZE;
	}
	else
	{
		xd = -1;	x1 = MAP_SIZE - 1; x2 = -1;
	}
	if(cos_dir > 0)
	{
		zd = 1;		z1 = 0;		z2 = MAP_SIZE;
	}
	else
	{
		zd = -1;	z1 = MAP_SIZE - 1;	z2 = -1;
	}
	
	//if(mabs(cos_dir) > mabs(sin_dir))
	{
		//printf("Cells rendered:\n");
		for(int z = z1; z != z2; z += zd)
		{
			for(int x = x1; x != x2; x += xd)
			{
				if((lx * (x - xcell) + lz * (z - zcell)) >= 0
				&& (rx * (x - xcell) + rz * (z - zcell)) >= 0)
				{
					drawCell(x, z);
				}
			}
		}
	}
	/*else
	{
		for(int x = x1; x != x2; x += xd)
		{
			for(int z = z1; z != z2; z += zd)
			{
				if((lx * (x - xcell) + lz * (z - zcell)) >= 0
				&& (rx * (x - xcell) + rz * (z - zcell)) >= 0)
				{
					drawCell(x, z);
				}
			}
		}
	}*/
	

	
	#else
	int16_t lx = (xpos + FIXED_TO_INT((int32_t)FixedMath::Sin(leftSide) * (int32_t)DRAW_DISTANCE) + (CELL_SIZE / 2)) / CELL_SIZE;
	int16_t lz = (zpos + FIXED_TO_INT((int32_t)FixedMath::Cos(leftSide) * (int32_t)DRAW_DISTANCE) + (CELL_SIZE / 2)) / CELL_SIZE;
	int16_t rx = (xpos + FIXED_TO_INT((int32_t)FixedMath::Sin(rightSide) * (int32_t)DRAW_DISTANCE) + (CELL_SIZE / 2)) / CELL_SIZE;
	int16_t rz = (zpos + FIXED_TO_INT((int32_t)FixedMath::Cos(rightSide) * (int32_t)DRAW_DISTANCE) + (CELL_SIZE / 2)) / CELL_SIZE;

	// Compute bounding box
    int minX = min3(xcell, lx, rx);
    int minZ = min3(zcell, lz, rz);
    int maxX = max3(xcell, lx, rx);
    int maxZ = max3(zcell, lz, rz);

	minX = max(minX, 0);
	minZ = max(minZ, 0);
    maxX = min(maxX, MAP_SIZE - 1);
    maxZ = min(maxZ, MAP_SIZE - 1);

	int xd, zd;
	int x1, x2, z1, z2;

	if(sin_dir > 0)
	{
		xd = 1;
		x1 = minX;
		x2 = maxX + 1;
	}
	else
	{
		xd = -1;
		x1 = maxX;
		x2 = minX - 1;
	}
	if(cos_dir > 0)
	{
		zd = 1;
		z1 = minZ;
		z2 = maxZ + 1;
	}
	else
	{
		zd = -1;
		z1 = maxZ;
		z2 = minZ - 1;
	}

	if(mabs(cos_dir) > mabs(sin_dir))
	{
		//printf("Cells rendered:\n");
		for(int z = z1; z != z2; z += zd)
		{
			for(int x = x1; x != x2; x += xd)
			{
				// Determine barycentric coordinates
				int w0 = orient2d(lx, lz, rx, rz, x, z);
				int w1 = orient2d(rx, rz, xcell, zcell, x, z);
				int w2 = orient2d(xcell, zcell, lx, lz, x, z);

				// If p is on or inside all edges, render pixel.
				if (w0 <= 0 && w1 <= 0 && w2 <= 0)
				{
					drawCell(x, z);
					//printf("X: %d\tZ:%d\n", x, z);
				}
			}
		}
	}
	else
	{
		for(int x = x1; x != x2; x += xd)
		{
			for(int z = z1; z != z2; z += zd)
			{
				// Determine barycentric coordinates
				int w0 = orient2d(lx, lz, rx, rz, x, z);
				int w1 = orient2d(rx, rz, xcell, zcell, x, z);
				int w2 = orient2d(xcell, zcell, lx, lz, x, z);

				// If p is on or inside all edges, render pixel.
				if (w0 <= 0 && w1 <= 0 && w2 <= 0)
				{
					drawCell(x, z);
					//printf("X: %d\tZ:%d\n", x, z);
				}
			}
		}
	}
	#endif
}

void Renderer::initWBuffer()
{
  for (int i=0; i<DISPLAYWIDTH; i++)
    wbuffer[i] = 0;
  numColumns = 0;
}

void Renderer::drawFloorAndCeiling()
{
	for(int x = 0; x < DISPLAYWIDTH; x++)
	{
		for(int y = 0; y < DISPLAYHEIGHT; y++)
		{
#if 0
			Platform.drawPixel(x, y, 1);
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

void Renderer::drawLayer(int layer)
{
  drawCell(xcell, zcell-layer);
  drawCell(xcell, zcell+layer);
  drawCell(xcell-layer, zcell);
  drawCell(xcell+layer, zcell);
  for (int i=1; i<layer; i++)
  {
    drawCell(xcell-i, zcell+layer);
    drawCell(xcell+i, zcell-layer);
    drawCell(xcell-layer, zcell-i);
    drawCell(xcell+layer, zcell+i);
  }
  for (int i=1; (i<layer) && (numColumns<DISPLAYWIDTH); i++)
  {
    drawCell(xcell+i, zcell+layer);
    drawCell(xcell-i, zcell-layer);
    drawCell(xcell-layer, zcell+i);
    drawCell(xcell+layer, zcell-i);
  }
  drawCell(xcell-layer, zcell+layer);
  drawCell(xcell+layer, zcell+layer);
  drawCell(xcell-layer, zcell-layer);
  drawCell(xcell+layer, zcell-layer);
}

void Renderer::drawCellWall(uint8_t textureId, int x1, int z1, int x2, int z2)
{
	drawWall(x1 * CELL_SIZE, z1 * CELL_SIZE, x2 * CELL_SIZE, z2 * CELL_SIZE, textureId);
	//drawWall(x2 * CELL_SIZE, z2 * CELL_SIZE, x1 * CELL_SIZE, z1 * CELL_SIZE);
}

void Renderer::drawCell(int cellX, int cellZ)
{
  if (!Engine::map.isValid(cellX, cellZ))
    return;
  if (!Engine::map.isBlocked(cellX, cellZ))
    return;

  // clip cells behind us
	if((cos_dir * (cellX - xcell) - sin_dir * (cellZ - zcell)) <= 0)
		return;

	uint8_t textureId = Engine::map.getTextureId(cellX, cellZ);
	
  if (zpos < cellZ * CELL_SIZE)
  {
    if (xpos > cellX * CELL_SIZE)
    {
      // north west quadrant
      if ((zpos < cellZ * CELL_SIZE) && !Engine::map.isBlocked(cellX, cellZ-1))
        drawCellWall(textureId, cellX, cellZ, cellX+1, cellZ);  // south wall
      if ((xpos > (cellX+1) * CELL_SIZE) && (!Engine::map.isBlocked(cellX+1, cellZ)))
        drawCellWall(textureId, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
    }
    else
    {
      // north east quadrant
      if ((zpos < cellZ * CELL_SIZE) && !Engine::map.isBlocked(cellX, cellZ-1))
        drawCellWall(textureId, cellX, cellZ, cellX+1, cellZ);  // south wall
      if ((xpos< cellX * CELL_SIZE) && !Engine::map.isBlocked(cellX-1, cellZ))
        drawCellWall(textureId, cellX, cellZ+1, cellX, cellZ);  // west wall
    }
  }
  else
  {
    if (xpos > cellX * CELL_SIZE)
    {
      // south west quadrant
      if ((zpos > (cellZ+1) * CELL_SIZE) && !Engine::map.isBlocked(cellX, cellZ+1))
        drawCellWall(textureId, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
      if ((xpos > (cellX+1) * CELL_SIZE) && !Engine::map.isBlocked(cellX+1, cellZ))
        drawCellWall(textureId, cellX+1, cellZ, cellX+1, cellZ+1);  // east wall
    }
    else
    {
      // south east quadrant
      if ((zpos > (cellZ+1) * CELL_SIZE) && !Engine::map.isBlocked(cellX, cellZ+1))
        drawCellWall(textureId, cellX+1, cellZ+1, cellX, cellZ+1);  // north wall
      if ((xpos< cellX * CELL_SIZE) && !Engine::map.isBlocked(cellX-1, cellZ))
        drawCellWall(textureId, cellX, cellZ+1, cellX, cellZ);  // west wall
    }
  }
}

/*inline*/ void Renderer::drawStrip(int16_t x, int16_t w, int8_t u, uint8_t textureId)
{
	int halfW = w >> 1;
	int y1 = (HALF_DISPLAYHEIGHT) - halfW;
	int y2 = (HALF_DISPLAYHEIGHT) + halfW;
	int verror = halfW;

	//int8_t* texPtr = (int8_t*) texture + u * 16;
	//char texData = pgm_read_byte(texPtr);
	BitPairReader textureReader((uint8_t*) textureData + u * TEXTURE_STRIDE + textureId * (TEXTURE_STRIDE * TEXTURE_SIZE));
	uint8_t texData = textureReader.read();
	
	for(int y = y1; y <= y2; y++)
	{
		if(y >= 0 && y < DISPLAYHEIGHT)
		{
			/*switch(texData)
			{
			default:
				Platform.drawPixel(x, y, 1);
				break;
			case '#':
				Platform.drawPixel(x, y, 0);
				break;
			case '-':
				if((x ^ y) & 1)
				{
					Platform.drawPixel(x, y, 1);
				}
				else
				{
					Platform.drawPixel(x, y, 0);
				}
				break;
			case '=':
				if((x & y) & 1)
				{
					Platform.drawPixel(x, y, 0);
				}
				else
				{
					Platform.drawPixel(x, y, 1);
				}
				break;
			}*/
			switch(texData)
			{
			case 1:
				Platform.drawPixel(x, y, 1);
				break;
			case 2:
				Platform.drawPixel(x, y, 0);
				break;
			case 0:
				if((x ^ y) & 1)
				{
					Platform.drawPixel(x, y, 1);
				}
				else
				{
					Platform.drawPixel(x, y, 0);
				}
				break;
			case 3:
				if((x & y) & 1)
				{
					Platform.drawPixel(x, y, 0);
				}
				else
				{
					Platform.drawPixel(x, y, 1);
				}
				break;
			}

		}
		
		verror -= 15;
		
		while(verror < 0)
		{
			//texPtr++;
			//texData = pgm_read_byte(texPtr);
			texData = textureReader.read();
			verror += w;
		}
	}
}

// draws one side of a cell
void Renderer::drawWall(int16_t _x1, int16_t _z1, int16_t _x2, int16_t _z2, uint8_t textureId, int8_t _u1, int8_t _u2)
{
  // find position of wall edges relative to eye
  /*int16_t x1 = (int16_t)(FIXED_TO_INT(cos_dir * (int32_t)(_x1-xpos)) - FIXED_TO_INT(sin_dir * (int32_t)(_z1-zpos)));
  int16_t z1 = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_x1-xpos)) + FIXED_TO_INT(cos_dir * (int32_t)(_z1-zpos)));
  int16_t x2 = (int16_t)(FIXED_TO_INT(cos_dir * (int32_t)(_x2-xpos)) - FIXED_TO_INT(sin_dir * (int32_t)(_z2-zpos)));
  int16_t z2 = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_x2-xpos)) + FIXED_TO_INT(cos_dir * (int32_t)(_z2-zpos)));
  */
  /*int16_t x1 = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_z1-zpos)) - FIXED_TO_INT(cos_dir * (int32_t)(_x1-xpos)));
  int16_t z1 = (int16_t)(- FIXED_TO_INT(sin_dir * (int32_t)(_x1-xpos)) - FIXED_TO_INT(cos_dir * (int32_t)(_z1-zpos)));
  int16_t x2 = (int16_t)(FIXED_TO_INT(sin_dir * (int32_t)(_z2-zpos)) - FIXED_TO_INT(cos_dir * (int32_t)(_x2-xpos)));
  int16_t z2 = (int16_t)(- FIXED_TO_INT(sin_dir * (int32_t)(_x2-xpos)) - FIXED_TO_INT(cos_dir * (int32_t)(_z2-zpos)));*/

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
#if 1
		drawStrip(x, w, u, textureId);
#else 
      
      // calculate top and bottom
      int sy1 = (int)(DISPLAYHEIGHT / 2 - w / 2);
      int sy2 = (int)(DISPLAYHEIGHT / 2 + w / 2) - 1;
  
      // clamp to the visible portion of the screen
      int firsty = max(sy1, 0);
      int lasty = min(sy2, DISPLAYHEIGHT-1);
      
      // draw this column
      if ((x==sx1) || (x==sx2))
        for (int y=firsty; y<=lasty; y++)
          Platform.drawPixel(x, y, 0);
        else
          for (int y=firsty; y<=lasty; y++)
	          Platform.drawPixel(x, y, 1);
      if (sy1 >= 0)
          Platform.drawPixel(x, sy1, 0);

      if (sy2 < DISPLAYHEIGHT)
          Platform.drawPixel(x, sy2, 0);
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

