/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : OS_MEM.C
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
#include "device.h"
#include "drivers/serial.h"

INLINE void serial_ringbuf_init(struct serial_ringbuf *rbuffer)
{
    memset(rbuffer->buffer, 0, sizeof(rbuffer->buffer));
    rbuffer->put_index = 0;
    rbuffer->get_index = 0;
}

INLINE void serial_ringbuf_putc(struct serial_ringbuf *rbuffer, char ch)
{
    rbuffer->buffer[rbuffer->put_index] = ch;
    rbuffer->put_index = (rbuffer->put_index + 1) & (CONFIG_SERIAL_RB_BUFSZ - 1);

    /* if the next position is read index, discard this 'read char' */
    if (rbuffer->put_index == rbuffer->get_index) {
        rbuffer->get_index = (rbuffer->get_index + 1) & (CONFIG_SERIAL_RB_BUFSZ - 1);
    }
}

INLINE int serial_ringbuf_putchar(struct serial_ringbuf *rbuffer, char ch)
{
    uint16_t next_index;

    next_index = (rbuffer->put_index + 1) & (CONFIG_SERIAL_RB_BUFSZ - 1);
    if (next_index != rbuffer->get_index) {
        rbuffer->buffer[rbuffer->put_index] = ch;
        rbuffer->put_index = next_index;
    } else {
        return -1;
    }

    return 1;
}

INLINE int serial_ringbuf_getc(struct serial_ringbuf *rbuffer)
{
    int ch;

    ch = -1;

    if (rbuffer->get_index != rbuffer->put_index)
    {
        ch = rbuffer->buffer[rbuffer->get_index];
        rbuffer->get_index = (rbuffer->get_index + 1) & (CONFIG_SERIAL_RB_BUFSZ - 1);
    }

    return ch;
}

INLINE uint32_t serial_ringbuf_size(struct serial_ringbuf *rbuffer)
{
    uint32_t size;

    size = (rbuffer->put_index - rbuffer->get_index) & (CONFIG_SERIAL_RB_BUFSZ - 1);

    return size;
}

/* Device Interface */

/*
 * This function initializes serial
 */
static err_t serial_init(struct device *dev)
{
    err_t result = ERROR_EOK;
    struct serial_device *serial;

    ASSERT(dev != 0);
    serial = (struct serial_device *)dev;

    if (!(dev->flag & DEVICE_FLAG_ACTIVATED))
    {
        /* apply configuration */
        if (serial->ops->configure)
            result = serial->ops->configure(serial, &serial->config);

        if (result != ERROR_EOK)
            return result;

        if (dev->flag & DEVICE_FLAG_INT_RX)
            serial_ringbuf_init(serial->int_rx);

        if (dev->flag & DEVICE_FLAG_INT_TX)
        {
			/* not supported yet */
			/*
            serial->ops->control(serial, DEVICE_CTRL_SET_INT, (void *)0);
            serial_ringbuf_init(serial->int_tx);
            serial->int_sending_flag = FALSE;
			*/
        }

        if (dev->flag & DEVICE_FLAG_DMA_TX)
        {
            // serial->dma_flag = FALSE;
            
            // /* init data queue */
            // rt_data_queue_init(&(serial->tx_dq), SERIAL_TX_DATAQUEUE_SIZE,
                               // SERIAL_TX_DATAQUEUE_LWM, 0);
        }

        /* set activated */
        dev->flag |= DEVICE_FLAG_ACTIVATED;
    }

    return result;
}

static err_t serial_open(struct device *dev, uint16_t oflag)
{
    struct serial_device *serial;
    uint32_t int_flags = 0;

    ASSERT(dev != 0);
    serial = (struct serial_device *)dev;

    if (dev->flag & DEVICE_FLAG_INT_RX)
        int_flags = SERIAL_RX_INT;
    if (dev->flag & DEVICE_FLAG_INT_TX)
        int_flags |= SERIAL_TX_INT;

    if (int_flags) {
        serial->ops->control(serial, DEVICE_CTRL_SET_INT, (void *)int_flags);
    } else {
        serial->ops->control(serial, DEVICE_CTRL_CLR_INT, (void *)int_flags);
    }

    return ERROR_EOK;
}

static err_t serial_close(struct device *dev)
{
    struct serial_device *serial;
    uint32_t int_flags = 0;

    ASSERT(dev != 0);
    serial = (struct serial_device *)dev;

    if (dev->flag & DEVICE_FLAG_INT_RX)
        int_flags = SERIAL_RX_INT;
    if (dev->flag & DEVICE_FLAG_INT_TX)
        int_flags |= SERIAL_TX_INT;

    if (int_flags)
    {
        serial->ops->control(serial, DEVICE_CTRL_CLR_INT, (void *)int_flags);
    }

    return ERROR_EOK;
}

static size_t serial_read(struct device *dev,
                                off_t          pos,
                                void             *buffer,
                                size_t         size)
{
    uint8_t *ptr;
    uint32_t read_nbytes;
    struct serial_device *serial;

    ASSERT(dev != 0);

    if (size == 0)
        return 0;

    serial = (struct serial_device *)dev;

    ptr = (uint8_t *)buffer;

    if (dev->flag & DEVICE_FLAG_INT_RX)
    {
        /* interrupt mode Rx */
        while (size)
        {
            int ch;

            if (serial_ringbuf_size(serial->int_rx) == 0)
                break;
            ch = serial_ringbuf_getc(serial->int_rx);

            *ptr = ch & 0xff;
            ptr ++;
            size --;
        }
    }
    else
    {
        /* polling mode */
        while ((uint32_t)ptr - (uint32_t)buffer < size)
        {
            if (serial->ops->poll(serial) == 0)
                break;
            *ptr = serial->ops->getc(serial);
            ptr ++;
        }
    }

    read_nbytes = (uint32_t)ptr - (uint32_t)buffer;
    /* set error code */
    if (read_nbytes == 0)
    {
        // rt_set_errno(-ERROR_EEMPTY);
    }

    return read_nbytes;
}

static size_t serial_write(struct device *dev,
                                 off_t          pos,
                                 const void       *buffer,
                                 size_t         size)
{
    uint8_t *ptr;
    size_t write_nbytes = 0;
    struct serial_device *serial;

    ASSERT(dev != 0);

    if (size == 0)
        return 0;

    serial = (struct serial_device *)dev;

    ptr = (uint8_t*)buffer;

    if (dev->flag & DEVICE_FLAG_INT_TX)
    {
        /* warning: data will be discarded if buffer is full */
        while (size)
        {
            if (serial_ringbuf_putchar(serial->int_tx, *ptr) != -1)
            {
                ptr ++;
                size --;
            }
            else
                break;
        }
    }
    else if (dev->flag & DEVICE_FLAG_DMA_TX)
    {
        // rt_base_t level;
        // err_t result;
        
        // ASSERT(0 == (dev->flag & DEVICE_FLAG_STREAM));

        // result = rt_data_queue_push(&(serial->tx_dq), buffer, size, 20); 
        // if (result == ERROR_EOK)
        // {
            // level = rt_hw_interrupt_disable();
            // if (serial->dma_flag == FALSE)
            // {
                // serial->dma_flag = TRUE;
                // rt_hw_interrupt_enable(level);
                // serial->ops->dma_transmit(serial, buffer, size);
            // }
            // else
                // rt_hw_interrupt_enable(level);

            // return size;
        // }
        // else
        // {
            // rt_set_errno(result);

            // return 0;
        // }
    }
    else
    {
        /* polling mode */
        while (size)
        {
            /*
             * to be polite with serial console add a line feed
             * to the carriage return character
             */
            if (*ptr == '\n' && (dev->flag & DEVICE_FLAG_STREAM))
            {
                serial->ops->putc(serial, '\r');
            }

            serial->ops->putc(serial, *ptr);

            ++ ptr;
            -- size;
        }
    }

    write_nbytes = (uint32_t)ptr - (uint32_t)buffer;
    if (write_nbytes == 0)
    {
        // rt_set_errno(-ERROR_EFULL);
    }

    return write_nbytes;
}

static err_t serial_control(struct device *dev,
                                  uint8_t        cmd,
                                  void             *args)
{
    struct serial_device *serial;

    ASSERT(dev != 0);
    serial = (struct serial_device *)dev;

    switch (cmd)
    {
    case DEVICE_CTRL_SUSPEND:
        /* suspend device */
        dev->flag |= DEVICE_FLAG_SUSPENDED;
        break;

    case DEVICE_CTRL_RESUME:
        /* resume device */
        dev->flag &= ~DEVICE_FLAG_SUSPENDED;
        break;

    case DEVICE_CTRL_CONFIG:
        /* configure device */
        serial->ops->configure(serial, (struct serial_configure *)args);
        break;
    }

    return ERROR_EOK;
}

/*
 * serial register
 */
err_t hw_serial_register(struct serial_device *serial,
                               const char              *name,
                               uint32_t              flag,
                               void                    *data)
{
    struct device *device;
    ASSERT(serial != 0);

    device = &(serial->parent);

    device->type        = Device_Class_Char;
    device->rx_indicate = 0;
    device->tx_complete = 0;

    device->init        = serial_init;
    device->open        = serial_open;
    device->close       = serial_close;
    device->read        = serial_read;
    device->write       = serial_write;
    device->control     = serial_control;
    device->user_data   = data;

    /* register a character device */
    return device_register(device, name, flag);
}

/* ISR for serial interrupt */
void hw_serial_isr(struct serial_device *serial)
{
    int ch = -1;

    /* interrupt mode receive */
    ASSERT(serial->parent.flag & DEVICE_FLAG_INT_RX);

    while (1)
    {
        ch = serial->ops->getc(serial);
        if (ch == -1)
            break;

        serial_ringbuf_putc(serial->int_rx, ch);
    }

    /* invoke callback */
    if (serial->parent.rx_indicate != 0)
    {
        size_t rx_length;

        /* get rx length */
        rx_length = serial_ringbuf_size(serial->int_rx);
        serial->parent.rx_indicate(&serial->parent, rx_length);
    }
}

/*
 * ISR for DMA mode Tx
 */
void hw_serial_dma_tx_isr(struct serial_device *serial)
{
    // const void *data_ptr;
    // size_t data_size;
    // const void *last_data_ptr;

    // data_queue_pop(&(serial->tx_dq), &last_data_ptr, &data_size, 0);
    // if (ERROR_EOK == rt_data_queue_peak(&(serial->tx_dq), &data_ptr, &data_size))
    // {
        // /* transmit next data node */
         // serial->ops->dma_transmit(serial, data_ptr, data_size);
    // }
    // else
    // {
        // serial->dma_flag = FALSE;
    // }

    // /* invoke callback */
    // if (serial->parent.tx_complete != 0)
    // {
        // serial->parent.tx_complete(&serial->parent, (void*)last_data_ptr);
    // }
}
