#include <PA_BgStruct.h>

extern const char main_up_Tiles[];
extern const char main_up_Map[];
extern const char main_up_Pal[];

const PA_BgStruct main_up = {
	PA_BgNormal,
	256, 192,

	main_up_Tiles,
	main_up_Map,
	{main_up_Pal},

	15552,
	{1536}
};
