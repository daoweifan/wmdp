/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : spi_dev.h
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

#ifndef __XPT2046__H_
#define __XPT2046__H_

#ifdef __cplusplus
extern "C" {
#endif

#include "spi.h"

/**
 * xpt2046_hw_init
 * called     : called in platform init
 * spidev_name: virtual spi device name which has been attached to spi bus
 * dev_name   : this device name, such as 'xpt2046'
 */
void xpt2046_hw_init(const char *spidev_name, const char *dev_name);

#ifdef __cplusplus
}
#endif

#endif
