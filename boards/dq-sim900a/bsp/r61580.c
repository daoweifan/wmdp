/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : r61580.c
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
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "r61580.h"

// Compatible list:
#ifdef __CC_ARM                         /* ARM Compiler */
    #define lcd_inline static __inline
#elif defined (__ICCARM__)              /* for IAR Compiler */
    #define lcd_inline inline
#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #define lcd_inline static __inline
#else
    #define lcd_inline static
#endif

// #define rw_data_prepare()               write_cmd(34)
#define rw_data_prepare()               
#define DISP_ORIENTATION                (0)

/* LCD is connected to the FSMC_Bank1_NOR/SRAM2 and NE2 is used as ship select signal */
/* RS <==> A2 */
#define LCD_REG              (*((volatile unsigned short *) 0x60000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x60020000)) /* RS = 1 */

static void LCD_FSMCConfig(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  Timing_read,Timing_write;

    /* FSMC GPIO configure */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF
                               | RCC_APB2Periph_GPIOG, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

        /*
        FSMC_D0 ~ FSMC_D3
        PD14 FSMC_D0   PD15 FSMC_D1   PD0  FSMC_D2   PD1  FSMC_D3
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /*
        FSMC_D4 ~ FSMC_D12
        PE7 ~ PE15  FSMC_D4 ~ FSMC_D12
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10
                                      | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_Init(GPIOE,&GPIO_InitStructure);

        /* FSMC_D13 ~ FSMC_D15   PD8 ~ PD10 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /*
        FSMC_A0 ~ FSMC_A5   FSMC_A6 ~ FSMC_A9
        PF0     ~ PF5       PF12    ~ PF15
        */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3
                                      | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_Init(GPIOF,&GPIO_InitStructure);

        /* FSMC_A10 ~ FSMC_A15  PG0 ~ PG5 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_Init(GPIOG,&GPIO_InitStructure);

        /* FSMC_A16 ~ FSMC_A18  PD11 ~ PD13 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /* RD-PD4 WR-PD5 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_Init(GPIOD,&GPIO_InitStructure);

        /* NBL0-PE0 NBL1-PE1 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
        GPIO_Init(GPIOE,&GPIO_InitStructure);

        /* NE1/NCE2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_Init(GPIOD,&GPIO_InitStructure);
        /* NE2 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
        /* NE3 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
        /* NE4 */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_Init(GPIOG,&GPIO_InitStructure);
    }
    /* FSMC GPIO configure */

    /*-- FSMC Configuration -------------------------------------------------*/
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &Timing_read;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &Timing_write;
    FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);

    Timing_read.FSMC_AddressSetupTime = 8;             /* 地址建立时间  */
    Timing_read.FSMC_AddressHoldTime  = 8;             /* 地址保持时间  */
    Timing_read.FSMC_DataSetupTime = 8;                /* 数据建立时间  */
    Timing_read.FSMC_AccessMode = FSMC_AccessMode_A;    /* FSMC 访问模式 */

    Timing_write.FSMC_AddressSetupTime = 8;             /* 地址建立时间  */
    Timing_write.FSMC_AddressHoldTime  = 8;             /* 地址保持时间  */
    Timing_write.FSMC_DataSetupTime = 8;                /* 数据建立时间  */
    Timing_write.FSMC_AccessMode = FSMC_AccessMode_A;   /* FSMC 访问模式 */

    /* Color LCD configuration ------------------------------------
       LCD configured as follow:
          - Data/Address MUX = Disable
          - Memory Type = SRAM
          - Data Width = 16bit
          - Write Operation = Enable
          - Extended Mode = Enable
          - Asynchronous Wait = Disable */
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

static void delay(int cnt)
{
    volatile unsigned int dl;
    while(cnt--) {
        for(dl=0; dl<500; dl++);
    }
}

static void lcd_port_init(void)
{
    LCD_FSMCConfig();
}

lcd_inline void write_cmd(unsigned short cmd)
{
    LCD_REG = cmd;
}

lcd_inline unsigned short read_data(void)
{
    return LCD_RAM;
}

lcd_inline void write_data(unsigned short data_code )
{
    LCD_RAM = data_code;
}

lcd_inline void write_reg(unsigned char reg_addr,unsigned short reg_val)
{
    write_cmd(reg_addr);
    write_data(reg_val);
}

lcd_inline unsigned short read_reg(unsigned char reg_addr)
{
    unsigned short val=0;
    write_cmd(reg_addr);
    val = read_data();
    return (val);
}

/********* control <只移植以上函数即可> ***********/

static unsigned short deviceid=0;//设置一个静态变量用来保存LCD的ID

//static unsigned short BGR2RGB(unsigned short c)
//{
//    u16  r, g, b, rgb;
//
//    b = (c>>0)  & 0x1f;
//    g = (c>>5)  & 0x3f;
//    r = (c>>11) & 0x1f;
//
//    rgb =  (b<<11) + (g<<5) + (r<<0);
//
//    return( rgb );
//}

static void lcd_SetCursor(unsigned int x,unsigned int y)
{
    write_reg(0x004e, x);    /* 0-239 */
    write_reg(0x004f, y);    /* 0-319 */
}

/* 读取指定地址的GRAM */
static unsigned short lcd_read_gram(unsigned int x,unsigned int y)
{
    unsigned short temp;
    lcd_SetCursor(x,y);
    rw_data_prepare();
    /* dummy read */
    temp = read_data();
    temp = read_data();
    return temp;
}

static void lcd_clear(unsigned short Color)
{
    unsigned int index=0;
    lcd_SetCursor(0,0);
    rw_data_prepare();                      /* Prepare to write GRAM */
    for (index=0; index<(LCD_WIDTH*LCD_HEIGHT); index++)
    {
        write_data(Color);
    }
}

static void lcd_data_bus_test(void)
{
    unsigned short temp1;
    unsigned short temp2;
//    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
//    write_reg(0x0003,(1<<12)|(1<<5)|(1<<4) | (0<<3) );

    /* wirte */
    lcd_SetCursor(0,0);
    rw_data_prepare();
    write_data(0x5555);

    lcd_SetCursor(1,0);
    rw_data_prepare();
    write_data(0xAAAA);

    /* read */
    lcd_SetCursor(0,0);
    temp1 = lcd_read_gram(0,0);
    temp2 = lcd_read_gram(1,0);

    if( (temp1 == 0x5555) && (temp2 == 0xAAAA) )
    {
        printf(" data bus test pass!\r\n");
    }
    else
    {
        printf(" data bus test error: %04X %04X\r\n",temp1,temp2);
    }
}

void r61580_init(void)
{
    lcd_port_init();
    deviceid = read_reg(0x00);

    /* deviceid check */
    if( deviceid != 0x1580 ) {
        printf("Invalid LCD ID:%04X\r\n",deviceid);
        printf("Please check you hardware and configure.\r\n");
    } else {
        printf("\r\nLCD Device ID : %04X ",deviceid);
    }

    // Synchronization after reset
    delay(2);
    write_reg(0x0000, 0x0000);
    write_reg(0x0000, 0x0000);
    write_reg(0x0000, 0x0000);
    write_reg(0x0000, 0x0000);

    // Setup display
    write_reg(0x00A4, 0x0001);  // CALB=1
    delay(2);
    write_reg(0x0060, 0xA700);  // Driver Output Control
    write_reg(0x0008, 0x0808);  // Display Control BP=8, FP=8
    write_reg(0x0030, 0x0111);  // y control
    write_reg(0x0031, 0x2410);  // y control
    write_reg(0x0032, 0x0501);  // y control
    write_reg(0x0033, 0x050C);  // y control
    write_reg(0x0034, 0x2211);  // y control
    write_reg(0x0035, 0x0C05);  // y control
    write_reg(0x0036, 0x2105);  // y control
    write_reg(0x0037, 0x1004);  // y control
    write_reg(0x0038, 0x1101);  // y control
    write_reg(0x0039, 0x1122);  // y control
    write_reg(0x0090, 0x0019);  // 80Hz
    write_reg(0x0010, 0x0530);  // Power Control
    write_reg(0x0011, 0x0237);
    write_reg(0x0012, 0x01BF);
    write_reg(0x0013, 0x1300);
    delay(100);

    write_reg(0x0001, 0x0100);
    write_reg(0x0002, 0x0200);
#if (DISP_ORIENTATION == 0)
    write_reg(0x0003,0x1030);
#else
    write_reg(0x0003,0x1038);
#endif
    write_reg(0x0009, 0x0001);
    write_reg(0x000A, 0x0008);
    write_reg(0x000C, 0x0001);
    write_reg(0x000D, 0xD000);
    write_reg(0x000E, 0x0030);
    write_reg(0x000F, 0x0000);
    write_reg(0x0020, 0x0000);
    write_reg(0x0021, 0x0000);
    write_reg(0x0029, 0x0077);
    write_reg(0x0050, 0x0000);
    write_reg(0x0051, 0xD0EF);
    write_reg(0x0052, 0x0000);
    write_reg(0x0053, 0x013F);
    write_reg(0x0061, 0x0001);
    write_reg(0x006A, 0x0000);
    write_reg(0x0080, 0x0000);
    write_reg(0x0081, 0x0000);
    write_reg(0x0082, 0x005F);
    write_reg(0x0093, 0x0701);
    write_reg(0x0007, 0x0100);
    write_reg(0x0022, 0x0000);

    //test, used to test the ram access whether success
    lcd_data_bus_test();

    //clear screen
    lcd_clear( Blue );
}

#if 0
/*  set pixel color,X,Y */
void r61580_lcd_set_pixel(const char* pixel, int x, int y)
{
    lcd_SetCursor(x,y);

    rw_data_prepare();
    write_data(*(uint16_t*)pixel);
}

/* get pixel color */
void r61580_lcd_get_pixel(char* pixel, int x, int y)
{
	*(uint16_t*)pixel = lcd_read_gram(x, y);
}

/* draw horizontal line */
void r61580_lcd_draw_hline(const char* pixel, int x1, int x2, int y)
{
    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
    write_reg(0x0011,0x6030 | (0<<3)); // AM=0 hline

    lcd_SetCursor(x1, y);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (x1 < x2)
    {
        write_data(*(rt_uint16_t*)pixel);
        x1++;
    }
}

/* draw vertical line */
void r61580_lcd_draw_vline(const char* pixel, int x, int y1, int y2)
{
    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
    write_reg(0x0011,0x6070 | (1<<3)); // AM=0 vline

    lcd_SetCursor(x, y1);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (y1 < y2)
    {
        write_data(*(rt_uint16_t*)pixel);
        y1++;
    }
}

/* blit a line */
void ssd1289_lcd_blit_line(const char* pixels, int x, int y, rt_size_t size)
{
	rt_uint16_t *ptr;

	ptr = (rt_uint16_t*)pixels;

    /* [5:4]-ID~ID0 [3]-AM-1垂直-0水平 */
    write_reg(0x0011,0x6070 | (0<<3)); // AM=0 hline

    lcd_SetCursor(x, y);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (size)
    {
        write_data(*ptr ++);
		size --;
    }
}

struct rt_device_graphic_ops ssd1289_ops =
{
	ssd1289_lcd_set_pixel,
	ssd1289_lcd_get_pixel,
	ssd1289_lcd_draw_hline,
	ssd1289_lcd_draw_vline,
	ssd1289_lcd_blit_line
};

struct rt_device lcd_device;
static rt_err_t lcd_init(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_open(rt_device_t dev, rt_uint16_t oflag)
{
	return RT_EOK;
}

static rt_err_t lcd_close(rt_device_t dev)
{
	return RT_EOK;
}

static rt_err_t lcd_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch (cmd)
	{
	case RTGRAPHIC_CTRL_GET_INFO:
		{
			struct rt_device_graphic_info *info;

			info = (struct rt_device_graphic_info*) args;
			RT_ASSERT(info != RT_NULL);

			info->bits_per_pixel = 16;
			info->pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565P;
			info->framebuffer = RT_NULL;
			info->width = 240;
			info->height = 320;
		}
		break;

	case RTGRAPHIC_CTRL_RECT_UPDATE:
		/* nothong to be done */
		break;

	default:
		break;
	}

	return RT_EOK;
}

void rt_hw_lcd_init(void)
{
    /* LCD RESET */
    /* PF10 : LCD RESET */
    {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_Init(GPIOF,&GPIO_InitStructure);

        GPIO_ResetBits(GPIOF,GPIO_Pin_10);
        GPIO_SetBits(GPIOF,GPIO_Pin_10);
        /* wait for lcd reset */
        rt_thread_delay(1);
    }

	/* register lcd device */
	_lcd_device.type  = RT_Device_Class_Graphic;
	_lcd_device.init  = lcd_init;
	_lcd_device.open  = lcd_open;
	_lcd_device.close = lcd_close;
	_lcd_device.control = lcd_control;
	_lcd_device.read  = RT_NULL;
	_lcd_device.write = RT_NULL;

	_lcd_device.user_data = &ssd1289_ops;
    ssd1289_init();

    /* register graphic device driver */
	rt_device_register(&_lcd_device, "lcd",
		RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
}
#endif

/************* LCD Module debug command **************/
#if 1
#include "command.h"
#include "os.h"

static int cmd_lcd_func(int argc, char *argv[])
{
	const char *usage = { \
		"usage:\n " \
		"lcd init         , initialize lcd module including FSMC and lcd controller\n " \
	};
	
	if(argc < 2) {
		printf(usage);
		return 0;
	}

	if (!strcmp(argv[1], "init")) {
		r61580_init();
		return 0;
	}

	return 0;
}
const cmd_t cmd_lcd = {"lcd", cmd_lcd_func, "LCD Module debug command"};
EXPORT_SHELL_CMD(cmd_lcd)
#endif

