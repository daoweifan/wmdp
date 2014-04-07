/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : spi_core.h
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
#include <assert.h>
#include <string.h>
#include "os.h"
#include "spi.h"

extern err_t spi_bus_device_init(struct spi_bus *bus, const char *name);
extern err_t spidev_device_init(struct spi_device *dev, const char *name);

err_t spi_bus_register(struct spi_bus       *bus,
                             const char              *name,
                             const struct spi_ops *ops)
{
    err_t result;

    result = spi_bus_device_init(bus, name);
    if (result != ERROR_OK)
        return result;

    /* initialize mutex lock */
    //mutex_init(&(bus->lock), name, IPC_FLAG_FIFO);
    /* set ops */
    bus->ops = ops;
    /* initialize owner */
    bus->owner = NULL;

    return ERROR_OK;
}

err_t spi_bus_attach_device(struct spi_device *device,
                                  const char           *name,
                                  const char           *bus_name,
                                  void                 *user_data)
{
    err_t result;
    device_t bus;

    /* get physical spi bus */
    bus = device_find_by_name(bus_name);
    if (bus != NULL && bus->type == Device_Class_SPIBUS)
    {
        device->bus = (struct spi_bus *)bus;

        /* initialize spidev device */
        result = spidev_device_init(device, name);
        if (result != ERROR_OK)
            return result;

        memset(&device->config, 0, sizeof(device->config));
        device->parent.user_data = user_data;

        return ERROR_OK;
    }

    /* not found the host bus */
    return -ERROR_GENERIC;
}

err_t spi_configure(struct spi_device        *device,
                          struct spi_configuration *cfg)
{
    err_t result = ERROR_OK;
    int irq_state;

    assert(device != NULL);

    /* set configuration */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & SPI_MODE_MASK ;
    device->config.max_hz     = cfg->max_hz ;

    if (device->bus != NULL)
    {
        // result = mutex_take(&(device->bus->lock), WAITING_FOREVER);
        // DISABLE_INTERRUPTS();
        irq_state = ENTER_CRITICAL_SECTION();
        if (result == ERROR_OK)
        {
            if (device->bus->owner == device)
            {
                device->bus->ops->configure(device, &device->config);
            }

            /* release lock */
            // mutex_release(&(device->bus->lock));
        }
        // ENABLE_INTERRUPTS();
        LEAVE_CRITICAL_SECTION(irq_state);
    }

    return ERROR_OK;
}

err_t spi_send_then_send(struct spi_device *device,
                               const void           *send_buf1,
                               size_t             send_length1,
                               const void           *send_buf2,
                               size_t             send_length2)
{
    err_t result = ERROR_OK;
    int irq_state;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    // result = mutex_take(&(device->bus->lock), WAITING_FOREVER);
    // DISABLE_INTERRUPTS();
    irq_state = ENTER_CRITICAL_SECTION();
    if (result == ERROR_OK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == ERROR_OK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -ERROR_IO;
                goto __exit;
            }
        }

        /* send data1 */
        message.send_buf   = send_buf1;
        message.recv_buf   = NULL;
        message.length     = send_length1;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -ERROR_IO;
            goto __exit;
        }

        /* send data2 */
        message.send_buf   = send_buf2;
        message.recv_buf   = NULL;
        message.length     = send_length2;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -ERROR_IO;
            goto __exit;
        }

        result = ERROR_OK;
    }
    else
    {
        return -ERROR_IO;
    }
    

__exit:
    // ENABLE_INTERRUPTS();
    // mutex_release(&(device->bus->lock));
    LEAVE_CRITICAL_SECTION(irq_state);

    return result;
}

err_t spi_send_then_recv(struct spi_device *device,
                               const void           *send_buf,
                               size_t             send_length,
                               void                 *recv_buf,
                               size_t             recv_length)
{
    err_t result = ERROR_OK;
    int irq_state;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    // DISABLE_INTERRUPTS();
    irq_state = ENTER_CRITICAL_SECTION();
    // result = mutex_take(&(device->bus->lock), WAITING_FOREVER);
    if (result == ERROR_OK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == ERROR_OK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                result = -ERROR_IO;
                goto __exit;
            }
        }

        /* send data */
        message.send_buf   = send_buf;
        message.recv_buf   = NULL;
        message.length     = send_length;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -ERROR_IO;
            goto __exit;
        }

        /* recv data */
        message.send_buf   = NULL;
        message.recv_buf   = recv_buf;
        message.length     = recv_length;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = -ERROR_IO;
            goto __exit;
        }

        result = ERROR_OK;
    }
    else
    {
        return -ERROR_IO;
    }

__exit:
    // mutex_release(&(device->bus->lock));
    // ENABLE_INTERRUPTS();
    LEAVE_CRITICAL_SECTION(irq_state);

    return result;
}

size_t spi_transfer(struct spi_device *device,
                          const void           *send_buf,
                          void                 *recv_buf,
                          size_t             length)
{
    err_t result = ERROR_OK;
    int irq_state;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    // DISABLE_INTERRUPTS();
    irq_state = ENTER_CRITICAL_SECTION();
    // result = mutex_take(&(device->bus->lock), WAITING_FOREVER);
    if (result == ERROR_OK)
    {
        if (device->bus->owner != device)
        {
            /* not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == ERROR_OK)
            {
                /* set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* configure SPI bus failed */
                // wm_set_errno(-ERROR_IO);
                result = 0;
                goto __exit;
            }
        }

        /* initial message */
        message.send_buf   = send_buf;
        message.recv_buf   = recv_buf;
        message.length     = length;
        message.cs_take    = 1;
        message.cs_release = 1;
        message.next       = NULL;

        /* transfer message */
        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            // wm_set_errno(-ERROR_IO);
            goto __exit;
        }
    }
    else
    {
        // wm_set_errno(-ERROR_IO);

        return 0;
    }

__exit:
    // mutex_release(&(device->bus->lock));
    // ENABLE_INTERRUPTS();
    LEAVE_CRITICAL_SECTION(irq_state);

    return result;
}

struct spi_message *spi_transfer_message(struct spi_device  *device,
                                               struct spi_message *message)
{
    err_t result = ERROR_OK;
    int irq_state;
    struct spi_message *index;

    assert(device != NULL);

    /* get first message */
    index = message;
    if (index == NULL)
        return index;

    // DISABLE_INTERRUPTS();
    irq_state = ENTER_CRITICAL_SECTION();
    // result = mutex_take(&(device->bus->lock), WAITING_FOREVER);
    if (result != ERROR_OK)
    {
        // wm_set_errno(-ERROR_BUSY);

        return index;
    }

    /* reset errno */
    // wm_set_errno(ERROR_OK);

    /* configure SPI bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == ERROR_OK)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            // wm_set_errno(-ERROR_IO);
            result = 0;
            goto __exit;
        }
    }

    /* transmit each SPI message */
    while (index != NULL)
    {
        /* transmit SPI message */
        result = device->bus->ops->xfer(device, index);
        if (result == 0)
        {
            // wm_set_errno(-ERROR_IO);
            break;
        }

        index = index->next;
    }

__exit:
    /* release bus lock */
    // mutex_release(&(device->bus->lock));
    // ENABLE_INTERRUPTS();
    LEAVE_CRITICAL_SECTION(irq_state);

    return index;
}

err_t spi_take_bus(struct spi_device *device)
{
    err_t result = ERROR_OK;

    assert(device != NULL);
    assert(device->bus != NULL);

    // result = mutex_take(&(device->bus->lock), WAITING_FOREVER);
    if (result != ERROR_OK)
    {
        // wm_set_errno(-ERROR_BUSY);

        return -ERROR_BUSY;
    }

    /* reset errno */
    // wm_set_errno(ERROR_OK);

    /* configure SPI bus */
    if (device->bus->owner != device)
    {
        /* not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == ERROR_OK)
        {
            /* set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* configure SPI bus failed */
            // wm_set_errno(-ERROR_IO);
            /* release lock */
            // mutex_release(&(device->bus->lock));

            return -ERROR_IO;
        }
    }

    return result;
}

err_t spi_release_bus(struct spi_device *device)
{
    assert(device != NULL);
    assert(device->bus != NULL);
    assert(device->bus->owner == device);

    /* release lock */
    // mutex_release(&(device->bus->lock));

    return ERROR_OK;
}

err_t spi_take(struct spi_device *device)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_take = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}

err_t spi_release(struct spi_device *device)
{
    err_t result;
    struct spi_message message;

    assert(device != NULL);
    assert(device->bus != NULL);

    memset(&message, 0, sizeof(message));
    message.cs_release = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}
