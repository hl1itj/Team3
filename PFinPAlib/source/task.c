/*
 * task.c
 *
 *  Created on: 2012. 12. 19.
 *      Author: ksi
 */

#include "task.h"
#include "game.h"

void key_init(void)
{
	int i;
	u8 key;

	for (i = 0; i < MAX_KEY_LOG; i++)
		xQueueReceive(KeyQueue, &key, 0);
}

int kbhit(void)
{
	u8 key;
	int ret = xQueuePeek(KeyQueue, &key, 0);
	return (ret == pdPASS);
}

u8 getkey(void)
{
	u8 key;
	xQueueReceive(KeyQueue, &key, portMAX_DELAY);
	return key;
}

// 4*4 key matrix �Է��� 6*6 �Է����� �ٲ�
// ������ key scan�ϴ� ��İ� ����
portTASK_FUNCTION(Puzzle_Key_Task, pvParameters)
{
	// Variables
	u8 key, scan = 0;
	u8 key_pressed = FALSE;

	while (1) {
		if (!key_pressed) {

			write_puzzle(0x20 >> scan);
			key = scan * 6;

			switch (read_puzzle()) {
			case 32 : key += 1; break;
			case 16 : key += 2; break;
			case 8 : key += 3; break;
			case 4 : key += 4; break;
			case 2 : key += 5; break;
			case 1 : key += 6; break;
			default : key = 255; break;
			}

			scan++;
			if (scan == 6)
				scan = 0;

			if (key <= 36) {
				key_pressed = TRUE;
				xQueueSend(KeyQueue, &key, 0);
			}
		}

		if (key_pressed && (read_puzzle() == 0))
			key_pressed = FALSE;

		vTaskDelay(MSEC2TICK(25));
	}
}

portTASK_FUNCTION(Game_Task, pvParameters)
{
	while (1) {
		main_screen();
		game();
	}
}

void create_tasks(void)
{
	xTaskCreate(Puzzle_Key_Task,
			(const signed char * const)"Puzzle_Key_Task",
			2048,
			(void *)NULL,
			tskIDLE_PRIORITY + 10,
			NULL);

	xTaskCreate(Game_Task,
			(const signed char * const)"Game_Task",
			2048,
			(void *)NULL,
			tskIDLE_PRIORITY + 1,
			NULL);

	KeyQueue = xQueueCreate(MAX_KEY_LOG, sizeof(u8));
}
