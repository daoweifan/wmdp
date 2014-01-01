/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : interrupt.h
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
#ifndef INTERRUPT_H
#define INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dddp_config.h"

#if (CONFIG_CPU_STM32==1) || (CONFIG_CPU_LM3S==1) || (CONFIG_CPU_SAM3U==1)
void DISABLE_INTERRUPTS(void);
void ENABLE_INTERRUPTS(void);
int ENTER_CRITICAL_SECTION(void);
void LEAVE_CRITICAL_SECTION(int);
#endif

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

