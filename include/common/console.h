/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : console.h
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
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "device.h"

/**
 * @addtogroup KernelService
 */
/*@{*/

device_t console_init(const char *name);
void     console_show_version(void);
device_t console_get(void);
device_t console_set_by_name(const char *name);
device_t console_set_by_device(device_t dev);
device_t console_switch_device(device_t dev);
size_t   console_read(void *buffer, size_t size);
size_t   console_write(const void *buffer, size_t size);
void     console_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /*__CONSOLE_H__*/


