#ifndef SDLPLATFORM_H_
#define SDLPLATFORM_H_

#include <SDL.h>
#include "Platform.h"

class SDLPlatform : public PlatformBase
{
public:
	void init();
	void run();
	void drawPixel(uint8_t x, uint8_t y, uint8_t colour);
	void playSound(uint8_t id) {}
	bool isMuted() { return m_isMuted; }
	void setMuted(bool muted) { m_isMuted = muted; }
	
private:
	void drawPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

	void updateInputState(int eventType, bool pressed);

	SDL_Window* m_appWindow;
	SDL_Renderer* m_appRenderer;
	SDL_Surface* m_screenSurface;
	SDL_Texture* m_screenTexture;
	bool m_isRunning;
	bool m_isMuted;
};

extern SDLPlatform Platform;

inline void drawPixel(uint8_t x, uint8_t y, uint8_t colour)
{
	Platform.drawPixel(x, y, colour);
}
inline void setPixel(uint8_t x, uint8_t y)
{
	Platform.drawPixel(x, y, 0);
}
inline void clearPixel(uint8_t x, uint8_t y)
{
	Platform.drawPixel(x, y, 1);
}

#endif
