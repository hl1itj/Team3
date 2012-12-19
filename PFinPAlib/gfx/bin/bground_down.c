#include <PA_BgStruct.h>

extern const char bground_down_Tiles[];
extern const char bground_down_Map[];
extern const char bground_down_Pal[];

const PA_BgStruct bground_down = {
	PA_BgNormal,
	256, 192,

	bground_down_Tiles,
	bground_down_Map,
	{bground_down_Pal},

	10560,
	{1536}
};
