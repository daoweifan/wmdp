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
#include "os.h"
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

#define rw_data_prepare()               write_cmd(34)
// #define rw_data_prepare()               
#define DISP_ORIENTATION                (1)

/* LCD is connected to the FSMC_Bank1_NOR/SRAM2 and NE2 is used as ship select signal */
/* RS <==> A2 */
#define LCD_REG              (*((volatile unsigned short *) 0x60000000)) /* RS = 0 */
#define LCD_RAM              (*((volatile unsigned short *) 0x60020000)) /* RS = 1 */

/* local function declarement */
static void LCD_FSMCConfig(void);
static void delay(unsigned int nCount);
static void LCD_RST(void);

/* local inline functions */
lcd_inline unsigned short read_data(void);
lcd_inline void write_cmd(unsigned short cmd);
lcd_inline void write_data(unsigned short data_code );
lcd_inline void write_reg(unsigned char reg_addr,unsigned short reg_val);

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
    /* FSMC Reset and back light configure */
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE , ENABLE);

        /* PE1 reset control pin */
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
        
        /* PC7, backlight control pin*/
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
    }
    /* lcm control pin init state */
    {
        GPIO_SetBits(GPIOD, GPIO_Pin_7);        //CS=1 
        GPIO_ResetBits(GPIOE, GPIO_Pin_1);      //RESET=0
        GPIO_SetBits(GPIOD, GPIO_Pin_4);        //RD=1
        GPIO_SetBits(GPIOD, GPIO_Pin_5);        //WR=1
        GPIO_SetBits(GPIOC, GPIO_Pin_7 );       //open the backlight
    }

    /*-- FSMC Configuration -------------------------------------------------*/
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &Timing_read;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &Timing_write;
    FSMC_NORSRAMStructInit(&FSMC_NORSRAMInitStructure);

    Timing_read.FSMC_AddressSetupTime = 0x02;             /* ��ַ����ʱ��  */
    Timing_read.FSMC_AddressHoldTime  = 0x00;             /* ��ַ����ʱ��  */
    Timing_read.FSMC_DataSetupTime = 0x05;                /* ���ݽ���ʱ��  */
    Timing_read.FSMC_AccessMode = FSMC_AccessMode_A;    /* FSMC ����ģʽ */

    Timing_write.FSMC_AddressSetupTime = 0x02;             /* ��ַ����ʱ��  */
    Timing_write.FSMC_AddressHoldTime  = 0x00;             /* ��ַ����ʱ��  */
    Timing_write.FSMC_DataSetupTime = 0x05;                /* ���ݽ���ʱ��  */
    Timing_write.FSMC_AccessMode = FSMC_AccessMode_A;   /* FSMC ����ģʽ */

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
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

static void delay(unsigned int nCount)
{
    for(; nCount != 0; nCount--);
}

static void LCD_RST(void)
{
    GPIO_ResetBits(GPIOE, GPIO_Pin_1);  //CS=0
    delay(0xffff);
    GPIO_SetBits(GPIOE, GPIO_Pin_1 );   //CS=1
    delay(0xffff);
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

static void lcd_SetCursor(unsigned int x,unsigned int y)
{
    write_reg(0x0020, x);    /* 0-239 */
    write_reg(0x0021, y);    /* 0-319 */
}

/* read gram rountine */
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
    for (index=0; index < (LCD_WIDTH*LCD_HEIGHT); index++) {
        write_data(Color);
    }
}

static void lcd_data_bus_test(void)
{
    unsigned short temp1, temp2;

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

    if ((temp1 == 0x5555) && (temp2 == 0xAAAA)) {
        printf(" data bus test pass!\n");
    } else {
        printf(" data bus test error: %04X %04X\n",temp1,temp2);
    }
}



void r61580_init(void)
{
    unsigned short deviceid=0;
    LCD_FSMCConfig();
    LCD_RST();

    deviceid = read_reg(0x00);

    /* deviceid check */
    if((deviceid == 0x1580) || (deviceid == 0xb505)) {
        printf("\r\nLCD Device ID : %04X ",deviceid);
    } else {
        printf("Invalid LCD ID:%04X\r\n",deviceid);
        printf("Please check you hardware and configure.\r\n");
        return;
    }

    // Synchronization after reset
    write_reg(0x0000, 0x0000);
    write_reg(0x0000, 0x0000);
    write_reg(0x0000, 0x0000);
    write_reg(0x0000, 0x0000);
    write_reg(0x00A4, 0x0001);  // CALB=1
    delay(50);
    // write_reg(0x0060, 0xA700);  // Driver Output Control
    write_reg(0x0060, 0x2700);  // Driver Output Control
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

    // delay(500000);   // Delay 50ms
    // lcd_clear(Red);  //clear screen
}

/*  set pixel color,X,Y */
static void r61580_set_pixel(const void *pixel, int x, int y)
{
    unsigned short *pp = (unsigned short *)pixel;
    lcd_SetCursor(x, y);
    write_data(*pp);
}

/* get pixel color */
static void r61580_get_pixel(void *pixel, int x, int y)
{
    unsigned short *pp = (unsigned short *)pixel;
    *pp = lcd_read_gram(x, y);
}

/* draw horizontal line */
static void r61580_draw_hline(const void *pixel, int x1, int x2, int y)
{
    unsigned short *pp = (unsigned short *)pixel;
    /* [5:4]-ID~ID0 [3]-AM, 1:vertical, 0:horizontal-0 */
    write_reg(0x0003,0x1030 | (0<<3)); // AM=0 hline
    lcd_SetCursor(x1, y);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (x1 < x2) {
        write_data(*pp);
        x1++;
    }
}

/* draw vertical line */
static void r61580_draw_vline(const void *pixel, int x, int y1, int y2)
{
    unsigned short *pp = (unsigned short *)pixel;
    /* [5:4]-ID~ID0 [3]-AM, 1:vertical, 0:horizontal-0 */
    write_reg(0x0003,0x1030 | (1<<3)); // AM=1 vline
    lcd_SetCursor(x, y1);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (y1 < y2) {
        write_data(*pp);
        y1++;
    }
}

/* fill rect area */
static void r61580_fill_rect(const void *pixel, int x0, int y0, int x1, int y1)
{
    while(y0 < y1) {
        r61580_draw_hline(pixel, x0, x1, y0);
        y0++;
    }
}


/* blit a line */
static void r61580_blit_line(const void* pixel, int x, int y, size_t size)
{
    uint16_t *pp = (uint16_t*)pixel;

    /* [5:4]-ID~ID0 [3]-AM, 1:vertical, 0:horizontal-0 */
    write_reg(0x0003,0x1030 | (0<<3)); // AM=0 hline

    lcd_SetCursor(x, y);
    rw_data_prepare(); /* Prepare to write GRAM */
    while (size) {
        write_data(*pp ++);
        size --;
    }
}

static struct device_graphic_ops r61580_ops =
{
	.set_pixel = r61580_set_pixel,
	.get_pixel = r61580_get_pixel,
	.draw_hline = r61580_draw_hline,
	.draw_vline = r61580_draw_vline,
	.fill_rect = r61580_fill_rect,
	.blit_line = r61580_blit_line
};

static struct device lcd_device;
static err_t lcd_init(device_t dev)
{
	return ERROR_EOK;
}

static err_t lcd_open(device_t dev, uint16_t oflag)
{
	return ERROR_EOK;
}

static err_t lcd_close(device_t dev)
{
	return ERROR_EOK;
}

static err_t lcd_control(device_t dev, uint8_t cmd, void *args)
{
	switch (cmd) {
	case GRAPHIC_CTRL_GET_INFO:
		{
			struct device_graphic_info *info;

			info = (struct device_graphic_info*) args;

			info->bits_per_pixel = 16;
			info->framebuffer = NULL;
			info->width = 240;
			info->height = 320;
		}
		break;

	case GRAPHIC_CTRL_RECT_UPDATE:
		/* nothong to be done */
		break;

	default:
		break;
	}

	return ERROR_EOK;
}

void hw_lcd_init(void)
{
	/* register lcd device */
	lcd_device.type  = Device_Class_Graphic;
	lcd_device.init  = lcd_init;
	lcd_device.open  = lcd_open;
	lcd_device.close = lcd_close;
	lcd_device.control = lcd_control;
	lcd_device.read  = NULL;
	lcd_device.write = NULL;

	lcd_device.user_data = &r61580_ops;
    r61580_init();

    /* register graphic device driver */
	device_register(&lcd_device, "r61580",
		DEVICE_FLAG_RDWR | DEVICE_FLAG_STANDALONE);
}
/* put this function in driver init section */
EXPORT_BOARD_INIT(hw_lcd_init)