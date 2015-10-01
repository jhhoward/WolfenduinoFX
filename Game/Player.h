#ifndef PLAYER_H_
#define PLAYER_H_

#include "FixedMath.h"

enum WeaponType
{
	WeaponType_Knife,
	WeaponType_Pistol,
	WeaponType_MachineGun,
	WeaponType_ChainGun
};

class Player
{
public:
	Player();
	void init();
	void update();
	void move(int16_t deltaX, int16_t deltaZ);

	int16_t x, z;
	angle_t direction;

	struct
	{
		uint8_t type;
		uint8_t ammo;
		uint8_t frame;
		uint8_t time;
		bool debounce : 1;
		bool shooting : 1;
	} weapon;

private:
	void updateWeapon();
	void shootWeapon();
};

#endif
