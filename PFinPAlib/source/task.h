/*
 * task.h
 *
 *  Created on: 2012. 12. 19.
 *      Author: ksi
 */

#ifndef TASK_H_
#define TASK_H_

#include "header.h"

xQueueHandle KeyQueue;
xTaskHandle gameTask;

void key_init(void);
int kbhit(void);
u8 getkey(void);
portTASK_FUNCTION(Puzzle_Key_Task, pvParameters);
portTASK_FUNCTION(Game_Task, pvParameters);
void create_tasks(void);

#endif /* TASK_H_ */
