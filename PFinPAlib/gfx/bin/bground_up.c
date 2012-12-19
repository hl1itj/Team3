#include <PA_BgStruct.h>

extern const char bground_up_Tiles[];
extern const char bground_up_Map[];
extern const char bground_up_Pal[];

const PA_BgStruct bground_up = {
	PA_BgNormal,
	256, 192,

	bground_up_Tiles,
	bground_up_Map,
	{bground_up_Pal},

	320,
	{1536}
};
