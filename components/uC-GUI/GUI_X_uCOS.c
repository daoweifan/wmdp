/*
*                                    uC/GUI
*              Universal graphic software for embedded applications
*
*              (c) Copyright 2002, Micrium Inc., Weston, FL
*              (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*/
#include <stdio.h>
#include "includes.h"

static OS_EVENT *DispSem;

/*********************************************************************
*
*      Timing:
*                 GUI_X_GetTime
*                 GUI_X_Delay

  Some timing dependent routines of emWin require a GetTime
  and delay funtion. Default time unit (tick), normally is
  1 ms.
*/

int GUI_X_GetTime(void) {
	return OSTimeGet();
}

void GUI_X_Delay(int period) {
	unsigned int ticks;
	ticks=(period*1000)/OS_TICKS_PER_SEC;
	OSTimeDly(ticks);
}

/*********************************************************************
*
*       GUI_X_ExecIdle()
*/
void GUI_X_ExecIdle(void) {
	OSTimeDly(50);
}

/*********************************************************************
*
*      Multitasking:
*
*                 GUI_X_InitOS()
*                 GUI_X_GetTaskId()
*                 GUI_X_Lock()
*                 GUI_X_Unlock()

Note:
  The following routines are required only if emWin is used in a
  true multi task environment, which means you have more than one
  thread using the emWin API.
  In this case the
                      #define GUI_OS 1
  needs to be in GUIConf.h
*/

unsigned int GUI_X_GetTaskId (void)
{
	return ((int)(OSTCBCur->OSTCBPrio));
}

void GUI_X_InitOS (void)
{
	DispSem = OSSemCreate(1);
}

void GUI_X_Unlock (void)
{
	OSSemPost(DispSem);
}

void GUI_X_Lock (void)
{
	INT8U err;
	OSSemPend(DispSem, 0, &err);
}

/*********************************************************************
*
*       GUI_X_Init()
*
* Note:
*     GUI_X_Init() is called from GUI_Init is a possibility to init
*     some hardware which needs to be up and running before the GUI.
*     If not required, leave this routine blank.
*/

void GUI_X_Init(void)
{

}

void GUI_X_Log(const char *s)
{

}

void GUI_X_Warn(const char *s)
{
}

void GUI_X_ErrorOut(const char *s)
{
}
