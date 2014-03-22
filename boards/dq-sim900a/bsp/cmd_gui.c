/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : cmd_lcd.c
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "command.h"
#include "os.h"
#include "gui.h"

/************* LCD Module debug command **************/
extern GUI_CONST_STORAGE GUI_BITMAP bmwendy;

static int cmd_gui_func(int argc, char *argv[])
{
	int temp1, temp2;

	const char *usage = { \
		"usage:\n" \
		" gui print str         , output string on screen\n" \
		" gui print str x y     , output string on screen at x y\n" \
		" gui bmp wendy         , draw a bmp picture 240*320\n" \
	};
	
	if(argc < 2) {
		printf(usage);
		return 0;
	}

	if (!strcmp(argv[1], "print")) {
		if (argc == 3) {
			GUI_DispString(argv[2]);
		} else if (argc == 5) {
			temp1 = atoi(argv[3]);
			temp2 = atoi(argv[4]);
			GUI_DispStringAt(argv[2], temp1, temp2);
		}
		return 0;
	}

	if (!strcmp(argv[1], "bmp")) {
		if (argc == 3) {
			GUI_Clear();
			GUI_DrawBitmap((void const *)&bmwendy, 0, 0);
		}
		return 0;
	}

	return 0;
}
const cmd_t cmd_gui = {"gui", cmd_gui_func, "ucGUI Module debug command"};
EXPORT_SHELL_CMD(cmd_gui)
