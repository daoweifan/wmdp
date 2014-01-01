/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : os_time.h
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
#ifndef __DDDP_TIME_H_
#define __DDDP_TIME_H_

#include <time.h>

void time_Init(void);
void time_Update(void);
void time_isr(void);
time_t time_get(int delay); //unit: ms
int time_left(time_t deadline); //unit: ms
time_t time_shift(time_t time, int ms);
int time_diff(time_t t0, time_t t1);
void udelay(int us);
void mdelay(int ms);
void sdelay(int ss);

/*rtc interface*/
void rtc_init(unsigned now);
unsigned rtc_get(void);
void rtc_alarm(unsigned t);

#endif /*__ULP_TIME_H_*/
