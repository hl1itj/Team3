#include <PA_BgStruct.h>

extern const char main_down_Tiles[];
extern const char main_down_Map[];
extern const char main_down_Pal[];

const PA_BgStruct main_down = {
	PA_BgNormal,
	256, 192,

	main_down_Tiles,
	main_down_Map,
	{main_down_Pal},

	2432,
	{1536}
};
