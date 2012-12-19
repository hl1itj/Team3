/*
 * game.c
 *
 *  Created on: 2012. 12. 19.
 *      Author: ksi
 */

#include "game.h"
#include "task.h"

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
	touch.py = touch.py / BLOCK_HEIGHT;
	touch.px = (touch.px / BLOCK_WIDTH) - SP;
	if ((touch.py < 0) || (touch.py > 5) || (touch.px < 0) || (touch.px > 5))
		return 0;
	if (!(virtual_puzzle & (0x20 >> touch.py))) return 0;
	return (0x20 >> touch.px);
}

void draw_block(u8 id, u8 color)
{
	PA_SetSpriteAnim(DOWN_SCREEN, id, color + 4);
}

void draw_monster(u8 level)
{
	PA_SetSpriteAnim(UP_SCREEN, 0, 0);
}

void draw_damaged_monster(void)
{
	PA_StartSpriteAnimEx(UP_SCREEN, 0, 0, 1, 10, ANIM_LOOP, 3);
	delay(500);
}

void draw_monster_attack(void)
{
	PA_LoadSpritePal(UP_SCREEN, HIT, (void *) hit_Pal);
	PA_CreateSprite(UP_SCREEN, 1, (void *) hit_Sprite,
				OBJ_SIZE_32X32, TRUE, HIT, 118, 130);

	PA_StartSpriteAnimEx(UP_SCREEN, 1, 0, 3, 10, ANIM_LOOP, 1);
	delay(500);
	PA_DeleteSprite(UP_SCREEN, 1);
}

void draw_text(void)
{
	PA_InitText(UP_SCREEN, 0);

	// 게임 정보
	PA_SetTextTileCol(UP_SCREEN, TEXT_RED);
	PA_OutputText(UP_SCREEN, 1, 1, "LEVEL %d", g_info.level);
	PA_OutputText(UP_SCREEN, 11, 1, "TURN %d", g_info.turn_count);

	if (g_info.turn == USER_TURN)
		PA_OutputText(UP_SCREEN, 23, 1, "USR TURN");
	else
		PA_OutputText(UP_SCREEN, 23, 1, "MON TURN");

	// 몬스터 정보
	PA_OutputText(UP_SCREEN, 13, 5, "HP %d", monster[g_info.level].hp);

	// 유저 정보
	PA_SetTextTileCol(UP_SCREEN, TEXT_RED);
	PA_OutputText(UP_SCREEN, 1, 22, "HP %d", u_info.hp);

	PA_SetTextTileCol(UP_SCREEN, TEXT_BLUE);
	PA_OutputText(UP_SCREEN, 13, 22, "MP %d", u_info.mp);

	PA_SetTextTileCol(UP_SCREEN, TEXT_GREEN);
	PA_OutputText(UP_SCREEN, 23, 22, "DEF %d", u_info.def);
}

void delay(int time)
{
	int delay;
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	do {
		delay = xTaskGetTickCount() - xLastWakeTime;
	} while (delay < ((int)(time / portTICK_RATE_MS)));
}

void set_monster_sprite(u8 level)
{
	if (level > 1)
		PA_DeleteSprite(UP_SCREEN, 0);
	PA_LoadSpritePal(UP_SCREEN, MONSTER, monster[level].img_pal);
	PA_CreateSprite(UP_SCREEN, 0, monster[level].img_sprite,
			OBJ_SIZE_64X64, TRUE, MONSTER, 96, 64);
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
				puzzle[row][i-2].bomb = TRUE;
				puzzle[row][i-1].bomb = TRUE;
				puzzle[row][i].bomb = TRUE;
				g_info.block_count[color] += 3;
			}
			else if (match >= 3) {
				puzzle[row][i].bomb = TRUE;
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
				puzzle[i-2][col].bomb = TRUE;
				puzzle[i-1][col].bomb = TRUE;
				puzzle[i][col].bomb = TRUE;
				g_info.block_count[color] += 3;
			}
			else if (match >= 3) {
				puzzle[i][col].bomb = TRUE;
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
	int damage = 0;
	u8 count = g_info.block_count[action];

	switch (action) {
	case HP:
		u_info.hp += (count * 10);
		if (u_info.hp > u_info.max_hp)
			u_info.hp = u_info.max_hp;
		break;

	case MP:
		u_info.mp += (count * 5);
		if (u_info.mp > u_info.max_mp)
			u_info.mp = u_info.max_mp;
		break;

	case DEF:
		u_info.def += (count * 5);
		break;

	case ATK:
		u_info.atk += (count * 10);
		damage = u_info.atk - monster[g_info.level].def;
		if (damage > 0) {
			monster[g_info.level].hp -= (damage);
			draw_damaged_monster();
		}
		u_info.atk = 0;
		break;

	case SPC:
		if (u_info.mp >= 100) {
			u_info.mp -= 100;
			damage = g_info.block_count[action] * 100;
			u_info.spc += damage;
			damage -= monster[g_info.level].def;
			if (damage > 0) {
				monster[g_info.level].hp -= (damage);
				draw_damaged_monster();
			}
			u_info.spc = 0;
		}
		break;
	}
}

void monster_action(void)
{
	int damage;
	damage = monster[g_info.level].atk - u_info.def;

	if (damage > 0) {
	u_info.hp -= damage;
	draw_text();
	draw_monster_attack();

	if (u_info.hp <= 0)
		game_over(); // game over
	}
}

void next_level(void)
{
	int i;

	if (g_info.level < MAX_LEVEL) {
		g_info.level++;
		g_info.turn_count = 0;

		// block count 초기화
		for (i = 0; i < N_BLOCK; i++)
			g_info.block_count[i] = 0;

		// user 정보 초기화
		initialize_user_info();

		// 해당 level 몬스터 이미지 표시
		set_monster_sprite(g_info.level);
		draw_monster(g_info.level);

		// text 표시
		draw_text();
	}
	else
		game_clear();// game clear
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

			// 폭발 이미지
			for (i = 0; i < N_PUZZLE; i++)
				for (j = 0; j < N_PUZZLE; j++)
					if (puzzle[i][j].bomb)
						PA_StartSpriteAnimEx(DOWN_SCREEN, puzzle[i][j].id, 0, 3, 10, ANIM_LOOP, 1);

			// delay 0.5 sec
			delay(500);

			// 새로운 block 생성
			for (i = 0; i < N_PUZZLE; i++)
				for (j = 0; j < N_PUZZLE; j++)
					// key값과 id값을 맞추기 위해 (j, i)로 사용
					if (puzzle[j][i].bomb) {
						old_color = color;
						do {
							color = PA_RandMax(N_BLOCK - 1);
						}while(color == old_color);

						puzzle[j][i].color = color;
						puzzle[j][i].bomb = FALSE;

						draw_block(puzzle[j][i].id, puzzle[j][i].color);
						old_color = color;
					}

			// block 상쇄에 따른 행동
			for (i = 0; i < N_BLOCK; i++)
				if (g_info.block_count[i] > 0) {
					u_info.action(i);
					draw_text();
					if (monster[g_info.level].hp <= 0) {
						next_level();
						//return;
					}
				}

			// block count 초기화
			for (i = 0; i < N_BLOCK; i++)
				g_info.block_count[i] = 0;
		}

		for(i = 0; i < N_PUZZLE; i++) {
			changed = changed | check_row(i);
			changed = changed | check_col(i);
		}
	} while (changed);
}

// 유저 정보 초기화
void initialize_user_info(void)
{
	u_info.atk = 0;
	u_info.def = 0;
	u_info.max_hp = u_info.hp = 1000 + 500 * (g_info.level - 1);
	//u_info.max_hp = u_info.hp = 10;
	if (g_info.level == 1)
		u_info.mp = 500;
	u_info.max_mp = 500 + 100 * (g_info.level - 1);
	u_info.action = user_action;
	u_info.spc = 0;
}

// 몬스터 정보를 초기화
void initialize_monster_info(void)
{
	int i;
	for (i = 1; i <= MAX_LEVEL; i++) {
		monster[i].atk = i * 50;
		monster[i].def = 0;
		//monster[i].hp = i * 200;
		monster[i].hp = i;
		monster[i].mp = 0;
		monster[i].hit = monster_action;
	}

	monster[1].img_pal = _1_Pal;
	monster[1].img_sprite = _1_Sprite;

	monster[2].img_pal = _2_Pal;
	monster[2].img_sprite = _2_Sprite;

	monster[3].img_pal = _3_Pal;
	monster[3].img_sprite = _3_Sprite;

	monster[4].img_pal = _4_Pal;
	monster[4].img_sprite = _4_Sprite;

	monster[5].img_pal = _5_Pal;
	monster[5].img_sprite = _5_Sprite;

	monster[6].img_pal = _6_Pal;
	monster[6].img_sprite = _6_Sprite;

	monster[7].img_pal = _7_Pal;
	monster[7].img_sprite = _7_Sprite;

	monster[8].img_pal = _8_Pal;
	monster[8].img_sprite = _8_Sprite;

	monster[9].img_pal = _9_Pal;
	monster[9].img_sprite = _9_Sprite;

	monster[10].img_pal = _10_Pal;
	monster[10].img_sprite = _10_Sprite;

	monster[11].img_pal = _11_Pal;
	monster[11].img_sprite = _11_Sprite;

	monster[12].img_pal = _12_Pal;
	monster[12].img_sprite = _12_Sprite;

	monster[13].img_pal = _13_Pal;
	monster[13].img_sprite = _13_Sprite;

	monster[14].img_pal = _14_Pal;
	monster[14].img_sprite = _14_Sprite;

	monster[15].img_pal = _15_Pal;
	monster[15].img_sprite = _15_Sprite;

	monster[16].img_pal = _16_Pal;
	monster[16].img_sprite = _16_Sprite;

	set_monster_sprite(g_info.level);
	draw_monster(g_info.level);
}

// 게임 정보를 초기화
void initialize_game_info(void)
{
	int i;
	for (i = 0; i < N_BLOCK; i++)
		g_info.block_count[i] = 0;

	g_info.level = 1;
	g_info.turn = USER_TURN;
	g_info.turn_count = 0;
}

// random으로 puzzle 색을 초기화
void initialize_puzzle(void)
{
	int i, j;
	u8 id = 0;
	u8 color;
	u8 old_color = -1;

	PA_LoadSpritePal(DOWN_SCREEN, PUZZLE, (void*) puzzle_Pal);

	for (i = 0; i < N_PUZZLE; i++)
		for (j = 0; j < N_PUZZLE; j++) {
			do {
				color = PA_RandMax(N_BLOCK - 1);
			}while(color == old_color);

			// key값과 id값을 맞추기 위해 (j, i)로 사용
			puzzle[j][i].id = id;
			puzzle[j][i].color = color;
			puzzle[j][i].bomb = FALSE;

			PA_CreateSprite(DOWN_SCREEN, id, (void*)puzzle_Sprite,
					OBJ_SIZE_32X32, TRUE, PUZZLE, j*32 + SP*32, i*32);
			draw_block(id, puzzle[j][i].color);
			id++;
			old_color = color;
		}
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

void user_turn(void)
{
	u8 old_key = 0;
	u8 key;
	u8 selected = FALSE;
	int delay;
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	g_info.turn = USER_TURN;
	g_info.turn_count++;
	draw_text();

	while(1) {
		if (kbhit()) {
			key = getkey();

			if (key <= 36) {
				key--;

				if (!selected) {
					selected = TRUE;
					draw_block(old_key, puzzle[PUZZLE_X(old_key)][PUZZLE_Y(old_key)].color);
					draw_block(key, puzzle[PUZZLE_X(key)][PUZZLE_Y(key)].color + N_BLOCK);
					old_key = key;
				}
				else {
					if (key == old_key) {
						selected = FALSE;
						draw_block(old_key, puzzle[PUZZLE_X(old_key)][PUZZLE_Y(old_key)].color);
					}
					else if (is_switching_position(old_key, key)) {
						selected = FALSE;
						switching_color(old_key, key);
						draw_block(old_key, puzzle[PUZZLE_X(old_key)][PUZZLE_Y(old_key)].color);
						draw_block(key, puzzle[PUZZLE_X(key)][PUZZLE_Y(key)].color);
						check_puzzle();
					}
				}
			}
		}

		delay = xTaskGetTickCount() - xLastWakeTime;
		if (delay > 10000) break;
	}
}

void monster_turn(void)
{
	g_info.turn = MONSTER_TURN;
	monster[g_info.level].hit();
}

void game(void)
{
	// initialize 순서를 바꾸면 제대로된 게임 실행이 안될 수 있음!
	initialize_game_info();
	initialize_user_info();
	initialize_monster_info();
	initialize_puzzle();
	draw_text();
	check_puzzle();

	while (1) {
		user_turn();
		monster_turn();
	}
}

void main_screen(void)
{
	while (1) {
		if (Stylus.Newpress || Pad.Newpress.Start) {
			PA_DeleteBg(UP_SCREEN, BACKGROUND_UP);
			PA_DeleteBg(DOWN_SCREEN, BACKGROUND_DOWN);
			PA_LoadBackground(UP_SCREEN, BACKGROUND_UP, &bground_up);
			PA_LoadBackground(DOWN_SCREEN, BACKGROUND_DOWN, &bground_down);
			break;
		}

		PA_WaitForVBL();
	}
}

void game_over(void)
{
	PA_Init();
	PA_LoadBackground(UP_SCREEN, BACKGROUND_UP, &gameover);
	PA_LoadBackground(DOWN_SCREEN, BACKGROUND_DOWN, &gameover);
	vTaskSuspend(gameTask);
}

void game_clear(void)
{
	PA_Init();
	PA_LoadBackground(UP_SCREEN, BACKGROUND_UP, &gameclear);
	PA_LoadBackground(DOWN_SCREEN, BACKGROUND_DOWN, &gameclear);
	vTaskSuspend(gameTask);
}
