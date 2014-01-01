/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : usart.c
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
#include "os.h"
#include "usart.h"
#include "drivers/serial.h"

/* USART_RT1 */
#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_GPIO			GPIOA

/* USART_RT2 */
#define UART2_GPIO_TX	    GPIO_Pin_2
#define UART2_GPIO_RX	    GPIO_Pin_3
#define UART2_GPIO	    	GPIOA

/* USART_RT3_REMAP[1:0] = 00 */
#define UART3_GPIO_TX		GPIO_Pin_10
#define UART3_GPIO_RX		GPIO_Pin_11
#define UART3_GPIO			GPIOB

/* STM32 uart driver */
struct stm32_uart
{
    USART_TypeDef* uart_device;
    IRQn_Type irq;
};

static err_t stm32_configure(struct serial_device *serial, struct serial_configure *cfg)
{
    struct stm32_uart* uart;
    USART_InitTypeDef USART_InitStructure;

    ASSERT(serial != NULL);
    ASSERT(cfg != NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;

    USART_InitStructure.USART_BaudRate = cfg->baud_rate;

    if (cfg->data_bits == DATA_BITS_8)
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;

    if (cfg->stop_bits == STOP_BITS_1)
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    else if (cfg->stop_bits == STOP_BITS_2)
        USART_InitStructure.USART_StopBits = USART_StopBits_2;

    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart->uart_device, &USART_InitStructure);

    /* Enable USART_RT */
    USART_Cmd(uart->uart_device, ENABLE);
    /* enable interrupt */
    USART_ITConfig(uart->uart_device, USART_IT_RXNE, ENABLE);

    return ERROR_EOK;
}

static err_t stm32_control(struct serial_device *serial, int cmd, void *arg)
{
    struct stm32_uart* uart;

    ASSERT(serial != NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    switch (cmd) {
    case DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        UART_DISABLE_IRQ(uart->irq);
        break;
    case DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        UART_ENABLE_IRQ(uart->irq);
        break;
    }

    return ERROR_EOK;
}

static int stm32_putc(struct serial_device *serial, char c)
{
    struct stm32_uart* uart;

    ASSERT(serial != NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    while (!(uart->uart_device->SR & USART_FLAG_TXE));
    uart->uart_device->DR = c;

    return 1;
}

static int stm32_getc(struct serial_device *serial)
{
    int ch;
    struct stm32_uart* uart;

    ASSERT(serial != NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    ch = -1;
    if (uart->uart_device->SR & USART_FLAG_RXNE)
    {
        ch = uart->uart_device->DR & 0xff;
    }

    return ch;
}

static int stm32_poll(struct serial_device *serial)
{
    int size = 0;
    struct stm32_uart* uart;

    ASSERT(serial != NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    if ((serial->parent.flag & DEVICE_FLAG_INT_RX) == 0) {
        if (uart->uart_device->SR & USART_FLAG_RXNE) {
            size = 1;
        } else {
            size = 0;
        }
    }

    return size;
}

static const struct uart_ops stm32_uart_ops =
{
    stm32_configure,
    stm32_control,
    stm32_putc,
    stm32_getc,
    stm32_poll,
};

#if defined(CONFIG_USING_UART1)
/* UART1 device driver structure */
struct serial_ringbuf uart1_int_rx;
struct stm32_uart uart1 =
{
    USART1,
    USART1_IRQn,
};
struct serial_device serial1;

void USART1_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart1;

    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        hw_serial_isr(&serial1);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }
}
#endif /* USING_UART1 */

#if defined(CONFIG_USING_UART2)
/* UART1 device driver structure */
struct serial_ringbuf uart2_int_rx;
struct stm32_uart uart2 =
{
    USART2,
    USART2_IRQn,
};
struct serial_device serial2;

void USART_RT2_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart2;

    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        hw_serial_isr(&serial2);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }
}
#endif /* USING_UART2 */

#if defined(CONFIG_USING_UART3)
/* UART1 device driver structure */
struct serial_ringbuf uart3_int_rx;
struct stm32_uart uart3 =
{
    USART3,
    USART3_IRQn,
};
struct serial_device serial3;

void USART_RT3_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart3;

    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        hw_serial_isr(&serial3);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }
}
#endif /* USING_UART3 */

static void RCC_Configuration(void)
{
#ifdef CONFIG_USING_UART1
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif /* USING_UART1 */

#ifdef CONFIG_USING_UART2
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif /* USING_UART2 */

#ifdef CONFIG_USING_UART3
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif /* USING_UART3 */
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#ifdef CONFIG_USING_UART1
    /* Configure USART_RT Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
#endif /* USING_UART1 */

#ifdef CONFIG_USING_UART2
    /* Configure USART_RT Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);
#endif /* USING_UART2 */

#ifdef CONFIG_USING_UART3
    /* Configure USART_RT Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
#endif /* USING_UART3 */
}

static void NVIC_Configuration(struct stm32_uart* uart)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART_RT1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = uart->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    /* disable interrupt, for this will be re-config in device open */
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void hw_usart_init(void)
{
    struct stm32_uart* uart;
    struct serial_configure config = SERIAL_CONFIG_DEFAULT;

    RCC_Configuration();
    GPIO_Configuration();

#ifdef CONFIG_USING_UART1
    uart = &uart1;
    config.baud_rate = BAUD_RATE_115200;

    serial1.ops    = &stm32_uart_ops;
    serial1.int_rx = &uart1_int_rx;
    serial1.config = config;

    NVIC_Configuration(&uart1);

    /* register UART1 device */
    hw_serial_register(&serial1, "uart1",
                          DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX | DEVICE_FLAG_STREAM,
                          uart);
#endif /* USING_UART1 */

#ifdef CONFIG_USING_UART2
    uart = &uart2;

    config.baud_rate = BAUD_RATE_115200;
    serial2.ops    = &stm32_uart_ops;
    serial2.int_rx = &uart2_int_rx;
    serial2.config = config;

    NVIC_Configuration(&uart2);

    /* register UART1 device */
    hw_serial_register(&serial2, "uart2",
                          DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX,
                          uart);
#endif /* CONFIG_USING_UART2 */

#ifdef CONFIG_USING_UART3
    uart = &uart3;

    config.baud_rate = BAUD_RATE_115200;

    serial3.ops    = &stm32_uart_ops;
    serial3.int_rx = &uart3_int_rx;
    serial3.config = config;

    NVIC_Configuration(&uart3);

    /* register UART1 device */
    hw_serial_register(&serial3, "uart3",
                          DEVICE_FLAG_RDWR | DEVICE_FLAG_INT_RX,
                          uart);
#endif /* CONFIG_USING_UART3 */
}
/* put this function in driver init section */
EXPORT_BOARD_INIT(hw_usart_init)
