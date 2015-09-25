// SDL platform
#include <stdio.h>
#include "SDLPlatform.h"
#include "Engine.h"

SDLPlatform Platform;

void SDLPlatform::init()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer( DISPLAYWIDTH * ZOOM_SCALE, DISPLAYHEIGHT * ZOOM_SCALE, SDL_WINDOW_RESIZABLE, &m_appWindow, &m_appRenderer );
	SDL_RenderSetLogicalSize(m_appRenderer, DISPLAYWIDTH, DISPLAYHEIGHT);
	
	m_screenSurface = SDL_CreateRGBSurface(0, DISPLAYWIDTH, DISPLAYHEIGHT, 32, 
											0x000000ff,
											0x0000ff00, 
											0x00ff0000, 
											0xff000000
											);
	m_screenTexture = SDL_CreateTexture(m_appRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, m_screenSurface->w, m_screenSurface->h);
	m_isRunning = true;
}

int SDL_KeyboardButtonMappings[] = 
{
	SDLK_x, Input_Btn_A, 
	SDLK_z, Input_Btn_B, 
	SDLK_LEFT, Input_Dpad_Left,
	SDLK_RIGHT, Input_Dpad_Right,
	SDLK_UP, Input_Dpad_Up,
	SDLK_DOWN, Input_Dpad_Down,
	SDLK_RETURN, Input_Btn_C,
};

void SDLPlatform::updateInputState(int eventType, bool pressed)
{
	int numMappings = sizeof(SDL_KeyboardButtonMappings) / 2;
	
	for(int n = 0; n < numMappings; n++)
	{
		if(SDL_KeyboardButtonMappings[n * 2] == eventType)
		{
			if(pressed)
			{
				inputState |= SDL_KeyboardButtonMappings[n * 2 + 1];
			}
			else
			{
				inputState &= ~(SDL_KeyboardButtonMappings[n * 2 + 1]);
			}
		}
	}
}

void SDLPlatform::run()
{
	m_isRunning = true;
	
	Engine::init();
	
	while(m_isRunning)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			switch(event.type) 
			{
				case SDL_QUIT:
				m_isRunning = false;
				break;
				case SDL_KEYDOWN:
				updateInputState(event.key.keysym.sym, true);
				break;
				case SDL_KEYUP:
				updateInputState(event.key.keysym.sym, false);
				break;
			}

		}

		SDL_SetRenderDrawColor ( m_appRenderer, 206, 221, 231, 255 );
		SDL_RenderClear ( m_appRenderer );

		Engine::update();

		SDL_UpdateTexture(m_screenTexture, NULL, m_screenSurface->pixels, m_screenSurface->pitch);
		SDL_RenderCopy(m_appRenderer, m_screenTexture, NULL, NULL);
		SDL_RenderPresent(m_appRenderer);

		SDL_Delay(1000 / 30);
	}

	SDL_Quit();
}

int main(int, char**)
{
	for(int n = 0; n < 256; n++)
	{
		printf("%d %d\n", n, FixedMath::Sin(n));
	}

	Platform.init();

	Platform.run();

	return 0;
}

void SDLPlatform::drawPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    *(Uint32 *)p = pixel;
}

uint8_t paletteColours[] =
{
#if defined(EMULATE_GAMEBUINO)
	0, 0, 0,
	206, 221, 231,
#elif defined(EMULATE_ARDUBOY)
	0, 0, 0,
	255, 255, 255,
#else
	0, 0, 0,
	255, 255, 255,
	145, 145, 170,
	182, 182, 170
#endif
};

void SDLPlatform::drawPixel(uint8_t x, uint8_t y, uint8_t colour)
{
	Uint32 col = SDL_MapRGBA(m_screenSurface->format, paletteColours[colour * 3], paletteColours[colour * 3 + 1], paletteColours[colour * 3 + 2], 255);
	drawPixel(m_screenSurface, x, y, col);

	/*if(!colour)
	{
		Uint32 black = SDL_MapRGBA(m_screenSurface->format, 0, 0, 0, 255);
		drawPixel(m_screenSurface, x, y, black);
	}
	else
	{
		Uint32 white = SDL_MapRGBA(m_screenSurface->format, 206, 221, 231, 255);
		drawPixel(m_screenSurface, x, y, white);
	}*/
}
