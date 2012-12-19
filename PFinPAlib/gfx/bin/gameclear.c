#include <PA_BgStruct.h>

extern const char gameclear_Tiles[];
extern const char gameclear_Map[];
extern const char gameclear_Pal[];

const PA_BgStruct gameclear = {
	PA_BgNormal,
	256, 192,

	gameclear_Tiles,
	gameclear_Map,
	{gameclear_Pal},

	4992,
	{1536}
};
