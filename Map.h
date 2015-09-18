#ifndef MAP_H_
#define MAP_H_

#define MAP_OUT_OF_BOUNDS 0xff

enum MapRead_Orientation
{
	MapRead_Horizontal,
	MapRead_Vertical
};

class Map
{
public:
	bool isValid(int cellX, int cellZ);
	bool isBlocked(int cellX, int cellZ);
	uint8_t getTextureId(int cellX, int cellZ);
	uint8_t getTile(int cellX, int cellZ);

	int bufferX;
	int bufferZ;

	uint8_t m_mapBuffer[MAP_BUFFER_SIZE * MAP_BUFFER_SIZE];

	void updateBufferPosition(int newX, int newZ);

private:
	void streamData(uint8_t* buffer, MapRead_Orientation orientation, int x, int z, int length);
	void updateHorizontalSlice(int offsetZ);
	void updateVerticalSlice(int offsetX);
	void updateEntireBuffer();

};

#endif
