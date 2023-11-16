#include <string.h>
#include "Engine.h"
#include "Menu.h"
#include "Sounds.h"
#include "Generated/fxdata.h"

#define MENU_ENTRY_END 0
#define MENU_STR(x) (const void*)(x)
#define MENU_CALLBACK(x) (const void*)(x)

typedef void (*MenuFn)(void);

// Main menu
const char Str_Wolfenduino3D[] PROGMEM = "WOLFENDUINO 3D";
const char Str_Continue[] PROGMEM = "CONTINUE";
const char Str_NewGame[] PROGMEM = "NEW GAME";
const char Str_Sound[] PROGMEM = "SOUND:";
const char Str_ViewScores[] PROGMEM = "VIEW SCORES";
const char Str_LoadGame[] PROGMEM = "LOAD GAME";
const char Str_On[] PROGMEM = "ON";
const char Str_Off[] PROGMEM = "OFF";

const void* const Menu_Main[] PROGMEM = 
{
	Str_Wolfenduino3D,
	Str_NewGame,		MENU_CALLBACK(&Menu::chooseNewSlot),
	Str_LoadGame,		MENU_CALLBACK(&Menu::loadGame),
	Str_ViewScores,		MENU_CALLBACK(&Menu::viewScores),
	Str_Sound,			MENU_CALLBACK(&Menu::toggleSound),
	MENU_ENTRY_END
};

// Difficulty menu
const char Str_ChooseDifficulty[] PROGMEM = "HOW TOUGH ARE YOU?";
const char Str_SkillBaby[] PROGMEM = "CAN I PLAY, DADDY?";
const char Str_SkillEasy[] PROGMEM = "DON'T HURT ME.";
const char Str_SkillMedium[] PROGMEM = "BRING 'EM ON!";
const char Str_SkillHard[] PROGMEM = "I AM DEATH INCARNATE!";

const void* const Menu_ChooseDifficulty[] PROGMEM = 
{
	Str_ChooseDifficulty,
	Str_SkillBaby,		MENU_CALLBACK(&Menu::skillBaby),
	Str_SkillEasy,		MENU_CALLBACK(&Menu::skillEasy),
	Str_SkillMedium,	MENU_CALLBACK(&Menu::skillMedium),
	Str_SkillHard,		MENU_CALLBACK(&Menu::skillHard),
	MENU_ENTRY_END
};

// High scores screen
const char Str_HighScores[] PROGMEM = "HIGH SCORES";
const void* const Menu_ViewScores[] PROGMEM =
{
	Str_HighScores,
	MENU_ENTRY_END
};

// Select slot menu
const char Str_SelectSlot[] PROGMEM = "CHOOSE A SLOT";
const char Str_EmptySlot[] PROGMEM = "EMPTY SLOT";
const void* const Menu_SelectSlot[] PROGMEM =
{
	Str_SelectSlot,
	Str_EmptySlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_EmptySlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_EmptySlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	MENU_ENTRY_END
};


// Load game menu
const void* const Menu_LoadGame[] PROGMEM =
{
	Str_LoadGame,
	Str_EmptySlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	Str_EmptySlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	Str_EmptySlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	MENU_ENTRY_END
};

void Menu::loadSelectedSave()
{
	Platform.playSound(NOWAYSND);
}

void Menu::toggleSound()
{
	Platform.setMuted(!Platform.isMuted());
}

void Menu::chooseNewSlot()
{
	engine.menu.switchMenu(Menu_SelectSlot);
}

void Menu::loadGame()
{
	engine.menu.switchMenu(Menu_LoadGame);
}

void Menu::chooseDifficulty()
{
	engine.menu.switchMenu(Menu_ChooseDifficulty);
}

void Menu::viewScores()
{
	engine.menu.switchMenu(Menu_ViewScores);
}

void Menu::continueGame()
{
	engine.gameState = GameState_Playing;
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
	engine.startNewGame();
}

void Menu::skillEasy()
{
	engine.difficulty = Difficulty_Easy;
	engine.startNewGame();
}

void Menu::skillMedium()
{
	engine.difficulty = Difficulty_Medium;
	engine.startNewGame();
}

void Menu::skillHard()
{
	engine.difficulty = Difficulty_Hard;
	engine.startNewGame();
}

void Menu::init()
{
	switchMenu(Menu_Main);
}

void Menu::draw()
{
	int startY = 20;
	int itemSpacing = 8;

	if (currentMenu == Menu_Main)
	{
		engine.renderer.drawBackground(Data_titleBG);
		startY += 6;
		engine.renderer.drawString(PSTR("@JAMESHHOWARD"), DISPLAYWIDTH / 2 - 26, DISPLAYHEIGHT - FONT_HEIGHT - 1, 0);
	}
	else
	{
		clearDisplay(1);

		const char* title = (const char*)pgm_read_ptr(&currentMenu[0]);
		int titleWidth = strlen_P(title) * 4;

		engine.renderer.drawBox(0, 4, DISPLAYWIDTH, FONT_HEIGHT, 0);
		engine.renderer.drawBox(DISPLAYWIDTH / 2 - titleWidth / 2 - 2, 2, titleWidth + 4, FONT_HEIGHT + 4, 0);
		engine.renderer.drawString(title, DISPLAYWIDTH / 2 - titleWidth / 2, 4, 1);
	}

	int index = 1;
	int x = 14;
	int y = startY;

	while(1)
	{
		if(pgm_read_ptr(&currentMenu[index]) == 0)
			break;

		engine.renderer.drawString((const char*)pgm_read_ptr(&currentMenu[index]), x, y, 0);

		if((const char*)pgm_read_ptr(&currentMenu[index]) == Str_Sound)
		{
			if(Platform.isMuted())
				engine.renderer.drawString(Str_Off, 40, y, 0);
			else
				engine.renderer.drawString(Str_On, 40, y, 0);
		}
		index += 2;
		y += itemSpacing;
	}

	if (numMenuItems())
	{
		int positionDelta = 0;
		if (selectionDelta < 0)
		{
			positionDelta = -itemSpacing / 2;
		}
		else if (selectionDelta > 0)
		{
			positionDelta = itemSpacing / 2;
		}
		engine.renderer.drawSprite2D(UI_Gun, 2, startY + currentSelection * itemSpacing + positionDelta);
	}

	if (currentMenu == Menu_ChooseDifficulty)
	{
		engine.renderer.drawSprite2D(UI_BJFace_Baby + currentSelection, DISPLAYWIDTH - 30, startY);
	}
}

void Menu::update()
{
	if (selectionDelta != 0)
	{
		if (selectionDelta > 0)
			selectionDelta--;
		if (selectionDelta < 0)
			selectionDelta++;

		if (!selectionDelta)
		{
			Platform.playSound(MOVEGUN2SND);
		}
	}

	if(!debounceInput)
	{
		if (numMenuItems())
		{
			if (Platform.readInput() & Input_Dpad_Up)
			{
				currentSelection--;
				selectionDelta = 3;
				if (currentSelection == -1)
				{
					currentSelection = numMenuItems() - 1;
					selectionDelta = 0;
				}
				Platform.playSound(MOVEGUN1SND);
			}
			if (Platform.readInput() & Input_Dpad_Down)
			{
				currentSelection++;
				selectionDelta = -3;
				if (currentSelection == numMenuItems())
				{
					currentSelection = 0;
					selectionDelta = 0;
				}
				Platform.playSound(MOVEGUN1SND);
			}
			if (Platform.readInput() & Input_Btn_B)
			{
				MenuFn fn = (MenuFn)pgm_read_ptr(&currentMenu[currentSelection * 2 + 2]);
				fn();
				Platform.playSound(SHOOTSND);
			}
		}
		else
		{
			if (Platform.readInput() & Input_Btn_B)
			{
				switchMenu(Menu_Main);
				Platform.playSound(ESCPRESSEDSND);
			}
		}

		if(Platform.readInput() & Input_Btn_A)
		{
			if(currentMenu != Menu_Main)
			{
				switchMenu(Menu_Main);
				Platform.playSound(ESCPRESSEDSND);
			}
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

void Menu::switchMenu(const MenuData* const newMenu)
{
	currentMenu = newMenu;
	currentSelection = 0;
	selectionDelta = 0;
	debounceInput = true;
	engine.fadeTransition();
}
