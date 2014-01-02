/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : os_time.c
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
#include "os_time.h"

void time_hwInit(void);

static time_t jiffies;

/*use RTC as timebase, tick period = 1ms*/
void time_Init(void)
{
	jiffies = 0;
	time_hwInit();
}

void time_Update(void)
{
}

void time_isr(void)
{
	jiffies ++;
}

/*unit: ms*/
time_t time_get(int delay)
{
	return (time_t)(jiffies + delay);
}

int time_left(time_t deadline)
{
	unsigned dt, now;
	int left;

	now = jiffies;
	dt = (deadline > now) ? deadline - now : now - deadline;
	if(dt >= (1U << 31)) {
		dt = 0 - (int)(dt);
	}

	left = (deadline > now) ? dt : (0 - dt);
	return left;
}

time_t time_shift(time_t time, int ms)
{
	int result = time + ms;
	return result;
}

int time_diff(time_t t0, time_t t1)
{
	int left = t0 - t1;
	return left;
}

void udelay(int us)
{
	while(us > 0) us--;
}

void mdelay(int ms)
{
	time_t deadline;

	ms *= CONFIG_TICK_HZ;
	ms /= 1000;
	deadline = time_get(ms);
	while(time_left(deadline) > 0);
}

void sdelay(int ss)
{
	mdelay(ss * 1000);
}

#if CONFIG_CPU_STM32 == 1
#include "stm32f10x.h"
#elif CONFIG_CPU_LM3S == 1
#include "lm3s.h"
#elif CONFIG_CPU_SAM3U == 1
#include "sam3u.h"
#endif

void time_hwInit(void)
{
#if CONFIG_CPU_SAM3U == 1
	SysTick_Config(CONFIG_MCK_VALUE / CONFIG_TICK_HZ);
#else
	SysTick_Config(SystemCoreClock / CONFIG_TICK_HZ);
#endif
}

#if 0
#include <stdio.h>
#include <string.h>
int main(int argc, char *argv[])
{
        time_t t0, t1;
        int ms = 0, delta = 0, mode = 't', steps = 17;
        const char *usage = {
                "time t ms              return t1 = time_shift(t0, ms) and time_diff(t1, t0)\n"
                "time t ms 100          t scan mode, delta t = 100\n"
                "time t ms 100ms        ms scan mode, delta ms = 100mS\n"
                "time t ms mode steps   scan mode, if step is not given, default to 17\n"
        };

        switch(argc) {
        case 5:
                if(argv[4][1] == 'x') sscanf(argv[4], "0x%x", &steps);
                else sscanf(argv[4], "%d", &steps);
        case 4:
                if(argv[3][1] == 'x') sscanf(argv[3], "0x%x", &delta);
                else sscanf(argv[3], "%d", &delta);
                mode = (argv[3][strlen(argv[3])-1] == 's') ? 's' : 't';
        case 3:
                if(argv[2][1] == 'x') sscanf(argv[2], "0x%x", &ms);
                else sscanf(argv[2], "%d", &ms);
        case 2:
                if(argv[1][1] == 'x') sscanf(argv[1], "0x%x", &t0);
                else sscanf(argv[1], "%d", &t0);
                break;
        default:
                printf("%s", usage);
                return 0;
        }

        printf("t0 = 0x%08x(%d)\n", t0, t0);
        printf("ms = 0x%08x(%d)\n", ms, ms);
        printf("delta = 0x%08x(%d)\n", delta, delta);
        printf("steps = 0x%08x(%d)\n", steps, steps);

        for(int i = 0; i < steps; i ++) {
                t1 = time_shift(t0, ms);
                int ms_new = time_diff(t1, t0);
                printf("t0: %d t1: %d ms: %d -> %d\n", t0, t1, ms, ms_new);
                if(delta == 0) break;
                if(mode == 't') t0 += delta;
                else ms += delta;
        }
}
#endif
