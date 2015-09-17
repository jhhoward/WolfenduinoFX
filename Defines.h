#ifndef DEFINES_H_
#define DEFINES_H_

#define DISPLAYWIDTH 84
#define DISPLAYHEIGHT 48

#define HALF_DISPLAYWIDTH (DISPLAYWIDTH >> 1)
#define HALF_DISPLAYHEIGHT (DISPLAYHEIGHT >> 1)

// WIN32 specific
#ifdef _WIN32
#define ZOOM_SCALE 1

#define PROGMEM
#define pgm_read_byte(x) (*((uint8_t*)x))
#define pgm_read_word(x) (*((uint16_t*)x))

#define pgm_read_ptr(x) (*((uintptr_t*)x))
#endif
// end

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define min3(a, b, c) (min(min(a, b), c))
#define max3(a, b, c) (max(max(a, b), c))
#define sign(x) ((x) < 0 ? -1 : 1)
#define mabs(x) ((x) < 0 ? -(x) : (x))

#define CELL_SIZE 32
#define MAP_SIZE 64
#define MAP_BUFFER_SIZE 16
//#define MAP_SIZE 16

#define TEXTURE_SIZE 16
#define TEXTURE_STRIDE 4

// ~60 degree field of view (these values in wacky 256 unit format)
#define FOV 44
#define HALF_FOV 22
#define CULLING_FOV 35
#define DRAW_DISTANCE (CELL_SIZE * 20)

#define NEAR_PLANE 73
//#define NEAR_PLANE 104
//#define NEAR_PLANE (LCDWIDTH * (0.5/tan(PI*(FOV / 2)/180)))
#define CLIP_PLANE 1
#define CAMERA_SCALE 1
//#define WALL_HEIGHT 1.0f
#define MOVEMENT 7
#define TURN 3


#endif