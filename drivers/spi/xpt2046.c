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
#include "linux/sort.h"

#define X_MIN (0x0738)
#define X_MAX (0x00a0)
#define Y_MIN (0x0738)
#define Y_MAX (0x0118)

#define START_BIT         (1 << 7)
#define CHS_TMP0          (0x00 << 4 | 1 << 2)
#define CHS_POSY          (0x01 << 4 | 1 << 2)
#define CHS_VBAT          (0x02 << 4 | 1 << 2)
#define CHS_PRZ1          (0x03 << 4 | 1 << 2)
#define CHS_PRZ2          (0x04 << 4 | 1 << 2)
#define CHS_POSX          (0x05 << 4 | 1 << 2)
#define CHS_AUXI          (0x06 << 4 | 1 << 2)
#define CHS_TMP1          (0x07 << 4 | 1 << 2)
#define CHD_POSY          (0x01 << 4 | 0 << 2)
#define CHD_PRZ1          (0x03 << 4 | 0 << 2)
#define CHD_PRZ2          (0x04 << 4 | 0 << 2)
#define CHD_POSX          (0x05 << 4 | 0 << 2)
#define ADC_08BIT         (1 << 3)
#define ADC_12BIT         (0 << 3)
#define PD_ON             (0x00)
#define PD_REF_OFF_ADC_ON (0x01)
#define PD_REF_ON_ADC_OFF (0x02)
#define PD_OFF            (0x03)

/* command buffer */
const static char z1_cmd[] = {START_BIT | CHD_PRZ1 | ADC_12BIT | PD_OFF, 0x00, 0x00 /*measure pos Z1*/};
const static char x_cmd[]  = {START_BIT | CHD_POSX | ADC_12BIT | PD_ON, 0x0a, 0x0a /*measure pos X */};
const static char y_cmd[]  = {START_BIT | CHD_POSY | ADC_12BIT | PD_ON, 0x0a, 0x0a /*measure pos Y */};
const static char z2_cmd[] = {START_BIT | CHD_PRZ2 | ADC_12BIT | PD_OFF, 0x00, 0x00 /*measure pos Z2*/};
//const static char z1_cmd[] = {START_BIT | CHD_PRZ1 | ADC_12BIT | PD_OFF, 0x00, 0x00 /*measure pos Z1*/};

struct touch_device {
	struct device parent;
	struct spi_device *spi_dev;
	struct calibration_data cal_data;
	bool calibrating;
	uint16_t x, y;
	uint16_t width;
	uint16_t height;
};

static uint16_t xpt2046_GetInfomation(const char * cmd, struct spi_device * dev);

static int sort_info(const void* a, const void *b)
{
	return (*(uint16_t *)a - *(uint16_t *)b);
}

static uint16_t xpt2046_GetInfomation(const char * cmd, struct spi_device * dev)
{
	int i;
	uint16_t info, sa_array[5];
	uint8_t r_data[3];

	// s_data[0] = 0xD0; // control byte, channel select
	// s_data[1] = 0x0A; // receive data in last two bytes
	// s_data[2] = 0x0A;
	for(i = 0; i < 5; i++) {
		spi_transfer(dev, cmd, r_data, 3); //first byte
		sa_array[i] = ((uint16_t)(r_data[1] << 8) + r_data[2]) >> 4;
	}
	sort(sa_array, 5, 2, sort_info , 0);
	/* fetch the middle one */
	info = sa_array[2];
	info &= 0xFFF;

	return info;
}

static err_t xpt2046_control(device_t dev, uint8_t cmd, void *args)
{
	struct touch_device *touch = (struct touch_device *)dev;
	struct calibration_data *data;
	uint16_t *info;

	switch (cmd) {
	case TOUCH_GOON_CALIBRATION:
		touch->calibrating = true;
		break;

	case TOUCH_GET_CALIBRATION_DATA:
		data = (struct calibration_data *)args;
		/* update */
		data->min_x = touch->cal_data.min_x;
		data->max_x = touch->cal_data.max_x;
		data->min_y = touch->cal_data.min_y;
		data->max_y = touch->cal_data.max_y;
		break;

	case TOUCH_SET_CALIBRATION_DATA:
		data = (struct calibration_data *)args;
		/* update */
		touch->cal_data.min_x = data->min_x;
		touch->cal_data.max_x = data->max_x;
		touch->cal_data.min_y = data->min_y;
		touch->cal_data.max_y = data->max_y;
		break;

	case TOUCH_GET_X_POSITION:
		info = (uint16_t *)args;
		(*info) = xpt2046_GetInfomation(x_cmd, touch->spi_dev);
		break;

	case TOUCH_GET_Y_POSITION:
		info = (uint16_t *)args;
		(*info) = xpt2046_GetInfomation(y_cmd, touch->spi_dev);
		break;

	case TOUCH_GET_Z1_POSITION:
		info = (uint16_t *)args;
		(*info) = xpt2046_GetInfomation(z1_cmd, touch->spi_dev);
		break;

	case TOUCH_GET_Z2_POSITION:
		info = (uint16_t *)args;
		(*info) = xpt2046_GetInfomation(z2_cmd, touch->spi_dev);
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
	cfg.mode = SPI_MSB | SPI_MODE_3; /* SPI Compatible Modes 0 */
	cfg.max_hz = 1000*1000; /* 1000kbit/s */
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
