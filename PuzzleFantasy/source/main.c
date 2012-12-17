// Free RTOS Headers
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nds.h>
#include <stdio.h>
#include "sevencore_io.h"
#include "up.h"
#include "down.h"

static portTASK_FUNCTION(Main_Task, pvParameters);
portTASK_FUNCTION(Key_Task, pvParameters);

void InitDebug(void);

int
main(void)
{
	// 상단 화면 설정
	InitDebug();
	videoSetMode(MODE_5_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	bgInit(3,BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	decompress(upBitmap, BG_GFX, LZ77Vram);

	// 하단 화면 설정
	videoSetModeSub(MODE_5_2D);
	vramSetBankC(VRAM_C_SUB_BG);
	bgInitSub(3,BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	decompress(downBitmap, BG_GFX_SUB, LZ77Vram);

	xTaskCreate(Key_Task,
			(const signed char * const)"Key_Task",
			2048,
			(void *)NULL,
			tskIDLE_PRIORITY + 10,
			NULL);

	xTaskCreate(Main_Task,
			(const signed char * const)"Main_Task",
			2048,
			(void *)NULL,
			tskIDLE_PRIORITY + 1,
			NULL);

	KeyQueue = xQueueCreate(MAX_KEY_LOG, sizeof(u8));

	vTaskStartScheduler();		// Never returns
	while(1)
		;
	return 0;
}

void
InitDebug(void)
{
#ifdef DEBUG
	irqInit();
	initSpi();
	initDebug();
	BreakPoint();
#endif
}

void Puzzle(void);

static
portTASK_FUNCTION(Main_Task, pvParameters)
{
	while (1) {
		Puzzle();
	}
}
