/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : ringbuf.h
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
#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include "def.h"
#include "debug.h"

/* ring buffer */
typedef struct ringbuf {
    uint8_t *buf_ptr;
    /* use the msb of the {read,write}_index as mirror bit. You can see this as
     * if the buffer adds a virtual mirror and the pointers point either to the
     * normal or to the mirrored buffer. If the write_index has the same value
     * with the read_index, but in a different mirror, the buffer is full.
     * While if the write_index and the read_index are the same and within the
     * same mirror, the buffer is empty. The ASCII art of the ringbuf is:
     *
     *          mirror = 0                    mirror = 1
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Full
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     *  read_idx-^                   write_idx-^
     *
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * | 0 | 1 | 2 | 3 | 4 | 5 | 6 ||| 0 | 1 | 2 | 3 | 4 | 5 | 6 | Empty
     * +---+---+---+---+---+---+---+|+~~~+~~~+~~~+~~~+~~~+~~~+~~~+
     * read_idx-^ ^-write_idx
     *
     * The tradeoff is we could only use 32KiB of buffer for 16 bit of index.
     * But it should be enough for most of the cases.
     *
     * Ref: http://en.wikipedia.org/wiki/Circular_buffer#Mirroring */
    uint16_t read_mirror : 1;
    uint16_t read_index : 15;
    uint16_t write_mirror : 1;
    uint16_t write_index : 15;
    /* as we use msb of index as mirror bit, the size should be signed and
     * could only be positive. */
    int16_t buf_size;
} ringbuf_t;

/**
 * ringbuf for DeviceDriver
 *
 * Please note that the ring buffer implementation of RT-Thread
 * has no thread wait or resume feature.
 */
void ringbuf_init(struct ringbuf *rb, uint8_t *pool, int16_t size);
size_t ringbuf_put(struct ringbuf *rb, const uint8_t *ptr, uint16_t length);
size_t ringbuf_put_force(struct ringbuf *rb, const uint8_t *ptr, uint16_t length);
size_t ringbuf_putchar(struct ringbuf *rb, const uint8_t ch);
size_t ringbuf_putchar_force(struct ringbuf *rb, const uint8_t ch);
size_t ringbuf_get(struct ringbuf *rb, uint8_t *ptr, uint16_t length);
size_t ringbuf_getchar(struct ringbuf *rb, uint8_t *ch);

enum ringbuf_state {
    ringbuf_EMPTY,
    ringbuf_FULL,
    /* half full is neither full nor empty */
    ringbuf_HALFFULL,
};

INLINE uint16_t ringbuf_get_size(struct ringbuf *rb)
{
    ASSERT(rb != RT_NULL);
    return rb->buf_size;
}

INLINE enum ringbuf_state ringbuf_status(struct ringbuf *rb)
{
    if (rb->read_index == rb->write_index) {
        if (rb->read_mirror == rb->write_mirror)
            return ringbuf_EMPTY;
        else
            return ringbuf_FULL;
    }
    return ringbuf_HALFFULL;
}

/** return the size of data in rb */
INLINE uint16_t ringbuf_data_len(struct ringbuf *rb)
{
    switch (ringbuf_status(rb)) {
    case ringbuf_EMPTY:
        return 0;
    case ringbuf_FULL:
        return rb->buf_size;
    case ringbuf_HALFFULL:
    default:
        if (rb->write_index > rb->read_index)
            return rb->write_index - rb->read_index;
        else
            return rb->buf_size - (rb->read_index - rb->write_index);
    };
}

/** return the size of empty space in rb */
#define ringbuf_space_len(rb) ((rb)->buf_size - ringbuf_data_len(rb))


#endif /* __RINGBUF_H__ */

