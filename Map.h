#ifndef MAP_H_
#define MAP_H_

class Map
{
public:
	bool isValid(int cellX, int cellZ);
	bool isBlocked(int cellX, int cellZ);
	uint8_t getTextureId(int cellX, int cellZ);
};

#endif
