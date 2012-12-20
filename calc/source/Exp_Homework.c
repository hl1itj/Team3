#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <nds.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "sevencore_io.h"

extern xQueueHandle KeyQueue;
#define MAX_KEY_LOG        	10
#define KEY_0               0
#define MIN_LED             0x01
#define MAX_LED             0x80
#define LED_DELAY				500
#define SEG7LED_ON(x)     	writeb_virtual_io(SEG7LED, x)
#define LED1(x)             writeb_virtual_io(BARLED1, x)

enum {ADD = 10, MUL, EQ, CE};

void
key_init(void)
{
	int i;
	u8 key;

	for (i = 0; i < MAX_KEY_LOG; i++)
		xQueueReceive(KeyQueue, &key, 0);
}

int
kbhit(void)
{
	u8 key;
	int ret = xQueuePeek(KeyQueue, &key, 0);
	return (ret == pdPASS);
}
u8
getkey(void)
{
	u8 key;
	xQueueReceive(KeyQueue, &key, portMAX_DELAY);
	//while (pdPASS != xQueueReceive(KeyQueue, &key, 0))
	//        vTaskDelay(MSEC2TICK(5));
	return key;
}

void
Exp_Homework_A(void)
{
	int i;
	int res = 0;
	int tmp;
	u8 key_arr[NUM_7SEG_LED];
	u8 key;
	u8 operator = ADD;
	u8 flag;

	for (i = 0; i < NUM_7SEG_LED; i++)    // Turn Off All
		SEG7LED_ON(0x80 + (i << 4));

	key_init();

	while(1) {

		key = getkey();

		if (key < 10) {
			if (operator == ADD)
				res += key;
			else
				res *= key;
		}
		else if (key == ADD) {
			operator = ADD;
		}
		else if (key == MUL) {
			operator = MUL;
		}
		else if (key == EQ) {
			for (i = 0; i < NUM_7SEG_LED; i++)    // Turn Off All
				SEG7LED_ON(0x80 + (i << 4));

			tmp = res;
			for (i = 0; i < NUM_7SEG_LED; i++) {
				key_arr[i] = tmp % 10;
				tmp -= key_arr[i];
				tmp /= 10;
			}

			flag = FALSE;
			for (i = NUM_7SEG_LED - 1; i >= 0; i--) {
				if (key_arr[i] > 0)
					flag = TRUE;

				if (flag)
					SEG7LED_ON((0x70 - (i << 4)) + key_arr[i]);
			}
		}
		else if (key == CE) {
			res = 0;
			for (i = 0; i < NUM_7SEG_LED; i++)    // Turn Off All
				SEG7LED_ON(0x80 + (i << 4));
		}
	}
}

portTASK_FUNCTION(Key_Task, pvParameters)
{
	// Variables
	u8 key, scan = 0;
	u8 key_pressed = FALSE;
	u8 pre_line;

	while (1) {
		if (!key_pressed) {
			writeb_virtual_io(KEY_MATRIX, 0x80 >> scan);
			key = scan * 4;

			pre_line = readb_virtual_io(KEY_MATRIX);
			switch (readb_virtual_io(KEY_MATRIX)) {
			case 8 : key += 1; break;
			case 4 : key += 2; break;
			case 2 : key += 3; break;
			case 1 : key += 4; if (key == 16) key = 0; break;
			default : key = 255; break;
			}

			scan++;
			if (scan == 4)
				scan = 0;

			if (key < 14) {
				key_pressed = TRUE;
				xQueueSend(KeyQueue, &key, 0);
			}
		}

		if (key_pressed
				&& ((readb_virtual_io(KEY_MATRIX) == 0)
						|| (readb_virtual_io(KEY_MATRIX) != pre_line)))
			key_pressed = FALSE;
		vTaskDelay(MSEC2TICK(25));
	}
}
