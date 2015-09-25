#ifndef PLAYER_H_
#define PLAYER_H_

#include "FixedMath.h"

class Player
{
public:
	void update();
	void move(int16_t deltaX, int16_t deltaZ);

	int16_t x, z;
	angle_t direction;
};

#endif
