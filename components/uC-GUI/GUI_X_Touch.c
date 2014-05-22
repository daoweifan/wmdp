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
File        : GUI_TOUCH_X.C
Purpose     : Config / System dependent externals for GUI
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_X.h"
#include "device.h"
#include "os.h"
#include "gui.h"
#include "GUITouchConf.h"
#include "LCDConf.h"

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

#define LCD_CROSS_X1 (50)
#define LCD_CROSS_X2 (LCD_XSIZE - 50)
#define LCD_CROSS_X3 (50)
#define LCD_CROSS_X4 (LCD_XSIZE - 50)
#define LCD_CROSS_Y1 (50)
#define LCD_CROSS_Y2 (50)
#define LCD_CROSS_Y3 (LCD_YSIZE - 50)
#define LCD_CROSS_Y4 (LCD_YSIZE - 50)

static void LCD_PUTCROSS(int x, int y)
{
	GUI_DrawHLine(y, x-5, x+5);
	GUI_DrawVLine(x, y-5, y+5);
}

/* calibrate the touch screen */
void GUI_TOUCH_X_Calibration(void)
{
	int result;
	uint16_t x1position, y1position;
	uint16_t x4position, y4position;

	do {
		GUI_Clear();
		/* first point */
		LCD_PUTCROSS(LCD_CROSS_X1, LCD_CROSS_Y1);
		do {
			device_control(gui_touch, TOUCH_GET_X_POSITION, &x1position);
			device_control(gui_touch, TOUCH_GET_Y_POSITION, &y1position);
		} while(!((x1position > 0x00A0) && (x1position < 0x0738) && (y1position > 0x0118) && (y1position < 0x06BB)));

		/* forth point */
		udelay(5000000);
		LCD_PUTCROSS(LCD_CROSS_X4, LCD_CROSS_Y4);
		GUI_DrawLine(LCD_CROSS_X1,LCD_CROSS_Y1, LCD_CROSS_X4, LCD_CROSS_Y4);
		do {
			device_control(gui_touch, TOUCH_GET_X_POSITION, &x4position);
			device_control(gui_touch, TOUCH_GET_Y_POSITION, &y4position);
		} while(!((x4position > 0x00A0) && (x4position < 0x0738) && (y4position > 0x0118) && (y4position < 0x06BB)));

		/* calibration */
		result = 0;
		result = GUI_TOUCH_Calibrate(0, LCD_CROSS_X1, LCD_CROSS_X4, x1position, x4position);
		result += GUI_TOUCH_Calibrate(1, LCD_CROSS_Y1, LCD_CROSS_Y4, y1position, y4position);
	} while (result);
}

EXPORT_TASK_INIT(GUI_TOUCH_X_Calibration)
