/*
 * header.h
 *
 *  Created on: 2012. 12. 19.
 *      Author: ksi
 */

#ifndef GAME_H_
#define GAME_H_

#include "header.h"

#define BLOCK_WIDTH			32
#define BLOCK_HEIGHT		32

#define SP			   2
#define N_PUZZLE       6
#define N_BLOCK        5

#define PUZZLE_X(x)    ((x) % N_PUZZLE)
#define PUZZLE_Y(x)    ((x) / N_PUZZLE)

#define PUZZLE		0
#define PUZZLE_PAL	0
#define BOMB		1
#define BOMB_PAL	1
#define MONSTER		2
#define MONSTER_PA	2

#define DOWN_SCREEN 0
#define UP_SCREEN 1

#define BACKGROUND_DOWN		0
#define BACKGROUND_UP		1

#define MAX_LEVEL			16

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
	int spc;
	int max_hp;
	int max_mp;
	void (*action) (int);

	void *img_pal;
	void *img_sprite;
} user_info;

typedef struct block {
	u8 id;
	u8 color;
	u8 bomb;
} block;

xQueueHandle KeyQueue;
volatile u8 virtual_puzzle;

game_info g_info;
user_info u_info;
user_info monster[MAX_LEVEL + 1];
block puzzle[N_PUZZLE][N_PUZZLE];
enum {HP = 0, MP, DEF, ATK, SPC};


void write_puzzle(u8 value);
u8 read_puzzle(void);
void draw_block(u8 id, u8 color);
void draw_bomb(u8 id);
u8 is_switching_position(u8 old_key, u8 key);
void switching_color(u8 old_key, u8 key);
u8 check_row(int row);
u8 check_col(int col);
void user_action(int action);
void next_level(void);
void check_puzzle(void);
void initialize_user_info(void);
void initialize_monster_info(void);
void initialize_game_info(void);
void initialize_puzzle(void);
void game(void);
void main_screen();
void set_monster_sprite(u8 level);
void draw_monster(u8 level);

#endif
