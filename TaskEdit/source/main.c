#include "header.h"
#include "all_gfx.h"
#include "game.h"

int main(){
	PA_Init();    // Initializes PA_Lib

	AS_Init(AS_MODE_SURROUND | AS_MODE_16CH);
	AS_SetDefaultSettings(AS_PCM_8BIT, 11025, AS_SURROUND);

	PA_LoadBackground(UP_SCREEN, BACKGROUND_UP, &main_up);
	PA_LoadBackground(DOWN_SCREEN, BACKGROUND_DOWN, &main_down);

	create_tasks();

	vTaskStartScheduler();		// Never returns
	while (1)
		;
	return 0;
}
