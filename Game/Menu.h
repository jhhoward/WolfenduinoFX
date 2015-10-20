#ifndef MENU_H_
#define MENU_H_

typedef const void* MenuData;

class Menu
{
public:
	void init();
	void draw();
	void update();

	MenuData* currentMenu;
	int8_t currentSelection;
	int8_t debounceInput;

	int8_t numMenuItems();

	void switchMenu(MenuData* newMenu);

	static void newGame();
	static void quit();
	static void skillBaby();
	static void skillEasy();
	static void skillMedium();
	static void skillHard();
	static void toggleSound();
	static void continueGame();
};

extern const void* Menu_Paused[];

#endif
