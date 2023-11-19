#include <string.h>
#include "Engine.h"
#include "Menu.h"
#include "Sounds.h"
#include "Save.h"
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
const char Str_Help[] PROGMEM = "HELP";

const void* const Menu_Main[] PROGMEM = 
{
	Str_Wolfenduino3D,
	Str_NewGame,		MENU_CALLBACK(&Menu::chooseNewSlot),
	Str_LoadGame,		MENU_CALLBACK(&Menu::loadGame),
	Str_ViewScores,		MENU_CALLBACK(&Menu::viewScores),
	Str_Sound,			MENU_CALLBACK(&Menu::toggleSound),
	Str_Help,			MENU_CALLBACK(&Menu::showHelp),
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
	Str_SkillBaby,		MENU_CALLBACK(&Menu::setDifficulty),
	Str_SkillEasy,		MENU_CALLBACK(&Menu::setDifficulty),
	Str_SkillMedium,	MENU_CALLBACK(&Menu::setDifficulty),
	Str_SkillHard,		MENU_CALLBACK(&Menu::setDifficulty),
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
const char Str_SaveSlot[] PROGMEM = "";
const char Str_EmptySlot[] PROGMEM = "EMPTY SLOT";
const void* const Menu_SelectSlot[] PROGMEM =
{
	Str_SelectSlot,
	Str_SaveSlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::chooseDifficulty),
	MENU_ENTRY_END
};


// Load game menu
const void* const Menu_LoadGame[] PROGMEM =
{
	Str_LoadGame,
	Str_SaveSlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	Str_SaveSlot,		MENU_CALLBACK(&Menu::loadSelectedSave),
	MENU_ENTRY_END
};

// Help screen
const void* const Menu_Help[] PROGMEM =
{
	Str_Wolfenduino3D,
	MENU_ENTRY_END
};


const char Str_Overwrite[] PROGMEM = "OVERWRITE THIS SAVE?";
const char Str_Yes[] PROGMEM = "YES";
const char Str_No[] PROGMEM = "NO";

// Overwrite dialog menu
const void* const Menu_OverwriteSlot[] PROGMEM =
{
	Str_Overwrite,
	Str_Yes,		MENU_CALLBACK(&Menu::chooseDifficulty),
	Str_No,			MENU_CALLBACK(&Menu::chooseNewSlot),
	MENU_ENTRY_END
};

const char Str_GameOver[] PROGMEM = "GAME OVER";
const char Str_FinalScore[] PROGMEM = "FINAL SCORE:";

// Game over menu
const void* const Menu_GameOver[] PROGMEM =
{
	Str_GameOver,
	MENU_ENTRY_END
};

void Menu::loadSelectedSave()
{
	engine.save.activeSlot = engine.menu.currentSelection;
	if (engine.save.saveFile.slots[engine.save.activeSlot].hp)
	{
		engine.loadGame();
	}
	else
	{
		Platform.playSound(NOWAYSND);
	}
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

void Menu::showHelp()
{
	engine.menu.switchMenu(Menu_Help);
}

void Menu::chooseDifficulty()
{
	if (engine.menu.currentMenu == Menu_SelectSlot)
	{
		engine.save.activeSlot = engine.menu.currentSelection;
		if (engine.save.saveFile.slots[engine.save.activeSlot].hp)
		{
			engine.menu.switchMenu(Menu_OverwriteSlot);
			return;
		}
	}
	engine.menu.switchMenu(Menu_ChooseDifficulty);
	engine.menu.currentSelection = 2;
}

void Menu::viewScores()
{
	engine.menu.switchMenu(Menu_ViewScores);
}

void Menu::setDifficulty()
{
	engine.difficulty = engine.menu.currentSelection;
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

//	clearDisplay(1);
//	engine.renderer.drawString(PSTR("MOVE"), 0, 0, 0);
//	engine.renderer.drawString(PSTR("SHOOT"), 0, 6, 0);
//	engine.renderer.drawString(PSTR("HOLD TO STRAFE"), 0, 12, 0);
//	engine.renderer.drawString(PSTR("DOUBLE TAP"), 0, 18, 0);
//	engine.renderer.drawString(PSTR("TO SWAP WEAPON"), 0, 24, 0);
//	return;

	if (currentMenu == Menu_Main)
	{
		engine.renderer.drawBackground(Data_titleBG);
		startY += 5;
		engine.renderer.drawString(PSTR("@JAMESHHOWARD"), DISPLAYWIDTH - 52, DISPLAYHEIGHT - FONT_HEIGHT - 1, 0);
	}
	else if (currentMenu == Menu_Help)
	{
		engine.renderer.drawBackground(Data_helpBG);
		return;
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
	int item = 0;

	while(1)
	{
		if(pgm_read_ptr(&currentMenu[index]) == 0)
			break;

		const char* text = (const char*)pgm_read_ptr(&currentMenu[index]);

		if (text == Str_SaveSlot)
		{
			SaveSlot* slot = &engine.save.saveFile.slots[item];
			if (slot->hp == 0)
			{
				engine.renderer.drawString(Str_EmptySlot, x, y, 0);
			}
			else
			{
				// FLOOR:XX SKILL: NORMAL
				engine.renderer.drawInt(slot->level + 1, x + 28, y, 0);
				engine.renderer.drawString(PSTR("FLOOR:"), x, y, 0);
				engine.renderer.drawString(PSTR("SKILL:"), x + 38, y, 0);
				switch (slot->difficulty)
				{
				case 0:
					engine.renderer.drawString(PSTR("BABY"), x + 66, y, 0);
					break;
				case 1:
					engine.renderer.drawString(PSTR("EASY"), x + 66, y, 0);
					break;
				case 2:
					engine.renderer.drawString(PSTR("NORMAL"), x + 66, y, 0);
					break;
				case 3:
					engine.renderer.drawString(PSTR("HARD"), x + 66, y, 0);
					break;
				}
			}
		}
		else
		{
			engine.renderer.drawString(text, x, y, 0);
		}

		if(text == Str_Sound)
		{
			if(Platform.isMuted())
				engine.renderer.drawString(Str_Off, 40, y, 0);
			else
				engine.renderer.drawString(Str_On, 40, y, 0);
		}
		index += 2;
		y += itemSpacing;
		item++;
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
	else if (currentMenu == Menu_ViewScores)
	{
		for (int n = 0; n < 3; n++)
		{
			for (int j = 0; j < 3; j++)
			{
				engine.renderer.drawGlyph(engine.save.saveFile.scores[n].name[j] - FIRST_FONT_GLYPH, x + 16 + j * 4, y, 0);
			}
			engine.renderer.drawLong(engine.save.saveFile.scores[n].score, x + 80, y, 0);
			y += itemSpacing;
		}
	}
	else if (currentMenu == Menu_GameOver)
	{
		engine.renderer.drawSprite2D(UI_BJFace_Dead, 20, startY);
		engine.renderer.drawString(Str_FinalScore, 64, startY + 8, 0);
		engine.renderer.drawInt(engine.player.score, 64 + 44, startY + 16, 0);
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
				selectionDelta = 2;
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
				selectionDelta = -2;
				if (currentSelection == numMenuItems())
				{
					currentSelection = 0;
					selectionDelta = 0;
				}
				Platform.playSound(MOVEGUN1SND);
			}
			if (Platform.readInput() & Input_Btn_B)
			{
				Platform.playSound(SHOOTSND);
				MenuFn fn = (MenuFn)pgm_read_ptr(&currentMenu[currentSelection * 2 + 2]);
				fn();
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
