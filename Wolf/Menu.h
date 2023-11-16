#ifndef MENU_H_
#define MENU_H_

typedef const void* MenuData;

class Menu
{
public:
	void init();
	void draw();
	void update();

	const MenuData* currentMenu;
	int8_t currentSelection;
	int8_t debounceInput;
	int8_t selectionDelta;

	int8_t numMenuItems();

	void switchMenu(const MenuData* newMenu);

	static void chooseNewSlot();
	static void chooseDifficulty();
	static void loadGame();
	static void loadSelectedSave();
	static void quit();
	static void skillBaby();
	static void skillEasy();
	static void skillMedium();
	static void skillHard();
	static void viewScores();
	static void toggleSound();
	static void continueGame();
};

#endif
