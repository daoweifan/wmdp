/*
 * File      : shell.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-06-02     Bernard      Add finsh_get_prompt function declaration
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include "wmconfig.h"
#include "finsh.h"
#include "device.h"
#include "def.h"


#define FINSH_USING_HISTORY
#ifndef FINSH_THREAD_PRIORITY
#define FINSH_THREAD_PRIORITY 20
#endif
#ifndef CONFIG_FINSH_TASK_STACK_SIZE
#define CONFIG_FINSH_TASK_STACK_SIZE 2048
#endif
#define FINSH_CMD_SIZE		80

#define FINSH_OPTION_ECHO	0x01

#define FINSH_PROMPT		"finsh>>"

#ifdef FINSH_USING_HISTORY
enum input_stat
{
	WAIT_NORMAL,
	WAIT_SPEC_KEY,
	WAIT_FUNC_KEY,
};
	#ifndef FINSH_HISTORY_LINES
		#define FINSH_HISTORY_LINES 5
	#endif
#endif

struct finsh_shell
{
	// xSemaphoreHandle rx_sem;

	enum input_stat stat;

	uint8_t echo_mode:1;
	uint8_t use_history:1;

#ifdef FINSH_USING_HISTORY
	uint8_t current_history;
	uint16_t history_count;

	char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];
#endif

	struct finsh_parser parser;

	char line[FINSH_CMD_SIZE];
	uint8_t line_position;

	device_t device;
};

void finsh_set_echo(uint32_t echo);
uint32_t finsh_get_echo(void);

void finsh_set_device(const char* device_name);
const char* finsh_get_device(void);

#endif
