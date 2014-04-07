/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : os.c
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
#include "wmconfig.h"
#include "def.h"
#include "os.h"
#include "os_time.h"
#include "device.h"
#include "common/console.h"

typedef union {
	struct {
		unsigned char Time1msLoopFlag     :1;
		unsigned char Time1msLoopOverFlag :1;
		unsigned char rsv                 :6;
	}bf;
	unsigned char byte;
} PitFlagsType;

static volatile PitFlagsType Pit_Flags;
static volatile int OS_Error;

/*normally this routine will be provided by cpu/xxx/cmsis/system_xxx.c*/
extern void SystemInit (void);

int OS_Get_Error(void)
{
	return OS_Error;
}

void OS_Set_Error(int error)
{
	OS_Error = error;
}

void OS_Init(void)
{
	void (**init)(void);
	void (**end)(void);

	DISABLE_INTERRUPTS();

	/* system basic modules init */
	// SystemInit(); //has been called in startup.s
	time_Init();
// #if CONFIG_DRIVER_LED == 1
	// led_Init();
// #endif
// #if (CONFIG_IAR_REDIRECT == 1) || (CONFIG_TASK_SHELL == 1)
	// console_Init();
// #endif
// #if (CONFIG_FLASH_NVM == 1)
	// nvm_init();
// #endif

	/* for board hardware modules init */
	init = (void (**)(void))SECTION_BEGIN(board_init);
	end = (void (**)(void))SECTION_END(board_init);
	while(init < end) {
		(*init)();
		init ++;
	}

	// device_init_all();
#ifdef CONFIG_CONSOLE_UART1
	console_init("uart1");
#elif defined CONFIG_CONSOLE_UART2
	console_init("uart2");
#else
	#error there is no device configured to console
#endif

	/* for component_init modules init */
	init = (void (**)(void))SECTION_BEGIN(component_init);
	end = (void (**)(void))SECTION_END(component_init);
	while(init < end) {
		(*init)();
		init ++;
	}

#ifdef CONFIG_USING_UCGUI
	int GUI_Init(void);
	/* ucgui init */
	GUI_Init();
#endif

	/* for task modules init */
	init = (void (**)(void))SECTION_BEGIN(task_init);
	end = (void (**)(void))SECTION_END(task_init);
	while(init < end) {
		(*init)();
		init ++;
	}

// #if (CONFIG_DRIVER_WDT == 1)
	// wdt_init(200);
// #endif

	ENABLE_INTERRUPTS();
}

void OS_BG_Update(void)
{
	void (**update)(void);
	void (**end)(void);
	static volatile int loop_4ms = 0;
	static volatile int loop_8ms = 0;
	static volatile int loop_16ms = 0;

	/* for os background update */
	update = (void (**)(void))SECTION_BEGIN(os_bg);
	end = (void (**)(void))SECTION_END(os_bg);
	while(update < end) {
		(*update)();
		update ++;
	}


	if (Pit_Flags.bf.Time1msLoopFlag) {
		Pit_Flags.bf.Time1msLoopFlag = 0;

		/* 1ms time os schedule */
		update = (void (**)(void))SECTION_BEGIN(os_bg_1ms);
		end = (void (**)(void))SECTION_END(os_bg_1ms);
		while(update < end) {
			(*update)();
			update ++;
		}

		/* 4ms time os schedule */
		loop_4ms++;
		if (loop_4ms & 0x04) {
			loop_4ms = 0;
		}
		update = (void (**)(void))SECTION_BEGIN(os_bg_4ms);
		end = (void (**)(void))SECTION_END(os_bg_4ms);
		update += loop_4ms;
		while(update < end) {
			(*update)();
			update += 4;
		}

		/* 8ms time os schedule */
		loop_8ms++;
		if (loop_8ms & 0x08) {
			loop_8ms = 0;
		}
		update = (void (**)(void))SECTION_BEGIN(os_bg_8ms);
		end = (void (**)(void))SECTION_END(os_bg_8ms);
		update += loop_8ms;
		while(update < end) {
			(*update)();
			update += 8;
		}

		/* 16ms time os schedule */
		loop_16ms++;
		if (loop_16ms & 0x10) {
			loop_16ms = 0;
		}
		update = (void (**)(void))SECTION_BEGIN(os_bg_16ms);
		end = (void (**)(void))SECTION_END(os_bg_16ms);
		update += loop_16ms;
		while(update < end) {
			(*update)();
			update += 16;
		}

		if (Pit_Flags.bf.Time1msLoopFlag) {
			Pit_Flags.bf.Time1msLoopOverFlag = 1;
		}
	}
}

void OS_FG_Update(void)
{
	void (**update)(void);
	void (**end)(void);
	static volatile int loop_4ms = 0;
	static volatile int loop_8ms = 0;
	static volatile int loop_16ms = 0;

	/* basic time reference */
	time_isr();

	/* 1ms time os schedule */
	Pit_Flags.bf.Time1msLoopFlag = 1;
	update = (void (**)(void))SECTION_BEGIN(os_fg_1ms);
	end = (void (**)(void))SECTION_END(os_fg_1ms);
	while(update < end) {
		(*update)();
		update ++;
	}

	/* 4ms time os schedule */
	loop_4ms++;
	if (loop_4ms & 0x04) {
		loop_4ms = 0;
	}
	update = (void (**)(void))SECTION_BEGIN(os_fg_4ms);
	end = (void (**)(void))SECTION_END(os_fg_4ms);
	update += loop_4ms;
	while(update < end) {
		(*update)();
		update += 4;
	}

	/* 8ms time os schedule */
	loop_8ms++;
	if (loop_8ms & 0x08) {
		loop_8ms = 0;
	}
	update = (void (**)(void))SECTION_BEGIN(os_fg_8ms);
	end = (void (**)(void))SECTION_END(os_fg_8ms);
	update += loop_8ms;
	while(update < end) {
		(*update)();
		update += 8;
	}

	/* 16ms time os schedule */
	loop_16ms++;
	if (loop_16ms & 0x10) {
		loop_16ms = 0;
	}
	update = (void (**)(void))SECTION_BEGIN(os_fg_16ms);
	end = (void (**)(void))SECTION_END(os_fg_16ms);
	update += loop_16ms;
	while(update < end) {
		(*update)();
		update += 16;
	}
}

void SysTick_Handler(void)
{
	OS_FG_Update();
}
