#include <PA_BgStruct.h>

extern const char down_Tiles[];
extern const char down_Map[];
extern const char down_Pal[];

const PA_BgStruct down = {
	PA_BgNormal,
	256, 192,

	down_Tiles,
	down_Map,
	{down_Pal},

	128,
	{1536}
};
