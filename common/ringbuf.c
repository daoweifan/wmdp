/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : ringbuf.C
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
#include "common/ringbuf.h"

void ringbuf_init(struct ringbuf *rb, uint8_t *pool, int16_t size)
{
    ASSERT(rb != RT_NULL);
    ASSERT(size > 0)

    /* initialize read and write index */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* set buffer pool and size */
    rb->buf_ptr = pool;
    rb->buf_size = size;
}

/**
 * put a block of data into ring buffer
 */
size_t ringbuf_put(struct ringbuf *rb, const uint8_t *ptr, uint16_t length)
{
    uint16_t size;

    ASSERT(rb != RT_NULL);

    /* whether has enough space */
    size = ringbuf_space_len(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (rb->buf_size - rb->write_index > length) {
        /* read_index - write_index = empty space */
        memcpy(&rb->buf_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;
        return length;
    }

    memcpy(&rb->buf_ptr[rb->write_index],
           &ptr[0],
           rb->buf_size - rb->write_index);
    memcpy(&rb->buf_ptr[0],
           &ptr[rb->buf_size - rb->write_index],
           length - (rb->buf_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buf_size - rb->write_index);

    return length;
}

/**
 * put a block of data into ring buffer
 *
 * When the buffer is full, it will discard the old data.
 */
size_t ringbuf_put_force(struct ringbuf *rb, const uint8_t *ptr, uint16_t length)
{
    enum ringbuf_state old_state;

    ASSERT(rb != RT_NULL);

    old_state = ringbuf_status(rb);

    if (length > rb->buf_size)
        length = rb->buf_size;

    if (rb->buf_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buf_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;

        if (old_state == ringbuf_FULL)
            rb->read_index = rb->write_index;

        return length;
    }

    memcpy(&rb->buf_ptr[rb->write_index],
           &ptr[0],
           rb->buf_size - rb->write_index);
    memcpy(&rb->buf_ptr[0],
           &ptr[rb->buf_size - rb->write_index],
           length - (rb->buf_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buf_size - rb->write_index);

    if (old_state == ringbuf_FULL)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = rb->write_index;
    }

    return length;
}

/**
 *  get data from ring buffer
 */
size_t ringbuf_get(struct ringbuf *rb, uint8_t *ptr, uint16_t length)
{
    size_t size;

    ASSERT(rb != RT_NULL);

    /* whether has enough data  */
    size = ringbuf_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (rb->buf_size - rb->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &rb->buf_ptr[rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &rb->buf_ptr[rb->read_index],
           rb->buf_size - rb->read_index);
    memcpy(&ptr[rb->buf_size - rb->read_index],
           &rb->buf_ptr[0],
           length - (rb->buf_size - rb->read_index));

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buf_size - rb->read_index);

    return length;
}

/**
 * put a character into ring buffer
 */
size_t ringbuf_putchar(struct ringbuf *rb, const uint8_t ch)
{
    ASSERT(rb != RT_NULL);

    /* whether has enough space */
    if (!ringbuf_space_len(rb))
        return 0;

    rb->buf_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buf_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    }
    else
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * put a character into ring buffer
 *
 * When the buffer is full, it will discard one old data.
 */
size_t ringbuf_putchar_force(struct ringbuf *rb, const uint8_t ch)
{
    enum ringbuf_state old_state;

    ASSERT(rb != RT_NULL);

    old_state = ringbuf_status(rb);

    rb->buf_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buf_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == ringbuf_FULL)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        if (old_state == ringbuf_FULL)
            rb->read_index = rb->write_index;
    }

    return 1;
}

/**
 * get a character from a ringbuffer
 */
size_t ringbuf_getchar(struct ringbuf *rb, uint8_t *ch)
{
    ASSERT(rb != RT_NULL);

    /* ringbuffer is empty */
    if (!ringbuf_data_len(rb))
        return 0;

    /* put character */
    *ch = rb->buf_ptr[rb->read_index];

    if (rb->read_index == rb->buf_size-1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}
