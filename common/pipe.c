/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : pipe.c
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
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

static void _rt_pipe_resume_writer(struct rt_pipe_device *pipe)
{
    if (!rt_list_isempty(&pipe->suspended_write_list))
    {
        rt_thread_t thread;

        RT_ASSERT(pipe->flag & RT_PIPE_FLAG_BLOCK_WR);

        /* get suspended thread */
        thread = rt_list_entry(pipe->suspended_write_list.next,
                struct rt_thread,
                tlist);

        /* resume the write thread */
        rt_thread_resume(thread);

        rt_schedule();
    }
}

static rt_size_t rt_pipe_read(rt_device_t dev,
                              rt_off_t    pos,
                              void       *buffer,
                              rt_size_t   size)
{
    rt_uint32_t level;
    rt_thread_t thread;
    struct rt_pipe_device *pipe;
    rt_size_t read_nbytes;

    pipe = PIPE_DEVICE(dev);
    RT_ASSERT(pipe != RT_NULL);

    if (!(pipe->flag & RT_PIPE_FLAG_BLOCK_RD))
    {
        level = rt_hw_interrupt_disable();
        read_nbytes = rt_ringbuffer_get(&(pipe->ringbuffer), buffer, size);

        /* if the ringbuffer is empty, there won't be any writer waiting */
        if (read_nbytes)
            _rt_pipe_resume_writer(pipe);

        rt_hw_interrupt_enable(level);

        return read_nbytes;
    }

    thread = rt_thread_self();

    /* current context checking */
    RT_DEBUG_NOT_IN_INTERRUPT;

    do {
        level = rt_hw_interrupt_disable();
        read_nbytes = rt_ringbuffer_get(&(pipe->ringbuffer), buffer, size);
        if (read_nbytes == 0)
        {
            rt_thread_suspend(thread);
            /* waiting on suspended read list */
            rt_list_insert_before(&(pipe->suspended_read_list),
                                  &(thread->tlist));
            rt_hw_interrupt_enable(level);

            rt_schedule();
        }
        else
        {
            _rt_pipe_resume_writer(pipe);
            rt_hw_interrupt_enable(level);
            break;
        }
    } while (read_nbytes == 0);

    return read_nbytes;
}

static void _rt_pipe_resume_reader(struct rt_pipe_device *pipe)
{
    if (pipe->parent.rx_indicate)
        pipe->parent.rx_indicate(&pipe->parent,
                                 rt_ringbuffer_data_len(&pipe->ringbuffer));

    if (!rt_list_isempty(&pipe->suspended_read_list))
    {
        rt_thread_t thread;

        RT_ASSERT(pipe->flag & RT_PIPE_FLAG_BLOCK_RD);

        /* get suspended thread */
        thread = rt_list_entry(pipe->suspended_read_list.next,
                struct rt_thread,
                tlist);

        /* resume the read thread */
        rt_thread_resume(thread);

        rt_schedule();
    }
}

static rt_size_t rt_pipe_write(rt_device_t dev,
                               rt_off_t    pos,
                               const void *buffer,
                               rt_size_t   size)
{
    rt_uint32_t level;
    rt_thread_t thread;
    struct rt_pipe_device *pipe;
    rt_size_t write_nbytes;

    pipe = PIPE_DEVICE(dev);
    RT_ASSERT(pipe != RT_NULL);

    if ((pipe->flag & RT_PIPE_FLAG_FORCE_WR) ||
       !(pipe->flag & RT_PIPE_FLAG_BLOCK_WR))
    {
        level = rt_hw_interrupt_disable();

        if (pipe->flag & RT_PIPE_FLAG_FORCE_WR)
            write_nbytes = rt_ringbuffer_put_force(&(pipe->ringbuffer),
                                                   buffer, size);
        else
            write_nbytes = rt_ringbuffer_put(&(pipe->ringbuffer),
                                             buffer, size);

        _rt_pipe_resume_reader(pipe);

        rt_hw_interrupt_enable(level);

        return write_nbytes;
    }

    thread = rt_thread_self();

    /* current context checking */
    RT_DEBUG_NOT_IN_INTERRUPT;

    do {
        level = rt_hw_interrupt_disable();
        write_nbytes = rt_ringbuffer_put(&(pipe->ringbuffer), buffer, size);
        if (write_nbytes == 0)
        {
            /* pipe full, waiting on suspended write list */
            rt_thread_suspend(thread);
            /* waiting on suspended read list */
            rt_list_insert_before(&(pipe->suspended_write_list),
                                  &(thread->tlist));
            rt_hw_interrupt_enable(level);

            rt_schedule();
        }
        else
        {
            _rt_pipe_resume_reader(pipe);
            rt_hw_interrupt_enable(level);
            break;
        }
    } while (write_nbytes == 0);

    return write_nbytes;
}

static rt_err_t rt_pipe_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    if (cmd == PIPE_CTRL_GET_SPACE && args)
        *(rt_size_t*)args = rt_ringbuffer_space_len(&PIPE_DEVICE(dev)->ringbuffer);
    return RT_EOK;
}

/**
 * This function will initialize a pipe device and put it under control of
 * resource management.
 *
 * @param pipe the pipe device
 * @param name the name of pipe device
 * @param flag the attribute of the pipe device
 * @param buf  the buffer of pipe device
 * @param size the size of pipe device buffer
 *
 * @return the operation status, RT_EOK on successful
 */
rt_err_t rt_pipe_init(struct rt_pipe_device *pipe,
                      const char *name,
                      enum rt_pipe_flag flag,
                      rt_uint8_t *buf,
                      rt_size_t size)
{
    RT_ASSERT(pipe);
    RT_ASSERT(buf);

    /* initialize suspended list */
    rt_list_init(&pipe->suspended_read_list);
    rt_list_init(&pipe->suspended_write_list);

    /* initialize ring buffer */
    rt_ringbuffer_init(&pipe->ringbuffer, buf, size);

    pipe->flag = flag;

    /* create pipe */
    pipe->parent.type    = RT_Device_Class_Pipe;
    pipe->parent.init    = RT_NULL;
    pipe->parent.open    = RT_NULL;
    pipe->parent.close   = RT_NULL;
    pipe->parent.read    = rt_pipe_read;
    pipe->parent.write   = rt_pipe_write;
    pipe->parent.control = rt_pipe_control;

    return rt_device_register(&(pipe->parent), name, RT_DEVICE_FLAG_RDWR);
}
RTM_EXPORT(rt_pipe_init);

/**
 * This function will detach a pipe device from resource management
 *
 * @param pipe the pipe device
 *
 * @return the operation status, RT_EOK on successful
 */
rt_err_t rt_pipe_detach(struct rt_pipe_device *pipe)
{
    return rt_device_unregister(&pipe->parent);
}
RTM_EXPORT(rt_pipe_detach);

#ifdef RT_USING_HEAP
rt_err_t rt_pipe_create(const char *name, enum rt_pipe_flag flag, rt_size_t size)
{
    rt_uint8_t *rb_memptr = RT_NULL;
    struct rt_pipe_device *pipe = RT_NULL;

    /* get aligned size */
    size = RT_ALIGN(size, RT_ALIGN_SIZE);
    pipe = (struct rt_pipe_device *)rt_calloc(1, sizeof(struct rt_pipe_device));
    if (pipe == RT_NULL)
        return -RT_ENOMEM;

    /* create ring buffer of pipe */
    rb_memptr = rt_malloc(size);
    if (rb_memptr == RT_NULL)
    {
        rt_free(pipe);
        return -RT_ENOMEM;
    }

    return rt_pipe_init(pipe, name, flag, rb_memptr, size);
}
RTM_EXPORT(rt_pipe_create);

void rt_pipe_destroy(struct rt_pipe_device *pipe)
{
    if (pipe == RT_NULL)
        return;

    /* un-register pipe device */
    rt_pipe_detach(pipe);

    /* release memory */
    rt_free(pipe->ringbuffer.buffer_ptr);
    rt_free(pipe);

    return;
}
RTM_EXPORT(rt_pipe_destroy);
#endif /* RT_USING_HEAP */
