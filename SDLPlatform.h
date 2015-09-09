#ifndef SDLPLATFORM_H_
#define SDLPLATFORM_H_

#include <SDL.h>
#include "Platform.h"

class SDLPlatform : public PlatformBase
{
public:
	void init();
	void run();
	virtual void drawPixel(uint8_t x, uint8_t y, uint8_t colour);
	
private:
	void drawPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

	void updateInputState(int eventType, bool pressed);

	SDL_Window* m_appWindow;
	SDL_Renderer* m_appRenderer;
	SDL_Surface* m_screenSurface;
	SDL_Texture* m_screenTexture;
	bool m_isRunning;
};

extern SDLPlatform Platform;

#endif
