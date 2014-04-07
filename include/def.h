/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : def.h
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
#ifndef __DEF_H__
#define __DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ********************************************************************* */
/* Module configuration */


/* ********************************************************************* */
/* Interface macro & data definition */

typedef enum _bool{false,true} bool;
// typedef unsigned char  uint8_t;                    /* Unsigned  8 bit quantity                           */
// typedef signed   char  int8_t;                     /* Signed    8 bit quantity                           */
// typedef unsigned short uint16_t;                   /* Unsigned 16 bit quantity                           */
// typedef signed   short int16_t;                    /* Signed   16 bit quantity                           */
// typedef unsigned long  uint32_t;                   /* Unsigned 32 bit quantity                           */
// typedef signed   long  int32_t;                    /* Signed   32 bit quantity                           */
// typedef float          FP32;                       /* Single precision floating point                    */
// typedef double         FP64;                       /* Double precision floating point                    */

#define boolean        uint8_t
#define off_t          uint32_t
#define err_t          int32_t


typedef unsigned int   OS_STK;                     /* Each stack entry is 16-bit wide                    */
typedef unsigned int   OS_CPU_SR;                  /* Define size of CPU status register (PSR = 32 bits) */

#define BYTE           int8_t                      /* Define data types for backward compatibility ...   */
#define UBYTE          uint8_t                     /* ... to uC/OS V1.xx.  Not actually needed for ...   */
#define WORD           int16_t                     /* ... uC/OS-II.                                      */
#define UWORD          uint16_t
#define LONG           int32_t
#define ULONG          uint32_t

/* maximum value of base type */
#define U8_MAX                    0xff            /* Maxium number of u8   */
#define U16_MAX                   0xffff          /* Maxium number of u16  */
#define U32_MAX                   0xffffffff      /* Maxium number of u32  */

#define WM_NULL                   ((void *)0)

/* Sets the result on bPort */
#define BIT_SET(bPort,bBitMask)        (bPort |= bBitMask)
#define BIT_CLR(bPort,bBitMask)        (bPort &= ~bBitMask)

/* Returns the result */
#define GET_BIT_SET(bPort,bBitMask)    (bPort | bBitMask)
#define GET_BIT_CLR(bPort,bBitMask)    (bPort & ~bBitMask)

/* Returns 0 if the condition is False & a non-zero value if it is True */
#define TEST_BIT_SET(bPort,bBitMask)   (bPort & bBitMask)
#define TEST_BIT_CLR(bPort,bBitMask)   ((~bPort) & bBitMask)

/**
 * @addtogroup Error
 */
/*@{*/
/* wm platform error code definitions */
#define ERROR_EOK                          0               /**< There is no error       */
#define ERROR_OK                           0               /**< There is no error       */
#define ERROR_ERROR                        1               /**< A generic error happens */
#define ERROR_GENERIC                      1               /**< A generic error happens */
#define ERROR_ETIMEOUT                     2               /**< Timed out               */
#define ERROR_EFULL                        3               /**< The resource is full    */
#define ERROR_EEMPTY                       4               /**< The resource is empty   */
#define ERROR_ENOMEM                       5               /**< No memory               */
#define ERROR_ENOSYS                       6               /**< No system               */
#define ERROR_NOSYS                        6               /**< No system               */
#define ERROR_EBUSY                        7               /**< Busy                    */
#define ERROR_BUSY                         7               /**< Busy                    */
#define ERROR_EIO                          8               /**< IO error                */
#define ERROR_IO                           8               /**< IO error                */
/*@}*/


/* Compiler Related Definitions */
#ifdef __CC_ARM                         /* ARM Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define SECTION_BEGIN(x)            &##x##$$Base
    #define SECTION_END(x)              &##x##$$Limit
    #define UNUSED                      __attribute__((unused))
    #define USED                        __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define INLINE                      static __inline
    /* module compiling */
    #ifdef RT_USING_MODULE
        #define RTT_API                 __declspec(dllimport)
    #else
        #define RTT_API                 __declspec(dllexport)
    #endif

#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  @ x
    #define SECTION_BEGIN(x)            __section_begin(#x)
    #define SECTION_END(x)              __section_end(#x)
    #define UNUSED
    #define USED
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
    #define INLINE                      static inline
    #define RTT_API

#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #ifdef RT_USING_NEWLIB
        #include <stdarg.h>
    #else
        #if __GNUC__ < 4
            typedef void *__sys_va_list;
            typedef __sys_va_list       va_list;
            #define __va_rounded_size(type) \
                (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
            #define va_start(ap, lastarg)   \
                (ap = ((char *) &(lastarg) + __va_rounded_size(lastarg)))
            #define va_end(ap)          ((void)0)
            /*  little endian */
            #define va_arg(ap, type)    \
                (ap = (__sys_va_list) ((char *)(ap) + __va_rounded_size(type)),  \
                *((type *) (void *) ((char *)(ap) - __va_rounded_size(type))))
        #else
            typedef __builtin_va_list   __gnuc_va_list;
            typedef __gnuc_va_list      va_list;
            #define va_start(v,l)       __builtin_va_start(v,l)
            #define va_end(v)           __builtin_va_end(v)
            #define va_arg(v,l)         __builtin_va_arg(v,l)
        #endif
    #endif

    #define SECTION(x)                  __attribute__((section(x)))
    #define UNUSED                      __attribute__((unused))
    #define USED                        __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define rt_inline                   static __inline
    #define RTT_API
#elif defined (__ADSPBLACKFIN__)        /* for VisualDSP++ Compiler */
    #include <stdarg.h>
    #define SECTION(x)                  __attribute__((section(x)))
    #define UNUSED                      __attribute__((unused))
    #define USED                        __attribute__((used))
    #define ALIGN(n)                    __attribute__((aligned(n)))
    #define rt_inline                   static inline
    #define RTT_API
#elif defined (_MSC_VER)
    #include <stdarg.h>
    #define SECTION(x)
    #define UNUSED
    #define USED
    #define ALIGN(n)                    __declspec(align(n))
    #define rt_inline                   static __inline
    #define RTT_API
#elif defined (__TI_COMPILER_VERSION__)
    #include <stdarg.h>
    /* The way that TI compiler set section is different from other(at least
     * GCC and MDK) compilers. See ARM Optimizing C/C++ Compiler 5.9.3 for more
     * details. */
    #define SECTION(x)
    #define UNUSED
    #define USED
    #define ALIGN(n)
    #define rt_inline                   static inline
    #define RTT_API
#else
    #error not supported tool chain
#endif

#ifdef __cplusplus
}
#endif

#endif /*__DEF_H__*/

