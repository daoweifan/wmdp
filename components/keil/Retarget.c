/*----------------------------------------------------------------------------
 * Name:    Retarget.c
 * Purpose: 'Retarget' layer for target-dependent low level functions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2012 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <rt_misc.h>
#include "common/console.h"

#pragma import(__use_no_semihosting_swi)

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

/*
* writes the character specified by c (converted to an unsigned char) to
* the output stream pointed to by stream, at the position indicated by the
* asociated file position indicator (if defined), and advances the
* indicator appropriately. If the file position indicator is not defined,
* the character is appended to the output stream.
* Returns: the character written. If a write error occurs, the error
*          indicator is set and fputc returns EOF.
*/
int fputc(int c, FILE * f)
{
  return console_write(&c, 1);
}

/*
* writes the string pointed to by s to the stream pointed to by stream.
* The terminating null character is not written.
* Returns: EOF if a write error occurs; otherwise it returns a nonnegative
*          value.
*/
int fputs(const char * s, FILE * f)
{
  return console_write(s, sizeof(s));
}

/*
* obtains the next character (if present) as an unsigned char converted to
* an int, from the input stream pointed to by stream, and advances the
* associated file position indicator (if defined).
* Returns: the next character from the input stream pointed to by stream.
*          If the stream is at end-of-file, the end-of-file indicator is
*          set and fgetc returns EOF. If a read error occurs, the error
*          indicator is set and fgetc returns EOF.
*/
int fgetc(FILE *f) {
  int ch;
  console_read(&ch, 1);
  return (ch&0xff);
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int c) {
  console_write(&c, 1);
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
