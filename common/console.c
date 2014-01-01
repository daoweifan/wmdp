/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : console.c
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
#include <stdio.h>
#include "debug.h"
#include "common/console.h"

static device_t _console_device = WM_NULL;
static device_t _console_default = WM_NULL;

/**
 * This function will set a device as console device.
 * After set a device to console, all output of rt_kprintf will be
 * redirected to this new device.
 *
 * @param name the name of new console device
 *
 * @return the old console device handler
 */
device_t console_init(const char *name)
{
	device_t dev;

	/* find new console device */
	dev = device_find_by_name(name);
	if (dev != WM_NULL) {
		/* set new console device */
		_console_device = dev;
		_console_default = dev;
		device_open(_console_device, DEVICE_OFLAG_RDWR);
	}

	return dev;
}

/**
 * This function will show the version of rt-thread rtos
 */
void console_show_version(void)
{
	console_printf("\n \\ | /\n");
	console_printf("- WMDP -     WM Develop Platform\n");
	console_printf(" 2006 - 2011 Copyright by WM team\n");
}


/**
 * This function returns the device using in console.
 *
 * @return the device using in console or RT_NULL
 */
device_t console_get_device(void)
{
	return _console_device;
}

/**
 * This function will set a device as console device.
 * After set a device to console, all output of rt_kprintf will be
 * redirected to this new device.
 *
 * @param name the name of new console device
 *
 * @return the old console device handler
 */
device_t console_set_by_name(const char *name)
{
	device_t new, old;

	/* save old device */
	old = _console_device;

	/* find new console device */
	new = device_find_by_name(name);
	if (new != WM_NULL) {
		if (_console_device != WM_NULL) {
			/* close old console device */
			device_close(_console_device);
		}

		/* set new console device */
		_console_device = new;
		device_open(_console_device, DEVICE_OFLAG_RDWR);
	}

	return old;
}

/**
 * This function will set a device as console device.
 * After set a device to console, all output of printf will be
 * redirected to this new device.
 *
 * @param dev new console device
 *
 * @return the old console device handler
 */
device_t console_set_by_device(device_t dev)
{
	device_t old;

	/* save old device */
	old = _console_device;

	if (dev != WM_NULL) {
		if (_console_device != WM_NULL) {
			/* close old console device */
			device_close(_console_device);
		}

		/* set new console device */
		_console_device = dev;
		device_open(_console_device, DEVICE_OFLAG_RDWR);
	}

	return old;
}

/**
 * This function will switch console device.
 * After set a device to console, all output of printf will be
 * redirected to this switched device.
 *
 * @param dev new console device
 *
 * @return the old console device handler
 */
device_t console_switch_device(device_t dev)
{
	device_t old;

	/* save old device */
	old = _console_device;

	if (dev != WM_NULL) {
		_console_device = dev;
	} else {
		_console_device = _console_default;
	}

	return old;
}


size_t console_read(void *buffer, size_t size)
{
	ASSERT(_console_device);
	return device_read(_console_device, 0, buffer, size);
}

size_t console_write(const void *buffer, size_t size)
{
	ASSERT(_console_device);
	return device_write(_console_device, 0, buffer, size);
}

/**
 * This function will print a formatted string on system console
 *
 * @param fmt the format
 */
void console_printf(const char *fmt, ...)
{
	static char log_buf[64];
	int length;
	va_list ap;

	va_start(ap, fmt);
	length = vsprintf(log_buf, fmt, ap);//Msg formate
	ASSERT(_console_device);
	device_write(_console_device, 0, log_buf, length);
	va_end(ap);
}
