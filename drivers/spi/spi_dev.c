/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : spi_dev.h
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

#include "device.h"
#include "spi.h"

#define off_t          uint32_t

/* SPI bus device interface, compatible with RT-Thread 0.3.x/1.0.x */
static err_t _spi_bus_device_init(device_t dev)
{
    struct spi_bus *bus;

    bus = (struct spi_bus *)dev;
    assert(bus != NULL);

    return ERROR_OK;
}

static size_t _spi_bus_device_read(device_t dev,
                                      off_t    pos,
                                      void       *buffer,
                                      size_t   size)
{
    struct spi_bus *bus;

    bus = (struct spi_bus *)dev;
    assert(bus != NULL);
    assert(bus->owner != NULL);

    return spi_transfer(bus->owner, NULL, buffer, size);
}

static size_t _spi_bus_device_write(device_t dev,
                                       off_t    pos,
                                       const void *buffer,
                                       size_t   size)
{
    struct spi_bus *bus;

    bus = (struct spi_bus *)dev;
    assert(bus != NULL);
    assert(bus->owner != NULL);

    return spi_transfer(bus->owner, buffer, NULL, size);
}

static err_t _spi_bus_device_control(device_t dev,
                                        uint8_t  cmd,
                                        void       *args)
{
    struct spi_bus *bus;

    bus = (struct spi_bus *)dev;
    assert(bus != NULL);

    switch (cmd)
    {
    case 0: /* set device */
        break;
    case 1: 
        break;
    }

    return ERROR_OK;
}

err_t spi_bus_device_init(struct spi_bus *bus, const char *name)
{
    struct device *device;
    assert(bus != NULL);

    device = &bus->parent;

    /* set device type */
    device->type    = Device_Class_SPIBUS;
    /* initialize device interface */
    device->init    = _spi_bus_device_init;
    device->open    = NULL;
    device->close   = NULL;
    device->read    = _spi_bus_device_read;
    device->write   = _spi_bus_device_write;
    device->control = _spi_bus_device_control;

    /* register to device manager */
    return device_register(device, name, DEVICE_FLAG_RDWR);
}

/* SPI Dev device interface, compatible with RT-Thread 0.3.x/1.0.x */
static err_t _spidev_device_init(device_t dev)
{
    struct spi_device *device;

    device = (struct spi_device *)dev;
    assert(device != NULL);

    return ERROR_OK;
}

static size_t _spidev_device_read(device_t dev,
                                     off_t    pos,
                                     void       *buffer,
                                     size_t   size)
{
    struct spi_device *device;

    device = (struct spi_device *)dev;
    assert(device != NULL);
    assert(device->bus != NULL);

    return spi_transfer(device, NULL, buffer, size);
}

static size_t _spidev_device_write(device_t dev,
                                      off_t    pos,
                                      const void *buffer,
                                      size_t   size)
{
    struct spi_device *device;

    device = (struct spi_device *)dev;
    assert(device != NULL);
    assert(device->bus != NULL);

    return spi_transfer(device, buffer, NULL, size);
}

static err_t _spidev_device_control(device_t dev,
                                       uint8_t  cmd,
                                       void       *args)
{
    struct spi_device *device;

    device = (struct spi_device *)dev;
    assert(device != NULL);

    switch (cmd)
    {
    case 0: /* set device */
        break;
    case 1: 
        break;
    }

    return ERROR_OK;
}

err_t spidev_device_init(struct spi_device *dev, const char *name)
{
    struct device *device;
    assert(dev != NULL);

    device = &(dev->parent);

    /* set device type */
    device->type    = Device_Class_SPIDevice;
    device->init    = _spidev_device_init;
    device->open    = NULL;
    device->close   = NULL;
    device->read    = _spidev_device_read;
    device->write   = _spidev_device_write;
    device->control = _spidev_device_control;
    
    /* register to device manager */
    return device_register(device, name, DEVICE_FLAG_RDWR);
}
