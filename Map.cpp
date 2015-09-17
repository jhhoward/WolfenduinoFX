#include "Engine.h"
#include "Map.h"

const char mapBuffer[MAP_BUFFER_SIZE*MAP_BUFFER_SIZE+1] PROGMEM =
{
  "################"
  "#.#............#"
  "#.#..#.........#"
  "#.#..#.........#"
  "#....#....#....#"
  "##.#.#.........#"
  "######.........#"
  "#..............#"
  "##########..####"
  "#........#.....#"
  "#.....#..#..#..#"
  "#.....#.....#..#"
  "#.....##########"
  "#..............#"
  "#.#.#.#.#......#"
  "################"
};

#include "map0.inc.h"

bool Map::isValid(int cellX, int cellZ)
{
  if (cellX < 0)
    return false;
  if (cellX >= MAP_SIZE)
    return false;
  if (cellZ < 0)
    return false;
  if (cellZ >= MAP_SIZE)
    return false;
  return true;
}

bool Map::isBlocked(int cellX, int cellZ)
{
//  return mapBuffer[cellZ * MAP_SIZE + cellX] == '#';
  return pgm_read_byte(mapData + cellZ * MAP_SIZE + cellX) != 0;// == '#';
//  return pgm_read_byte(mapBuffer + cellZ * MAP_SIZE + cellX) == '#';
}

uint8_t Map::getTextureId(int cellX, int cellZ)
{
	return pgm_read_byte(mapData + cellZ * MAP_SIZE + cellX) - 1;
}

