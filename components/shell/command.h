/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : command.h
* By      : Fan Daowei
* Version : V1.0
*
* LICENSING TERMS:
* ---------------
*   WMDP is provided in source form for FREE evaluation, for educational use or for peaceful research.
* If you plan on using  WMDP  in a commercial product you need to contact WM to properly license
* its use in your product. We provide ALL the source code for your convenience and to help you experience
* WMDP.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a
* licensing fee.
*********************************************************************************************************
*/
#ifndef __CMD_H_
#define __CMD_H_

#include <stdio.h>
#include "def.h"
#include "os_time.h"
#include "list.h"

typedef const struct {
	char *name;
/*cmd_xxx_func(int argc, char *argv) note:
	1, return code: 0 -> finished success, <0 finished with err, >0 repeat exec needed
	2, in repeat exec mode, argc = 0;
*/
	int (*func)(int argc, char *argv[]);
	char *help;
} cmd_t;

struct cmd_list_s {
	char *cmdline;
	short len;
	short ms; //repeat period
	time_t deadline;
	struct list_head list;
};

struct cmd_queue_s {
	int flag;
	cmd_t *trap;
	struct list_head cmd_list;
};

#define CMD_FLAG_REPEAT 1

#ifdef __CC_ARM      /* ARM Compiler */
    extern const int shell_cmd$$Base;
    extern const int shell_cmd$$Limit;
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #pragma section="shell_cmd" 4
#else
    #error not supported tool chain
#endif

/* command table pragma */
#define EXPORT_SHELL_CMD(cmd) \
	const cmd_t * const cmd##_entry SECTION("shell_cmd") = &##cmd;

/*cmd module i/f*/
void cmd_Init(void);
void cmd_Update(void);

/*cmd queue ops*/
int cmd_queue_init(struct cmd_queue_s *);
int cmd_queue_update(struct cmd_queue_s *);
int cmd_queue_exec(struct cmd_queue_s *, const char *);

/*common command related subroutines*/
int cmd_pattern_get(const char *str); //get pattern from a string, such as: "0,2-5,8" or "all,1" for inverse selection

#endif /*__CMD_H_*/
