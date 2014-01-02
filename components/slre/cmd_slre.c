/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : cmd_slre.c
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
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "command.h"
#include "slre.h"

static int cmd_slre_func(int argc, char *argv[])
{
	const char *usage = { \
		"usage:\n " \
		"slre test1, show example one\n" \
	};
	
	if(argc < 2) {
		printf(usage);
		return 0;
	}

	if (!strcmp(argv[1], "test1")) {
		const char *error_msg, *request = " GET /index.html HTTP/1.0\r\n\r\n";
		struct slre_cap caps[4];

		if (slre_match("^\\s*(\\S+)\\s+(\\S+)\\s+HTTP/(\\d)\\.(\\d)",
					   request, strlen(request), caps, 4, &error_msg)) {
		  printf("Method: [%.*s], URI: [%.*s]\n",
				 caps[2].len, caps[2].ptr,
				 caps[3].len, caps[3].ptr);
		} else {
		  printf("Error parsing [%s]: [%s]\n", request, error_msg);
		}
	}

	return 0;
}
const cmd_t cmd_slre = {"slre", cmd_slre_func, "slre test cmds for understand regular expression"};
EXPORT_SHELL_CMD(cmd_slre)
