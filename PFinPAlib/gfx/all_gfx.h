// Graphics converted using PAGfx by Mollusk.

#pragma once

#include <PA_BgStruct.h>

#ifdef __cplusplus
extern "C"{
#endif

// Sprites:
extern const unsigned char puzzle_Sprite[10240] _GFX_ALIGN; // Palette: puzzle_Pal
extern const unsigned char bomb_Sprite[7168] _GFX_ALIGN; // Palette: bomb_Pal

// Backgrounds:
extern const PA_BgStruct down;
extern const PA_BgStruct bground_up;

// Palettes:
extern const unsigned short puzzle_Pal[256] _GFX_ALIGN;
extern const unsigned short bomb_Pal[256] _GFX_ALIGN;

#ifdef __cplusplus
}
#endif
