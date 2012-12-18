////////////////////////////
// PAlib project template //
////////////////////////////

// Lines starting with two slashes are ignored by the compiler
// Basically you can use them to comment what are you doing
// In fact, this kind of lines are called comments :P

// Include PAlib so that you can use it
#include <PA9.h>
#include "all_gfx.h"
// Free RTOS Headers
// See API reference Document in http://www.freertos.org/

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nds.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sevencore_io.h>

#define COLOR_RED          RGB(31,  0,  0)    /* Bright Red      */
#define COLOR_WHITE        RGB(31, 31, 31)    /* Bright White */
#define COLOR_YELLOW       RGB(31, 31,  0)    /* Bright Yellow */
#define COLOR_BLACK        RGB( 0,  0,  0)    /* Black : Zero    */
#define COLOR_GRAY         RGB(16, 16, 16)    /* Bright Gray */
#define COLOR_GREEN        RGB( 0, 31,  0)    /* Bright Green */
#define COLOR_BLUE         RGB( 0,  0, 31)    /* Bright Blue */
#define COLOR_PURPLE       RGB(16,  0, 16)    /* Bright Purple */
#define COLOR_SELECT       RGB(31,  0, 31)    /* Bright Magenta */

#define BLOCK_WIDTH			32
#define BLOCK_HEIGHT		32

#define SP             2
#define N_PUZZLE       6
#define N_BLOCK        5
#define N_BLOCK_SPRITE	10
#define PUZZLE_X(x)    ((x) % N_PUZZLE)
#define PUZZLE_Y(x)    ((x) / N_PUZZLE)
#define SCREEN_X(x)    (((x) % N_PUZZLE) + SP)
#define SCREEN_Y(x)    ((x) / N_PUZZLE)

#define UP_SCREEN 1
#define DOWN_SCREEN 0

#define BACKGROUND_UP		1
#define BACKGROUND_DOWN		0

#define MAX_LEVEL			10

// global variable
typedef struct game_info {
	u8 block_count[N_BLOCK];
	u8 turn;
	u8 level;
} game_info;

typedef struct user_info {
	int hp;
	int mp;
	int atk;
	int def;
	void (*action) (int);
} user_info;

typedef struct block {
	u8 color;
	u8 flag;
} block;

xQueueHandle KeyQueue;
volatile u8 virtual_puzzle;
game_info g_info;
user_info u_info;
user_info monster[MAX_LEVEL];
block puzzle[N_PUZZLE][N_PUZZLE];
enum {RED = 0, BLUE, GREEN, YELLOW, PURPLE};
enum {HP = 0, MP, DEF, ATK, SPC};

void draw_block(int pos_x, int pos_y, u16 color)
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

void select_block(int pos_x, int pos_y, u16 color)
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

u16 set_color(u8 n) {
	switch (n) {
	case RED:
		return COLOR_RED;
	case BLUE:
		return COLOR_BLUE;
	case GREEN:
		return COLOR_GREEN;
	case YELLOW:
		return COLOR_YELLOW;
	case PURPLE:
		return COLOR_PURPLE;
	}
}

void write_puzzle(u8 value)
{
	virtual_puzzle = value & 0x3F;
}

u8 read_puzzle(void)
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

// 지금 선택한 block이 바로 전에 선택한 block의 상하좌우에 위치해 있는지 검사
u8 is_switching_position(u8 old_key, u8 key)
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

// 바로 전에 선택한 block과 지금 선택한 block의 색을 바꾼다
void switching_color(u8 old_key, u8 key)
{
	u8 old_x, old_y;
	u8 x, y;
	u8 temp;

	old_x = PUZZLE_X(old_key); old_y = PUZZLE_Y(old_key);
	x = PUZZLE_X(key); y = PUZZLE_Y(key);

	temp = puzzle[old_x][old_y].color;
	puzzle[old_x][old_y].color = puzzle[x][y].color;
	puzzle[x][y].color = temp;

	draw_block(SCREEN_X(old_key), SCREEN_Y(old_key), set_color(puzzle[old_x][old_y].color));
	draw_block(SCREEN_X(key), SCREEN_Y(key), set_color(puzzle[x][y].color));
}

// 해당 행에 연속된 색이 있는지 검사
u8 check_row(int row)
{
	u8 changed = FALSE;
	u8 color;
	int i, match;

	match = 0;
	color = puzzle[row][0].color;

	for(i = 1; i < N_PUZZLE; i++) {
		if (color == puzzle[row][i].color) {
			match++;

			if (match == 2) {
				changed = TRUE;
				puzzle[row][i-2].flag = TRUE;
				puzzle[row][i-1].flag = TRUE;
				puzzle[row][i].flag = TRUE;
				g_info.block_count[color] += 3;
			}
			else if (match >= 3) {
				puzzle[row][i].flag = TRUE;
				g_info.block_count[color]++;
			}
		}
		else {
			// match 할 수 있는 block이 2개이상 이라면
			if (i < (N_PUZZLE-2)) {
				match = 0;
				color = puzzle[row][i].color;
			}
			else
				break;
		}
	}
	return changed;
}

// 해당 열에 연속된 색이 있는지 검사
u8 check_col(int col)
{
	u8 changed = FALSE;
	u8 color;
	int i, match;

	match = 0;
	color = puzzle[0][col].color;

	for(i = 1; i < N_PUZZLE; i++) {
		if (color == puzzle[i][col].color) {
			match++;

			if (match == 2) {
				changed = TRUE;
				puzzle[i-2][col].flag = TRUE;
				puzzle[i-1][col].flag = TRUE;
				puzzle[i][col].flag = TRUE;
				g_info.block_count[color] += 3;
			}
			else if (match >= 3) {
				puzzle[i][col].flag = TRUE;
				g_info.block_count[color]++;
			}
		}
		else {
			// match 할 수 있는 block이 2개이상 이라면
			if (i < (N_PUZZLE-2)) {
				match = 0;
				color = puzzle[i][col].color;
			}
			else
				break;
		}
	}
	return changed;
}

// action값에 따라 유저가 행동함
void user_action(int action)
{
	int damage;

	switch (action) {
	case HP:
		u_info.hp += (g_info.block_count[action] * 10);
		break;

	case MP:
		u_info.mp += (g_info.block_count[action] * 5);
		break;

	case DEF:
		u_info.def += (g_info.block_count[action] * 5);
		break;

	case ATK:
		u_info.atk += (g_info.block_count[action] * 10);
		damage = u_info.atk - monster[g_info.level].def;
		if (damage > 0)
			monster[g_info.level].hp -= (damage);
		break;

	case SPC:
		damage = g_info.block_count[action] * 10;
		damage *= (g_info.block_count[action] - 1);
		damage -= monster[g_info.level].def;
		if (damage > 0)
			monster[g_info.level].hp -= (damage);
		break;
	}
}

void next_level(void)
{
	if (g_info.level < MAX_LEVEL)
		g_info.level++;
	else
		;// game clear
}

// puzzle에 상쇄 가능한 블럭이 있는지 검사하여
// 상쇄된 block 대신에 새로운 block을 생성
// block 상쇄에 따른 행동(공격, 방어 등등)
void check_puzzle(void)
{
	int i, j;
	u8 color;
	u8 old_color = -1;
	u8 changed = FALSE;

	do {
		if (changed) {
			changed = FALSE;

			// 상쇄된 block을 검은색으로 표시
			for (i = 0; i < N_PUZZLE; i++)
				for (j = 0; j < N_PUZZLE; j++)
					if (puzzle[i][j].flag)
						draw_block(i + SP, j, COLOR_BLACK);

			// 일정 시간 delay
			for (i = 0; i < 1000000; i++);

			// block 상쇄에 따른 행동
			for (i = 0; i < N_BLOCK; i++)
				if (g_info.block_count[i] > 0) {
					u_info.action(i);
					if (monster[g_info.level].hp <= 0)
						next_level();
				}

			// block count 초기화
			for (i = 0; i < N_BLOCK; i++)
				g_info.block_count[i] = 0;

			// 새로운 block 생성
			srand((int)time(NULL));
			for (i = 0; i < N_PUZZLE; i++)
				for (j = 0; j < N_PUZZLE; j++)
					if (puzzle[i][j].flag) {
						do {
							color = rand() % N_BLOCK;
						}while(color == old_color);

						draw_block(i + SP, j, set_color(color));
						puzzle[i][j].color = color;
						puzzle[i][j].flag = FALSE;
						old_color = color;
					}
		}

		for(i = 0; i < N_PUZZLE; i++) {
			changed = changed | check_row(i);
			changed = changed | check_col(i);
		}
	} while (changed);
}

// 유저 정보를 초기화
void initialize_user_info(void)
{
	u_info.atk = 0;
	u_info.def = 0;
	u_info.hp = 1000;
	u_info.mp = 500;
	u_info.action = user_action;
}

// 몬스터 정보를 초기화
void initialize_monster_info(void)
{
	int i;
	for (i = 0; i < MAX_LEVEL; i++) {
		monster[i].atk = i * 10;
		monster[i].def = i;
		monster[i].hp = (i + 1) * 200;
		monster[i].mp = 0;
	}
}

// 게임 정보를 초기화
void initialize_game_info(void)
{
	int i;
	for (i = 0; i < N_BLOCK; i++)
		g_info.block_count[i] = 0;

	g_info.level = 1;
}

// random으로 puzzle 색을 초기화
void initialize_puzzle(void)
{
	int i, j;
	u8 color;
	u8 old_color = -1;

	/*
	srand((int)time(NULL));
	for (i = 0; i < N_PUZZLE; i++)
		for (j = 0; j < N_PUZZLE; j++) {
			do {
				color = rand() % N_BLOCK;
			}while(color == old_color);

			draw_block(i + SP, j, set_color(color));
			puzzle[i][j].color = color;
			puzzle[i][j].flag = FALSE;
			old_color = color;
		}
	 */

	for (i = 0; i < N_PUZZLE; i++)
		for (j = 0; j < N_PUZZLE; j++) {
			do {
				color = PA_RandMax(N_BLOCK);
			}while(color == old_color);

			puzzle[i][j].color = color;
			puzzle[i][j].flag = FALSE;
			PA_CreateSprite(DOWN_SCREEN, color,(void*)puzzle_Sprite, OBJ_SIZE_32X32, 1, N_BLOCK_SPRITE, i*32, j*32);
			PA_SetSpriteAnim(DOWN_SCREEN, color, puzzle[i][j].color);
			old_color = color;
		}

	/*
	for ( i=0;i<SIZE;i++ ) {
		for ( j=0;j<SIZE;j++ ){
			PA_CreateSprite(DOWN_SCREEN, k,(void*)number_Sprite, OBJ_SIZE_32X32,1,NPAL, i*32, j*32);
			Blksprite[j][i] = k; // 해당 블락스프라이트정보배열 값  초기화
			PA_SetSpriteAnim(DOWN_SCREEN, k, blocks[j][i]); // 블록배열의 값을 스프라이트에 반영.
			k++;
		}
	}
	*/
}

/* key입력에 따른 처리 pusedo code */
///////////////////////////////////////////////
// getkey
// selected == false인 경우
//         key 값에 따라 puzzle 선택
// selected == true인 경우
//         key == old_key라면, 선택 해제
//        선택한 puzzle의 상하좌우 라면, 위치 바꿈
//        가능하다면 puzzle 상쇄
///////////////////////////////////////////////

void game(void)
{
	u8 old_key = -1;
	u8 key;
	u8 selected = FALSE;

	initialize_user_info();
	initialize_monster_info();
	initialize_game_info();
	initialize_puzzle();
	check_puzzle();

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
					check_puzzle();
				}
			}
		}
	}
}

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

// 4*4 key matrix 입력을 6*6 입력으로 바꿈
// 기존의 key scan하는 방식과 동일
portTASK_FUNCTION(Puzzle_Key_Task, pvParameters)
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

static
portTASK_FUNCTION(Game_Task, pvParameters)
{
	while (1) {
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

int main(){
	PA_Init();    // Initializes PA_Lib
	//	PA_InitVBL(); // Initializes a standard VBL

	AS_Init(AS_MODE_SURROUND | AS_MODE_16CH);
	AS_SetDefaultSettings(AS_PCM_8BIT, 11025, AS_SURROUND);

	PA_LoadBackground(UP_SCREEN, BACKGROUND_UP, &bground_up);
	PA_LoadBackground(DOWN_SCREEN, BACKGROUND_DOWN, &down);

	create_tasks();

	vTaskStartScheduler();		// Never returns
	while (1)
		;
	return 0;
}
