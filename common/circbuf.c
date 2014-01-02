/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : circbuf.c
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
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "common/circbuf.h"


int buf_init (circbuf_t * buf, unsigned int size)
{
	assert (buf != NULL);

	buf->size = 0;
	buf->totalsize = size;
	buf->data = (char *) malloc (sizeof (char) * size);
	assert (buf->data != NULL);

	buf->top = buf->data;
	buf->tail = buf->data;
	buf->end = &(buf->data[size]);

	return 1;
}

int buf_free (circbuf_t * buf)
{
	assert (buf != NULL);
	assert (buf->data != NULL);

	free (buf->data);
	memset (buf, 0, sizeof (circbuf_t));

	return 1;
}

int buf_pop (circbuf_t * buf, char *dest, unsigned int len)
{
	unsigned int i;
	char *p = buf->top;

	assert (buf != NULL);
	assert (dest != NULL);

	/* Cap to number of bytes in buffer */
	if (len > buf->size)
		len = buf->size;

	for (i = 0; i < len; i++) {
		dest[i] = *p++;
		/* Bounds check. */
		if (p == buf->end) {
			p = buf->data;
		}
	}

	/* Update 'top' pointer */
	buf->top = p;
	buf->size -= len;

	return len;
}

int buf_push (circbuf_t * buf, const char *src, unsigned int len)
{
	/* NOTE:  this function allows push to overwrite old data. */
	unsigned int i;
	char *p = buf->tail;

	assert (buf != NULL);
	assert (src != NULL);

	for (i = 0; i < len; i++) {
		*p++ = src[i];
		if (p == buf->end) {
			p = buf->data;
		}
		/* Make sure pushing too much data just replaces old data */
		if (buf->size < buf->totalsize) {
			buf->size++;
		} else {
			buf->top++;
			if (buf->top == buf->end) {
				buf->top = buf->data;
			}
		}
	}

	/* Update 'tail' pointer */
	buf->tail = p;

	return len;
}
