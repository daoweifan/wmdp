/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : os.h
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
#ifndef __OS_H_
#define __OS_H_

#include "os_time.h"
#include "interrupt.h"

#ifdef __CC_ARM      /* ARM Compiler */
    extern const int board_init$$Base;
    extern const int board_init$$Limit;
    extern const int component_init$$Base;
    extern const int component_init$$Limit;
    extern const int task_init$$Base;
    extern const int task_init$$Limit;
    extern const int shell_cmd$$Base;
    extern const int shell_cmd$$Limit;
    extern const int os_bg$$Base;
    extern const int os_bg$$Limit;
    extern const int os_bg_1ms$$Base;
    extern const int os_bg_1ms$$Limit;
    extern const int os_bg_4ms$$Base;
    extern const int os_bg_4ms$$Limit;
    extern const int os_bg_8ms$$Base;
    extern const int os_bg_8ms$$Limit;
    extern const int os_bg_16ms$$Base;
    extern const int os_bg_16ms$$Limit;
    extern const int os_fg_1ms$$Base;
    extern const int os_fg_1ms$$Limit;
    extern const int os_fg_4ms$$Base;
    extern const int os_fg_4ms$$Limit;
    extern const int os_fg_8ms$$Base;
    extern const int os_fg_8ms$$Limit;
    extern const int os_fg_16ms$$Base;
    extern const int os_fg_16ms$$Limit;
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #pragma section="board_init" 4
    #pragma section="component_init" 4
    #pragma section="task_init" 4
    #pragma section="shell_cmd" 4
    #pragma section="os_bg" 4
    #pragma section="os_bg_1ms" 4
    #pragma section="os_bg_4ms" 4
    #pragma section="os_bg_8ms" 4
    #pragma section="os_bg_16ms" 4
    #pragma section="os_fg_1ms" 4
    #pragma section="os_fg_4ms" 4
    #pragma section="os_fg_8ms" 4
    #pragma section="os_fg_16ms" 4
#else
    #error not supported tool chain
#endif

#define EXPORT_BOARD_INIT(func) \
	void (* const func##_entry)(void) SECTION("board_init") = &##func;

#define EXPORT_COMPONENT_INIT(func) \
	void (* const func##_entry)(void) SECTION("component_init") = &##func;

#define EXPORT_TASK_INIT(func) \
	void (* const func##_entry)(void) SECTION("task_init") = &##func;

/* os background schedule macros */
/* background task pragma */
#define EXPORT_OS_BG_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_bg") = &##task;

/* 1 ms task pragma */
#define EXPORT_OS_BG_1MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_bg_1ms") = &##task;

/* 4 ms task pragma */
#define EXPORT_OS_BG_4MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_bg_4ms") = &##task;

/* 8 ms task pragma */
#define EXPORT_OS_BG_8MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_bg_8ms") = &##task;

/* 16 ms task pragma */
#define EXPORT_OS_BG_16MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_bg_16ms") = &##task;

/* os foreground schedule macros */
/* 1 ms task pragma */
#define EXPORT_OS_FG_1MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_fg_1ms") = &##task;

/* 4 ms task pragma */
#define EXPORT_OS_FG_4MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_fg_4ms") = &##task;

/* 8 ms task pragma */
#define EXPORT_OS_FG_8MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_fg_8ms") = &##task;

/* 16 ms task pragma */
#define EXPORT_OS_FG_16MS_UPDATE(task) \
	void (* const task##_entry)(void) SECTION("os_fg_16ms") = &##task;

void OS_Init(void);
void OS_BG_Update(void);
void OS_FG_Update(void);

int OS_Get_Error(void);
void OS_Set_Error(int error);

#endif /*__TASK_H_*/
