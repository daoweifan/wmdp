/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              �C/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUI_TOUCH_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_X.h"
#include "device.h"
#include "os.h"
#include <stdio.h>

extern struct device * gui_touch;

void GUI_TOUCH_X_ActivateX(void)
{
	return;
}

void GUI_TOUCH_X_ActivateY(void)
{
	return;
}

int GUI_TOUCH_X_MeasureX(void) 
{
	uint16_t xposition;
	device_control(gui_touch, TOUCH_GET_X_POSITION, &xposition);
	// printf("%x\n", xposition);
	return xposition;
}

int GUI_TOUCH_X_MeasureY(void)
{
	uint16_t yposition;
	device_control(gui_touch, TOUCH_GET_Y_POSITION, &yposition);
	// printf("%x\n", yposition);
	return yposition;
}

void GUI_BG_Callback(void)
{
	GUI_TOUCH_Exec();
}
EXPORT_OS_BG_16MS_UPDATE(GUI_BG_Callback)
