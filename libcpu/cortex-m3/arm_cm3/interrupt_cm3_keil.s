;/*
;*********************************************************************************************************
;*                                                WMDP
;*                                          WM Develop Platform
;*
;*                              (c) Copyright 2010-2014, WM, China
;*                                           All Rights Reserved
;*
;* File    : interrupt_cm3_keil.s
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

;/**
; * @addtogroup CORTEX-M3
; */
;/*@{*/

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

;/*
; * void DISABLE_INTERRUPTS();
; */
DISABLE_INTERRUPTS    PROC
    EXPORT  DISABLE_INTERRUPTS
    PUSH    { r0 }
    MOV     R0, #191
    MSR     BASEPRI, R0
    POP     { R0 }
    BX      R14
    
    ENDP

;/*
; * void ENABLE_INTERRUPTS();
; */
ENABLE_INTERRUPTS    PROC
    EXPORT  ENABLE_INTERRUPTS
    PUSH    { r0 }
    MOV     R0, #0
    MSR     BASEPRI, R0
    POP     { R0 }
    BX      R14
    
    ENDP

;/*
; * int ENTER_CRITICAL_SECTION();
; */
ENTER_CRITICAL_SECTION    PROC
    EXPORT  ENTER_CRITICAL_SECTION
    MRS     r0, PRIMASK
    CPSID   I
    BX      LR
    ENDP

;/*
; * void LEAVE_CRITICAL_SECTION(int level);
; */
LEAVE_CRITICAL_SECTION    PROC
    EXPORT  LEAVE_CRITICAL_SECTION
    MSR     PRIMASK, r0
    BX      LR
    ENDP

    END
