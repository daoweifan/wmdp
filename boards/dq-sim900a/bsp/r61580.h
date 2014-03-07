/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : r61580.h
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
#ifndef _R61580_H
#define _R61580_H

#include "device.h"

// Compatible list:

/* LCD color */
#define White            0xFFFF
#define Black            0x0000
#define Grey             0xF7DE
#define Blue             0x001F
#define Blue2            0x051F
#define Red              0xF800
#define Magenta          0xF81F
#define Green            0x07E0
#define Cyan             0x7FFF
#define Yellow           0xFFE0

/*---------------------- Graphic LCD size definitions ------------------------*/
#define LCD_WIDTH       240                 /* Screen Width (in pixels)           */
#define LCD_HEIGHT      320                 /* Screen Hight (in pixels)           */
#define BPP             16                  /* Bits per pixel                     */
#define BYPP            ((BPP+7)/8)         /* Bytes per pixel                    */

#endif // _R61580_H
