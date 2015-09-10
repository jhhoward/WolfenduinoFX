#ifndef DEFINES_H_
#define DEFINES_H_

#define PLATFORM_GAMEBUINO

#define DISPLAYWIDTH 84
#define DISPLAYHEIGHT 48

#define HALF_DISPLAYWIDTH (DISPLAYWIDTH >> 1)
#define HALF_DISPLAYHEIGHT (DISPLAYHEIGHT >> 1)

// WIN32 specific
#ifdef WIN32
#define ZOOM_SCALE 3

#define PROGMEM
#define pgm_read_byte(x) (*((uint8_t*)x))
#define pgm_read_word(x) (*((uint16_t*)x))

#define pgm_read_ptr(x) (*((uintptr_t*)x))
#endif
// end

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))


#define CELL_SIZE 32
#define MAP_SIZE 16
#define FOV 60
#define NEAR_PLANE 73
//#define NEAR_PLANE 104
//#define NEAR_PLANE (LCDWIDTH * (0.5/tan(PI*(FOV / 2)/180)))
#define CLIP_PLANE 1
#define CAMERA_SCALE 1
//#define WALL_HEIGHT 1.0f
#define MOVEMENT 5
#define TURN 3


#endif
