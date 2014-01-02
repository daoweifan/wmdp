/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : shell.c
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
#ifndef __SHELL_H_
#define __SHELL_H_

#include "wmconfig.h"
#include "device.h"
#include "command.h"


#ifdef CONFIG_TASK_BOOTLOADER
	#define CONFIG_SHELL_PROMPT "boot# "
#else
	#define CONFIG_SHELL_PROMPT "dddp# "
#endif

#ifdef CONFIG_TASK_BOOTLOADER
	#define CONFIG_WMDP_BANNER "Welcome to boot console"
#else
	#define CONFIG_WMDP_BANNER "Welcome to dddp console"
#endif

#ifndef CONFIG_SHELL_LEN_CMD_MAX
#define CONFIG_SHELL_LEN_CMD_MAX 32
#endif

#ifndef CONFIG_SHELL_NR_PARA_MAX
#define CONFIG_SHELL_NR_PARA_MAX 8
#endif

#ifndef CONFIG_SHELL_LEN_HIS_MAX
#define CONFIG_SHELL_LEN_HIS_MAX 64
#endif

typedef struct shell_s *shell_t;

struct shell_s {
	struct device * dev;

	short status;
	#define SHELL_CONFIG_MUTE (1<<0)
	#define SHELL_CONFIG_LOCK (1<<1)
	short config;

	/*cmd line*/
	const char *prompt;
	char cmd_buffer[CONFIG_SHELL_LEN_CMD_MAX];
	short cmd_idx;

	/*cmd history*/
	short cmd_hsz; /*cmd history size*/
	char *cmd_history;
	short cmd_hrail;
	short cmd_hrpos;

	struct list_head list; //list of shells
	struct cmd_queue_s cmd_queue;
};

void shell_Init(void);
void shell_Update(void);

/*to dynamic register a new shell console device with specified history buffer size*/
int shell_register(struct device *);
int shell_unregister(struct device *);
int shell_mute_set(struct device *dev, int enable); //disable shell echo
#define shell_mute(cnsl) shell_mute_set(cnsl, 1)
#define shell_unmute(cnsl) shell_mute_set(cnsl, 0)
int shell_lock_set(struct device *dev, int enable); //disable shell input
#define shell_lock(cnsl) shell_lock_set(cnsl, 1)
#define shell_unlock(cnsl) shell_lock_set(cnsl, 0)
int shell_trap(struct device *dev, cmd_t *cmd);
int shell_prompt(struct device *dev, const char *prompt);

/*to execute a specified cmd in specified console*/
int shell_exec_cmd(struct device *, const char *cmdline);

/*read a line from current console, return 0 if not finish*/
int shell_ReadLine(const char *prompt, char *str);

#endif /*__SHELL_H_*/
