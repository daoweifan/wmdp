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

/************* LCD Module debug command **************/
static device_t lcd = NULL;
static int cmd_lcd_func(int argc, char *argv[])
{
	int color, temp1, temp2, temp3, temp4;
	struct device_graphic_ops * lcd_ops;
	struct device_graphic_info lcd_info;

	const char *usage = { \
		"usage:\n" \
		" lcd init name              , initialize lcd module with name\n" \
		" lcd get info               , get the lcd info\n" \
		" lcd vdraw color x y1 y2    , draw vertical line x y1 y2\n" \
		" lcd hdraw color x1 x2 y    , draw horizontal line x1 x2 y\n" \
		" lcd rect color x0 y0 x1 y1 , fill rect with color(hex)\n" \
	};
	
	if(argc < 2) {
		printf(usage);
		return 0;
	}

	if (!strcmp(argv[1], "init") && (argc == 3)) {
		lcd = device_find_by_name(argv[2]);
		if (lcd == NULL)
			printf("There is no device %s", argv[2]);
		return 0;
	}

	if (!strcmp(argv[2], "info") && (argc == 3)) {
		if (lcd == NULL) {
			printf("There is no device init");
			return 0;
		}
		device_control(lcd, GRAPHIC_CTRL_GET_INFO, &lcd_info);
		printf("width: %d, height: %d\n", lcd_info.width, lcd_info.height);
	}

	if (!strcmp(argv[1], "vdraw")) {
		if (lcd == NULL) {
			printf("There is no device init");
			return 0;
		}
		color = atoi(argv[2]);
		temp1 = atoi(argv[3]);
		temp2 = atoi(argv[4]);
		temp3 = atoi(argv[5]);
		lcd_ops = (struct device_graphic_ops *)lcd->user_data;
		lcd_ops->draw_vline(&color, temp1, temp2, temp3);
		return 0;
	}

	if (!strcmp(argv[1], "hdraw")) {
		if (lcd == NULL) {
			printf("There is no device init");
			return 0;
		}
		color = atoi(argv[2]);
		temp1 = atoi(argv[3]);
		temp2 = atoi(argv[4]);
		temp3 = atoi(argv[5]);
		lcd_ops = (struct device_graphic_ops *)lcd->user_data;
		lcd_ops->draw_hline(&color, temp1, temp2, temp3);
		return 0;
	}


	if (!strcmp(argv[1], "rect")) {
		if (lcd == NULL) {
			printf("There is no device init");
			return 0;
		}
		sscanf(argv[2], "%x", &color);
		temp1 = atoi(argv[3]);
		temp2 = atoi(argv[4]);
		temp3 = atoi(argv[5]);
		temp4 = atoi(argv[6]);
		lcd_ops = (struct device_graphic_ops *)lcd->user_data;
		lcd_ops->fill_rect(&color, temp1, temp2, temp3, temp4);
		return 0;
	}

	return 0;
}
const cmd_t cmd_lcd = {"lcd", cmd_lcd_func, "LCD Module debug command"};
EXPORT_SHELL_CMD(cmd_lcd)
