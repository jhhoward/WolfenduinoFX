#include "Save.h"
#include "Engine.h"

void SaveSystem::init()
{
	readSaveFile((uint8_t*) &saveFile, sizeof(SaveFile));
}

void SaveSystem::save()
{
	writeSaveFile((uint8_t*) &saveFile, sizeof(SaveFile));
}
