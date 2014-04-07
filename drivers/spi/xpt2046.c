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
#include "string.h"
#include "xpt2046.h"
#include "os.h"

#define X_MIN (0x0738)
#define X_MAX (0x00a0)
#define Y_MIN (0x0738)
#define Y_MAX (0x0118)

struct touch_device {
	struct device parent;
	struct spi_device *spi_dev;
	struct calibration_data cal_data;
	bool calibrating;
	uint16_t x, y;
	uint16_t width;
	uint16_t height;
};

static uint16_t xpt2046_GetXPosition(struct spi_device * dev);
static uint16_t xpt2046_GetYPosition(struct spi_device * dev);

static uint16_t xpt2046_GetXPosition(struct spi_device * dev)
{
	uint16_t RecX;
	uint8_t s_data[3], r_data[3];

	s_data[0] = 0xD0; // control byte, channel select
	s_data[1] = 0x0A; // receive data in last two bytes
	s_data[2] = 0x0A;
	spi_transfer(dev, s_data, r_data, 3); //first byte
	RecX = ((r_data[1] << 8) + r_data[2]) >> 4;
	RecX &= 0xFFF;

	return RecX;
}

static uint16_t xpt2046_GetYPosition(struct spi_device * dev)
{
	uint16_t RecY;
	uint8_t s_data[3], r_data[3];

	s_data[0] = 0x90; // control byte, channel select
	s_data[1] = 0x0A; // receive data in last two bytes
	s_data[2] = 0x0A;
	spi_transfer(dev, s_data, r_data, 3); //first byte
	RecY = ((r_data[1] << 8) + r_data[2]) >> 4;
	RecY &= 0xFFF;

	return RecY;
}


static err_t xpt2046_control(device_t dev, uint8_t cmd, void *args)
{
	struct touch_device *touch = (struct touch_device *)dev;
	struct calibration_data *data;
	uint16_t *yp;
	uint16_t *xp;

	switch (cmd) {
	case TOUCH_GOON_CALIBRATION:
		touch->calibrating = true;
		break;

	case TOUCH_GET_CALIBRATION_DATA:
		data = (struct calibration_data *)args;
		/* update */
		touch->cal_data.min_x = data->min_x;
		touch->cal_data.max_x = data->max_x;
		touch->cal_data.min_y = data->min_y;
		touch->cal_data.max_y = data->max_y;
		break;

	case TOUCH_GET_X_POSITION:
		xp = (uint16_t *)args;
		(*xp) = xpt2046_GetXPosition(touch->spi_dev);
		break;

	case TOUCH_GET_Y_POSITION:
		yp = (uint16_t *)args;
		(*yp) = xpt2046_GetYPosition(touch->spi_dev);
		break;

	default:
		break;
	}

	return ERROR_OK;
}

void xpt2046_hw_init(const char *spidev_name, const char *dev_name)
{
	/* config xpt2046 spi parameter */
	struct spi_configuration cfg;
	struct spi_device *xpt2046_spi;
	xpt2046_spi = (struct spi_device *)device_find_by_name(spidev_name);
	cfg.data_width = 8;
	cfg.mode = SPI_MSB; /* SPI Compatible Modes 0 */
	cfg.max_hz = 1000*400; /* 400kbit/s */
	spi_configure(xpt2046_spi, &cfg);

	/* create xpt2046 heap space*/
	struct touch_device *xpt2046_touch;
	xpt2046_touch = (struct touch_device *)malloc(sizeof(struct touch_device));
	if (xpt2046_touch == NULL) {
		OS_Set_Error(ERROR_ENOMEM);
		return;
	}
	
	/* clear device structure */
	memset(&(xpt2046_touch->parent), 0, sizeof(struct device));
	xpt2046_touch->spi_dev = xpt2046_spi;
	xpt2046_touch->calibrating = false;
	xpt2046_touch->cal_data.min_x = X_MIN;
	xpt2046_touch->cal_data.max_x = X_MAX;
	xpt2046_touch->cal_data.min_y = Y_MIN;
	xpt2046_touch->cal_data.max_y = Y_MAX;
	xpt2046_touch->width = 240;
	xpt2046_touch->height = 320;

	/* init device structure */
	xpt2046_touch->parent.type = Device_Class_Unknown;
	xpt2046_touch->parent.init = NULL;
	xpt2046_touch->parent.control = xpt2046_control;
	xpt2046_touch->parent.user_data = NULL;
	xpt2046_touch->parent.read = NULL;
	xpt2046_touch->parent.write = NULL;

	/* register touch device to RT-Thread */
	device_register(&(xpt2046_touch->parent), dev_name, DEVICE_FLAG_RDWR);
}
