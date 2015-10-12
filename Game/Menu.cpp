#include "Engine.h"
#include "Menu.h"

#define MENU_ENTRY_END 0
#define MENU_STR(x) (const void*)(x)
#define MENU_CALLBACK(x) (const void*)(x)

typedef void (*MenuFn)(void);

// Main menu
const char Str_Wolfenduino3D[] PROGMEM = "WOLFENDUINO 3D";
const char Str_NewGame[] PROGMEM = "NEW GAME";
const char Str_Sound[] PROGMEM = "SOUND:";
const char Str_On[] PROGMEM = "ON";
const char Str_Off[] PROGMEM = "OFF";
const char Str_Quit[] PROGMEM = "QUIT";

const void* Menu_Main[] PROGMEM = 
{
	Str_Wolfenduino3D,
	Str_NewGame,		MENU_CALLBACK(&Menu::newGame),
	Str_Sound,			MENU_CALLBACK(&Menu::toggleSound),
	Str_Quit,			MENU_CALLBACK(&Menu::quit),
	MENU_ENTRY_END
};

// Difficulty menu
const char Str_ChooseDifficulty[] PROGMEM = "HOW TOUGH ARE YOU?";
const char Str_SkillBaby[] PROGMEM = "CAN I PLAY, DADDY?";
const char Str_SkillEasy[] PROGMEM = "DON'T HURT ME.";
const char Str_SkillMedium[] PROGMEM = "BRING 'EM ON!";
const char Str_SkillHard[] PROGMEM = "I AM DEATH\n  INCARNATE!";

const void* Menu_ChooseDifficulty[] PROGMEM = 
{
	Str_ChooseDifficulty,
	Str_SkillBaby,		MENU_CALLBACK(&Menu::skillBaby),
	Str_SkillEasy,		MENU_CALLBACK(&Menu::skillEasy),
	Str_SkillMedium,	MENU_CALLBACK(&Menu::skillMedium),
	Str_SkillHard,		MENU_CALLBACK(&Menu::skillHard),
	MENU_ENTRY_END
};

void Menu::toggleSound()
{
	Platform.setMuted(!Platform.isMuted());
}

void Menu::newGame()
{
	engine.menu.switchMenu((MenuData*) Menu_ChooseDifficulty);
}

#ifdef PLATFORM_GAMEBUINO
extern Gamebuino gb;
void Menu::quit()
{
	gb.changeGame();
}
#else
void Menu::quit()
{
}
#endif

void Menu::skillBaby()
{
	engine.difficulty = Difficulty_Baby;
	engine.startLevel();
}

void Menu::skillEasy()
{
	engine.difficulty = Difficulty_Easy;
	engine.startLevel();
}

void Menu::skillMedium()
{
	engine.difficulty = Difficulty_Medium;
	engine.startLevel();
}

void Menu::skillHard()
{
	engine.difficulty = Difficulty_Hard;
	engine.startLevel();
}

void Menu::init()
{
	switchMenu((MenuData*) Menu_Main);
}

void Menu::draw()
{
	for(int j = 0; j < DISPLAYHEIGHT; j++)
		for(int i = 0; i < DISPLAYWIDTH; i++)
			clearPixel(i, j);

	engine.renderer.drawString((const char*)pgm_read_ptr(&currentMenu[0]), 5, 1);
	int index = 1;
	int y = 12;

	while(1)
	{
		if(pgm_read_ptr(&currentMenu[index]) == 0)
			break;
		engine.renderer.drawString((const char*)pgm_read_ptr(&currentMenu[index]), 8, y);

		if((const char*)pgm_read_ptr(&currentMenu[index]) == Str_Sound)
		{
			if(Platform.isMuted())
				engine.renderer.drawString(Str_Off, 40, y);
			else
				engine.renderer.drawString(Str_On, 40, y);
		}
		index += 2;
		y += 6;
	}

	engine.renderer.drawGlyph('*' - FIRST_FONT_GLYPH, 2, 12 + currentSelection * 6);
}

void Menu::update()
{
	if(!debounceInput)
	{
		if(Platform.readInput() & Input_Dpad_Up)
		{
			currentSelection --;
			if(currentSelection == -1)
				currentSelection = numMenuItems() - 1;
		}
		if(Platform.readInput() & Input_Dpad_Down)
		{
			currentSelection ++;
			if(currentSelection == numMenuItems())
				currentSelection = 0;
		}
		if(Platform.readInput() & Input_Btn_A)
		{
			MenuFn fn = (MenuFn)pgm_read_ptr(&currentMenu[currentSelection * 2 + 2]);
			fn();
		}
	}
	debounceInput = Platform.readInput() != 0;
}

int8_t Menu::numMenuItems()
{
	int8_t index = 1;
	int8_t count = 0;

	while(1)
	{
		if(pgm_read_ptr(&currentMenu[index]) == 0)
			break;
		index += 2;
		count++;
	}
	return count;
}

void Menu::switchMenu(MenuData* newMenu)
{
	currentMenu = newMenu;
	currentSelection = 0;
}
