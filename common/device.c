/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : device.c
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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "device.h"
#include "list.h"
#include "debug.h"

static LIST_HEAD(devs_head);

/**
 * This function registers a device driver with specified name.
 *
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @param flags the flag of device
 *
 * @return the error code, ERROR_EOK on initialization successfully.
 */
int32_t device_register (device_t dev, const char *name, uint16_t flags)
{
	if (sizeof(name) > NAME_MAX)
		return ERROR_ERROR;
	if (name != NULL) {
		strcpy(dev->name, name);
		dev->flag = flags;
	}
	list_add_tail(&(dev->list), &devs_head);

	return ERROR_EOK;
}

/**
 * This function removes a previously registered device driver
 *
 * @param dev the pointer of device driver structure
 *
 * @return the error code, ERROR_EOK on successfully.
 */
int32_t device_unregister(const char *devname)
{
	device_t dev;

	ASSERT(devname != NULL);

	dev = device_find_by_name(devname);

	list_del(&(dev->list));

	return ERROR_EOK;
}


/**
 * This function initializes all registered device driver
 *
 * @return the error code, RT_EOK on successfully.
 */
int32_t device_init_all(void)
{
	struct device *dev;
	struct list_head *pos;
	int32_t (*init)(device_t dev);
	int32_t result = ERROR_EOK;

	list_for_each(pos, &devs_head) {
		dev = list_entry(pos, struct device, list);
		init = dev->init;
		if (init != NULL && !(dev->flag & DEVICE_FLAG_ACTIVATED)) {
			result = init(dev);
			dev->flag |= DEVICE_FLAG_ACTIVATED;
		}
	}

	return result;
}

/**
 * This function finds a device driver by specified name.
 *
 * @param name the device driver's name
 *
 * @return the registered device driver on successful, or NULL on failure.
 */
device_t device_find_by_name(const char* name)
{
	struct list_head *pos;
	device_t dev;

	if(!name)
		return NULL;

	list_for_each(pos, &devs_head) {
		dev = list_entry(pos, struct device, list);
		if(strcmp(dev->name, name) == 0)
			return dev;
	}

	return NULL;
}

/**
 * This function will initialize the specified device
 *
 * @param dev the pointer of device driver structure
 * 
 * @return the result
 */
int32_t device_init(device_t dev)
{
	int32_t result = ERROR_EOK;
	int32_t (*init)(device_t dev);

	ASSERT(dev != NULL);

	/* get device init handler */
	init = dev->init;
	if ((init != NULL) && !(dev->flag & DEVICE_FLAG_ACTIVATED)) {
		result = init(dev);
		dev->flag |= DEVICE_FLAG_ACTIVATED;
	} else {
		result = -ERROR_ENOSYS;
	}

	return result;
}


/**
 * This function will open a device
 *
 * @param dev the pointer of device driver structure
 * @param oflag the flags for device open
 *
 * @return the result
 */
int32_t device_open(device_t dev, uint16_t oflag)
{
	int32_t result;
	int32_t (*open)(device_t dev, uint16_t oflag);

	ASSERT(dev != NULL);

	result = ERROR_EOK;

	/* if device is not initialized, initialize it. */
	if (!(dev->flag & DEVICE_FLAG_ACTIVATED)) {
		result = dev->init(dev);
		if (result != ERROR_EOK) {
			return result;
		} else {
			dev->flag |= DEVICE_FLAG_ACTIVATED;
		}
	}

	/* device is a stand alone device and opened */
	if ((dev->flag & DEVICE_FLAG_STANDALONE) && (dev->open_flag & DEVICE_OFLAG_OPEN))
		return -ERROR_EBUSY;

	/* call device open interface */
	open = dev->open;
	if (open != NULL) {
		result = open(dev, oflag);
	} else {
		/* no this interface in device driver */
		result = -ERROR_ENOSYS;
	}

	/* set open flag */
	if (result == ERROR_EOK || result == -ERROR_ENOSYS)
		dev->open_flag = oflag | DEVICE_OFLAG_OPEN;

	return result;
}

/**
 * This function will close a device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
int32_t device_close(device_t dev)
{
	int32_t result;
	int32_t (*close)(device_t dev);

	ASSERT(dev != NULL);

	/* call device close interface */
	close = dev->close;
	if (close != NULL) {
		result = close(dev);
	} else {
		/* no this interface in device driver */
		result = -ERROR_ENOSYS;
	}

	/* set open flag */
	if (result == ERROR_EOK || result == -ERROR_ENOSYS)
		dev->open_flag = DEVICE_OFLAG_CLOSE;

	return result;
}

/**
 * This function will read some data from a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of reading
 * @param buffer the data buffer to save read data
 * @param size the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 *
 * @note since 0.4.0, the unit of size/pos is a block for block device.
 */
size_t device_read(device_t dev, uint32_t pos, void *buffer, size_t size)
{
	size_t (*read)(device_t dev, uint32_t pos, void *buffer, size_t size);

	ASSERT(dev != NULL);

	/* call device read interface */
	read = dev->read;
	if (read != NULL) {
		return read(dev, pos, buffer, size);
	}

	/* set error code */
	return ERROR_ENOSYS;
}

/**
 * This function will write some data to a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of written
 * @param buffer the data buffer to be written to device
 * @param size the size of buffer
 *
 * @return the actually written size on successful, otherwise negative returned.
 *
 * @note since 0.4.0, the unit of size/pos is a block for block device.
 */
size_t device_write(device_t dev, uint32_t pos, const void *buffer, size_t size)
{
	size_t (*write)(device_t dev, uint32_t pos, const void *buffer, size_t size);

	ASSERT(dev != NULL);

	/* call device write interface */
	write = dev->write;
	if (write != NULL) {
		return write(dev, pos, buffer, size);
	}

	/* set error code */
	return ERROR_ENOSYS;
}

/**
 * This function will perform a variety of control functions on devices.
 *
 * @param dev the pointer of device driver structure
 * @param cmd the command sent to device
 * @param arg the argument of command
 *
 * @return the result
 */
int32_t device_control(device_t dev, uint8_t cmd, void *arg)
{
	int32_t (*control)(device_t dev, uint8_t cmd, void *arg);

	ASSERT(dev != NULL);

	/* call device write interface */
	control = dev->control;
	if (control != NULL) {
		return control(dev, cmd, arg);
	}

	return -ERROR_ENOSYS;
}

/**
 * This function will set the indication callback function when device receives
 * data.
 *
 * @param dev the pointer of device driver structure
 * @param rx_ind the indication callback function
 *
 * @return RT_EOK
 */
int32_t device_set_rx_indicate(device_t dev, int32_t (*rx_ind)(device_t dev, int32_t size))
{
	ASSERT(dev != NULL);

	dev->rx_indicate = rx_ind;
	return ERROR_EOK;
}

/**
 * This function will set the indication callback function when device has written
 * data to physical hardware.
 *
 * @param dev the pointer of device driver structure
 * @param tx_done the indication callback function
 *
 * @return RT_EOK
 */
int32_t device_set_tx_complete(device_t dev, int32_t (*tx_done)(device_t dev, void *buffer))
{
	ASSERT(dev != NULL);

	dev->tx_complete = tx_done;
	return ERROR_EOK;
}

struct list_head* device_get_list(void)
{
	return &(devs_head);
}

