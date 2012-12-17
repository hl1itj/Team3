// Free RTOS Headers
// See API reference Document in http://www.freertos.org/

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nds.h>
#include <sevencore_io.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define COLOR_RED       RGB(31,  0,  0) /* Bright Red  	*/
#define COLOR_WHITE     RGB(31, 31, 31) /* Bright White */
#define COLOR_YELLOW     RGB(31, 31, 0) /* Bright White */
#define COLOR_BLACK     RGB( 0,  0,  0)	/* Black : Zero	*/
#define COLOR_GRAY     RGB(16, 16, 16)
#define COLOR_GREEN     RGB( 0, 31,  0) /* Bright Green */
#define COLOR_BLUE      RGB( 0,  0, 31) /* Bright White */
#define COLOR_PURPLE     RGB(16, 0, 16) /* Bright White */
#define COLOR_SELECT     RGB(31, 0, 31) /* Bright White */

#define BLOCK_WIDTH		32
#define BLOCK_HEIGHT		32

#define SP				2
#define N_PUZZLE		6
#define N_BLOCK		5
#define PUZZLE_X(x)	((x) % N_PUZZLE)
#define PUZZLE_Y(x)	((x) / N_PUZZLE)
#define SCREEN_X(x)	(((x) % N_PUZZLE) + SP)
#define SCREEN_Y(x)	((x) / N_PUZZLE)

// global variable
//xSemaphoreHandle xSemaphore[BLOCK_X_MAX - 1];
volatile u8 virtual_puzzle;
u8 puzzle[N_PUZZLE][N_PUZZLE];

void
draw_block(int pos_x, int pos_y, u16 color)
{
	// draw big white box
	int i, j;
	u32 *basePoint, pixel, blank_pixel;

	pixel = (color << 16) + color;
	blank_pixel = (COLOR_GRAY << 16) + COLOR_GRAY;
	for (i = 0; i < BLOCK_HEIGHT; i++) {

		basePoint = (u32 *) BG_GFX_SUB+
				((((pos_y * BLOCK_HEIGHT) + i) * SCREEN_WIDTH) +
						pos_x * BLOCK_WIDTH) / 2;

		for (j = 0; j < (BLOCK_WIDTH / 2); j++) {
			if ((i < 2) || (i > (BLOCK_HEIGHT-3))
					|| (j == 0) || (j == (BLOCK_HEIGHT/2-1))) {
				*basePoint++ = blank_pixel;
				continue;
			}
			*basePoint++ = pixel;
		}
	}
}

void
select_block(int pos_x, int pos_y, u16 color)
{
	// draw big white box
	int i, j;
	u32 *basePoint, pixel;

	pixel = (color << 16) + color;

	for (i = 0; i < BLOCK_HEIGHT; i++) {

		basePoint = (u32 *) BG_GFX_SUB+
				((((pos_y * BLOCK_HEIGHT) + i) * SCREEN_WIDTH) +
						pos_x * BLOCK_WIDTH) / 2;

		for (j = 0; j < (BLOCK_WIDTH / 2); j++) {
			if ((i < 2) || (i > (BLOCK_HEIGHT-3))
					|| (j == 0) || (j == (BLOCK_HEIGHT/2-1))) {
				*basePoint++ = pixel;
				continue;
			}
			*basePoint++;
		}
	}
}

u16
set_color(u8 n) {
	switch (n) {
	case 0:
		return COLOR_YELLOW;
	case 1:
		return COLOR_RED;
	case 2:
		return COLOR_GREEN;
	case 3:
		return COLOR_BLUE;
	case 4:
		return COLOR_PURPLE;
	}
}

void
write_puzzle(u8 value)
{
	virtual_puzzle = value & 0x3F;
}

u8
read_puzzle(void)
{
	touchPosition touch;

	if (!(virtual_puzzle & 0x3F))
		return 0;
	touchRead(&touch);
	touch.py = (touch.py / BLOCK_HEIGHT);
	touch.px = (touch.px / BLOCK_WIDTH) - SP;
	if ((touch.py < 0) || (touch.py > 5) || (touch.px < 0) || (touch.px > 5))
		return 0;
	if (!(virtual_puzzle & (0x20 >> touch.py))) return 0;
	return (0x20 >> touch.px);
}

u8
is_switching_position(u8 old_key, u8 key)
{
	u8 old_x, old_y;
	u8 x, y;
	old_x = PUZZLE_X(old_key); old_y = PUZZLE_Y(old_key);
	x = PUZZLE_X(key); y = PUZZLE_Y(key);

	if (old_x > 0) {
		if (((old_x-1) == x) && (old_y == y))
			return TRUE;
	}

	if (old_x < 5) {
		if (((old_x+1) == x) && (old_y == y))
			return TRUE;
	}

	if (old_y > 0) {
		if (((old_y-1) == y) && (old_x == x))
			return TRUE;
	}

	if (old_y < 5) {
		if (((old_y+1) == y) && (old_x == x))
			return TRUE;
	}

	return FALSE;
}

void
switching_color(u8 old_key, u8 key)
{
	u8 old_x, old_y;
	u8 x, y;
	u8 temp;

	old_x = PUZZLE_X(old_key); old_y = PUZZLE_Y(old_key);
	x = PUZZLE_X(key); y = PUZZLE_Y(key);

	temp = puzzle[old_x][old_y];
	puzzle[old_x][old_y] = puzzle[x][y];
	puzzle[x][y] = temp;

	draw_block(SCREEN_X(old_key), SCREEN_Y(old_key), set_color(puzzle[old_x][old_y]));
	draw_block(SCREEN_X(key), SCREEN_Y(key), set_color(puzzle[x][y]));
}

void
cancel_out_block()
{

}

void
Game(void)
{
	int i, j;
	u8 old_key = -1;
	u8 key;
	u8 selected = FALSE;
	u8 color;

	// random으로 puzzle 초기화
	srand((int)time(NULL));
	for (i = 0; i < N_PUZZLE; i++)
		for (j = 0; j < N_PUZZLE; j++) {
			color = rand() % N_BLOCK;
			draw_block(i + SP, j, set_color(color));
			puzzle[i][j] = color;
		}
	// BLOCK 상쇄 되는지 확인


	/* key입력에 따른 처리 pusedo code */
	///////////////////////////////////////////////
	// getkey
	// selected == false인 경우
	// 		key 값에 따라 puzzle 선택
	// selected == true인 경우
	// 		key == old_key라면, 선택 해제
	//		선택한 puzzle의 상하좌우 라면, 위치 바꿈
	//		가능하다면 puzzle 상쇄
	///////////////////////////////////////////////
	while (1) {
		key = getkey();

		if (key <= 36) {
			printf("%d ", key);
			key--;

			if (!selected) {
				selected = TRUE;
				if (old_key != -1)
					select_block(SCREEN_X(old_key), SCREEN_Y(old_key), COLOR_GRAY);
				select_block(SCREEN_X(key), SCREEN_Y(key), COLOR_SELECT);
				old_key = key;
			}
			else {
				if (key == old_key) {
					selected = FALSE;
					select_block(SCREEN_X(old_key), SCREEN_Y(old_key), COLOR_GRAY);
				}
				else if (is_switching_position(old_key, key)) {
					selected = FALSE;
					select_block(SCREEN_X(old_key), SCREEN_Y(old_key), COLOR_GRAY);
					switching_color(old_key, key);
					// BLOCK 상쇄 되는지 확인
					cancel_out_block();
					// 적용가능한 행동이 있다면 행동함
				}
			}
		}
	}
}

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
	return key;
}

// 4*4 key matrix 입력을 6*6 입력으로 바꿈
// 기존의 key scan하는 방식과 동일
portTASK_FUNCTION(Key_Task, pvParameters)
{
	// Variables
	u8 key, scan = 0;
	u8 key_pressed = FALSE;
	u8 pre_line;

	while (1) {

		if (!key_pressed) {

			write_puzzle(0x20 >> scan);
			key = scan * 6;

			pre_line = read_puzzle();
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

		if (key_pressed
				&& ((read_puzzle() == 0)
						|| (read_puzzle() != pre_line)))
			key_pressed = FALSE;

		vTaskDelay(MSEC2TICK(25));
	}
}
