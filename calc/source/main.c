#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <nds.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "sevencore_io.h"

#include "card_spi.h"
#include "gdbStub.h"
#include "gdbStubAsm.h"

static portTASK_FUNCTION(Exp_Task, pvParameters);
portTASK_FUNCTION(Key_Task, pvParameters);

void InitDebug(void);

xQueueHandle KeyQueue;
#define MAX_KEY_LOG		10

int
main(void)
{
	InitDebug();
	init_virtual_io(ENABLE_LED | ENABLE_SW | ENABLE_MATRIX);	// Enable Virtual LED's on Top Screen
	//init_printf();							// Initialize Bottom Screen for printf()

	xTaskCreate(Key_Task,
					     (const signed char * const)"Key_Task",
					     2048,
					     (void *)NULL,
					     tskIDLE_PRIORITY + 1,
					     NULL);

	KeyQueue = xQueueCreate(MAX_KEY_LOG, sizeof(u8));
	// Error Processing Needed !

	xTaskCreate(Exp_Task,
						 (const signed char * const)"Exp_Task",
						 2048,
						 (void *)NULL,
						 tskIDLE_PRIORITY + 1,
						 NULL);

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

void Exp_Homework_A(void);

static
portTASK_FUNCTION(Exp_Task, pvParameters)
{
	while (1) {
		Exp_Homework_A();
	}
}
