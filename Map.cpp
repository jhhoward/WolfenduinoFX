#include "Engine.h"
#include "Map.h"

const char mapBuffer[MAP_SIZE*MAP_SIZE+1] PROGMEM =
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
  return pgm_read_byte(mapBuffer + cellZ * MAP_SIZE + cellX) == '#';
}
