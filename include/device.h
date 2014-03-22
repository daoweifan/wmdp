/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : device.h
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
#ifndef _DEVICES_H_
#define _DEVICES_H_

#include "def.h"
#include "list.h"

#define NAME_MAX 0x08

/**
 * device (I/O) class type
 */
enum device_class_type {
	Device_Class_Char = 0,                       /**< character device                           */
	Device_Class_Block,                          /**< block device                               */
	Device_Class_NetIf,                          /**< net interface                              */
	Device_Class_MTD,                            /**< memory device                              */
	Device_Class_CAN,                            /**< CAN device                                 */
	Device_Class_RTC,                            /**< RTC device                                 */
	Device_Class_Sound,                          /**< Sound device                               */
	Device_Class_Graphic,                        /**< Graphic device                             */
	Device_Class_I2C,                            /**< I2C device                                 */
	Device_Class_USBDevice,                      /**< USB slave device                           */
	Device_Class_USBHost,                        /**< USB host bus                               */
	Device_Class_SPIBUS, 						/**< SPI bus device                             */
	Device_Class_SPIDevice,                      /**< SPI device                                 */
	Device_Class_SDIO, 							/**< SDIO bus device                            */
	Device_Class_Unknown                         /**< unknown device                             */
};


/**
 * device flags defitions
 */
#define DEVICE_FLAG_DEACTIVATE       0x000       /**< device is not not initialized              */

#define DEVICE_FLAG_RDONLY           0x001       /**< read only                                  */
#define DEVICE_FLAG_WRONLY           0x002       /**< write only                                 */
#define DEVICE_FLAG_RDWR             0x003       /**< read and write                             */

#define DEVICE_FLAG_REMOVABLE        0x004       /**< removable device                           */
#define DEVICE_FLAG_STANDALONE       0x008       /**< standalone device                          */
#define DEVICE_FLAG_ACTIVATED        0x010       /**< device is activated                        */
#define DEVICE_FLAG_SUSPENDED        0x020       /**< device is suspended                        */
#define DEVICE_FLAG_STREAM           0x040       /**< stream mode                                */

#define DEVICE_FLAG_INT_RX           0x100       /**< INT mode on Rx                             */
#define DEVICE_FLAG_DMA_RX           0x200       /**< DMA mode on Rx                             */
#define DEVICE_FLAG_INT_TX           0x400       /**< INT mode on Tx                             */
#define DEVICE_FLAG_DMA_TX           0x800       /**< DMA mode on Tx                             */

#define DEVICE_OFLAG_CLOSE           0x000       /**< device is closed                           */
#define DEVICE_OFLAG_RDONLY          0x001       /**< read only access                           */
#define DEVICE_OFLAG_WRONLY          0x002       /**< write only access                          */
#define DEVICE_OFLAG_RDWR            0x003       /**< read and write                             */
#define DEVICE_OFLAG_OPEN            0x008       /**< device is opened                           */

/**
 * general device commands
 */
#define DEVICE_CTRL_RESUME           0x01        /**< resume device                              */
#define DEVICE_CTRL_SUSPEND          0x02        /**< suspend device                             */

/**
 * special device commands
 */
#define DEVICE_CTRL_CHAR_STREAM      0x10        /**< stream mode on char device                 */
#define DEVICE_CTRL_BLK_GETGEOME     0x10        /**< get geometry information                   */
#define DEVICE_CTRL_NETIF_GETMAC     0x10        /**< get mac address                            */
#define DEVICE_CTRL_MTD_FORMAT       0x10        /**< format a MTD device                        */
#define DEVICE_CTRL_RTC_GET_TIME     0x10        /**< get time                                   */
#define DEVICE_CTRL_RTC_SET_TIME     0x11        /**< set time                                   */

/**
 * graphic device control command
 */
#define GRAPHIC_CTRL_RECT_UPDATE      0
#define GRAPHIC_CTRL_POWERON          1
#define GRAPHIC_CTRL_POWEROFF         2
#define GRAPHIC_CTRL_GET_INFO         3
#define GRAPHIC_CTRL_SET_MODE         4
#define GRAPHIC_CTRL_GET_EXT          5

/**
 * graphic device information structure
 */
struct device_graphic_info
{
    uint8_t  bits_per_pixel;                         /**< bits per pixel */
    uint16_t reserved;                               /**< reserved field */

    uint16_t width;                                  /**< width of graphic device */
    uint16_t height;                                 /**< height of graphic device */

    uint8_t *framebuffer;                            /**< frame buffer */
};

/**
 * rectangle information structure
 */
struct device_rect_info
{
    uint16_t x;                                      /**< x coordinate */
    uint16_t y;                                      /**< y coordinate */
    uint16_t width;                                  /**< width */
    uint16_t height;                                 /**< height */
};

/**
 * graphic operations
 */
struct device_graphic_ops
{
    /* low level driver, not neccessary */
    void (*set_window) (int x0, int y0, int x1, int y1);
    void (*write_ram) (const void *pixel);

    void (*set_pixel) (const void *pixel, int x, int y);
    void (*get_pixel) (void *pixel, int x, int y);
    void (*draw_hline)(const void *pixel, int x1, int x2, int y);
    void (*draw_vline)(const void *pixel, int x, int y1, int y2);
    void (*fill_rect) (const void *pixel, int x0, int y0, int x1, int y1);
    void (*draw_rect) (const void *pixel, int x0, int y0, int xsize, int ysize);
    void (*blit_line) (const void *pixel, int x, int y, size_t size);
};
#define graphix_ops(device)          ((struct device_graphic_ops *)(device->user_data))


typedef struct device *device_t;
/**
 * Device structure
 */
struct device {
	char name[NAME_MAX];                      /**< name of kernel object      */
	enum device_class_type type;                 /**< device type                                */
	unsigned short flag, open_flag;                    /**< device flag and device open flag           */

	/* device call back */
	int32_t (*rx_indicate)(device_t dev, int32_t size);
	int32_t (*tx_complete)(device_t dev, void* buffer);

	/* common device interface */
	int32_t (*init)    (device_t dev);
	int32_t (*open)    (device_t dev, uint16_t oflag);
	int32_t (*close)   (device_t dev);
	size_t  (*read)    (device_t dev, uint32_t pos, void *buffer, size_t size);
	size_t  (*write)   (device_t dev, uint32_t pos, const void *buffer, size_t size);
	int32_t (*control) (device_t dev, unsigned char cmd, void *args);

#ifdef USING_DEVICE_SUSPEND
	int32_t (*suspend) (device_t dev);
	int32_t (*resumed) (device_t dev);
#endif

	void *user_data;                                /**< device private data */
	struct list_head list;
};

/* init head of device list table */
struct list_head* device_get_list(void);
int32_t devices_init (void);

device_t device_find_by_name(const char *name);
int32_t device_register(device_t dev, const char *name, uint16_t flags);
int32_t device_unregister(const char *devname);
int32_t device_init_all(void);

int32_t device_set_rx_indicate(device_t dev, int32_t (*rx_ind )(device_t dev, int32_t size));
int32_t device_set_tx_complete(device_t dev, int32_t (*tx_done)(device_t dev, void *buffer));

int32_t device_init(device_t dev);
int32_t device_open(device_t dev, uint16_t oflag);
int32_t device_close(device_t dev);
size_t device_read(device_t dev, uint32_t pos, void *buffer, size_t size);
size_t device_write(device_t dev, uint32_t pos, const void *buffer, size_t size);
int32_t device_control(device_t dev, uint8_t cmd, void *arg);

#endif /* _DEVICES_H_ */


