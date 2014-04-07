/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCDConf_1375_C8_C320x240.h
Purpose     : Sample configuration file
----------------------------------------------------------------------
*/

#ifndef LCDCONF_H
#define LCDCONF_H

/*********************************************************************
*
*                   General configuration of LCD
*
**********************************************************************
*/

#define LCD_XSIZE      (240)   /* X-resolution of LCD, Logical coor. */
#define LCD_YSIZE      (320)   /* Y-resolution of LCD, Logical coor. */

#define LCD_BITSPERPIXEL (16)

#define LCD_CONTROLLER 0
#define LCD_BUSWIDTH      (16)
#define LCD_FIXEDPALETTE  (565)
#define LCD_SWAP_RB        1

/*********************************************************************
*
*                   Full bus configuration
*
**********************************************************************
*/

// #define LCD_READ_MEM(Off)            *((U16*)         (0xc00000+(((U32)(Off))<<1)))
// #define LCD_WRITE_MEM(Off,data)      *((U16*)         (0xc00000+(((U32)(Off))<<1)))=data
// #define LCD_READ_REG(Off)            *((volatile U16*)(0xc1ffe0+(((U16)(Off))<<1)))
// #define LCD_WRITE_REG(Off,data)      *((volatile U16*)(0xc1ffe0+(((U16)(Off))<<1)))=data

//extern int  LCD_L0_Init(void);
#define LCD_INIT_CONTROLLER() LCD_L0_Init()
 
#endif /* LCDCONF_H */
