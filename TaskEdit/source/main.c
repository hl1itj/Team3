#include "header.h"
#include "all_gfx.h"
#include "game.h"
#include "main_sound.h"

int main(){
	PA_Init();    // Initializes PA_Lib
	PA_InitVBL(); // Initializes a standard VBL

	AS_Init(AS_MODE_SURROUND | AS_MODE_16CH);
	AS_SetDefaultSettings(AS_PCM_8BIT, 11025, AS_SURROUND);

	PA_LoadBackground(UP_SCREEN, BACKGROUND_UP, &main_up);
	PA_LoadBackground(DOWN_SCREEN, BACKGROUND_DOWN, &main_down);

	// ���� ���.AS_SoundDefaultPlay((u8*)<�������ϸ�>, (u32)<�������ϸ�>_size, 127, 64, <���� ����>, 0);
	AS_SoundDefaultPlay((u8*)main_sound, (u32)main_sound_size, 127, 64, TRUE, 0);

	create_tasks();

	vTaskStartScheduler();		// Never returns
	while (1)
		;
	return 0;
}
