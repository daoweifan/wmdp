;/*
;*********************************************************************************************************
;*                                                WMDP
;*                                          WM Develop Platform
;*
;*                              (c) Copyright 2010-2014, WM, China
;*                                           All Rights Reserved
;*
;* File    : interrupt_cm3_iar.s
;* By      : Fan Daowei
;* Version : V1.0
;*
;* LICENSING TERMS:
;* ---------------
;*   WMDP is provided in source form for FREE evaluation, for educational use or for peaceful research.
;* If you plan on using  WMDP  in a commercial product you need to contact WM to properly license
;* its use in your product. We provide ALL the source code for your convenience and to help you experience
;* WMDP.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a
;* licensing fee.
;*********************************************************************************************************
;*/
    RSEG    CODE:CODE(2)
    thumb

    PUBLIC DISABLE_INTERRUPTS
    PUBLIC ENABLE_INTERRUPTS
    PUBLIC ENTER_CRITICAL_SECTION
    PUBLIC LEAVE_CRITICAL_SECTION

/*-----------------------------------------------------------*/

DISABLE_INTERRUPTS:
    PUSH { r0 }
    MOV  R0, #191
    MSR  BASEPRI, R0
    POP  { R0 }
    BX   R14
/*-----------------------------------------------------------*/

ENABLE_INTERRUPTS:
    PUSH { r0 }
    MOV  R0, #0
    MSR  BASEPRI, R0
    POP  { R0 }
    BX   R14
/*-----------------------------------------------------------*/

;/*
; * int ENTER_CRITICAL_SECTION(void);
; */
    EXPORT ENTER_CRITICAL_SECTION
ENTER_CRITICAL_SECTION:
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR

;/*
; * void LEAVE_CRITICAL_SECTION(int level);
; */
    EXPORT  LEAVE_CRITICAL_SECTION
LEAVE_CRITICAL_SECTION:
    MSR     PRIMASK, r0
    BX      LR

    END
