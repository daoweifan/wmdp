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
#include "wmconfig.h"
#include "stm32_spi.h"

static err_t configure(struct spi_device* device, struct spi_configuration* configuration);
static uint32_t xfer(struct spi_device* device, struct spi_message* message);

static struct spi_ops stm32_spi_ops =
{
    configure,
    xfer
};

//------------------ DMA ------------------
#ifdef CONFIG_CONFIG_SPI_USE_DMA
static uint8_t dummy = 0xFF;
#endif

#ifdef CONFIG_SPI_USE_DMA
static void DMA_Configuration(struct stm32_spi_bus * stm32_spi_bus, const void * send_addr, void * recv_addr, size_t size)
{
    DMA_InitTypeDef DMA_InitStructure;

    DMA_ClearFlag(stm32_spi_bus->DMA_Channel_RX_FLAG_TC
                  | stm32_spi_bus->DMA_Channel_RX_FLAG_TE
                  | stm32_spi_bus->DMA_Channel_TX_FLAG_TC
                  | stm32_spi_bus->DMA_Channel_TX_FLAG_TE);

    /* RX channel configuration */
    DMA_Cmd(stm32_spi_bus->DMA_Channel_RX, DISABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(stm32_spi_bus->SPI->DR));
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_InitStructure.DMA_BufferSize = size;

    if(recv_addr != NULL)
    {
        DMA_InitStructure.DMA_MemoryBaseAddr = (u32) recv_addr;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    }
    else
    {
        DMA_InitStructure.DMA_MemoryBaseAddr = (u32) (&dummy);
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }

    DMA_Init(stm32_spi_bus->DMA_Channel_RX, &DMA_InitStructure);

    DMA_Cmd(stm32_spi_bus->DMA_Channel_RX, ENABLE);

    /* TX channel configuration */
    DMA_Cmd(stm32_spi_bus->DMA_Channel_TX, DISABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(stm32_spi_bus->SPI->DR));
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_InitStructure.DMA_BufferSize = size;

    if(send_addr != NULL)
    {
        DMA_InitStructure.DMA_MemoryBaseAddr = (u32)send_addr;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    }
    else
    {
        DMA_InitStructure.DMA_MemoryBaseAddr = (u32)(&dummy);;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    }

    DMA_Init(stm32_spi_bus->DMA_Channel_TX, &DMA_InitStructure);

    DMA_Cmd(stm32_spi_bus->DMA_Channel_TX, ENABLE);
}
#endif

inline uint16_t get_spi_BaudRatePrescaler(uint32_t max_hz)
{
    uint16_t SPI_BaudRatePrescaler;

    /* STM32F10x SPI MAX 18Mhz */
    if(max_hz >= SystemCoreClock/2 && SystemCoreClock/2 <= 18000000)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    }
    else if(max_hz >= SystemCoreClock/4)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    }
    else if(max_hz >= SystemCoreClock/8)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    }
    else if(max_hz >= SystemCoreClock/16)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    }
    else if(max_hz >= SystemCoreClock/32)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    }
    else if(max_hz >= SystemCoreClock/64)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;
    }
    else if(max_hz >= SystemCoreClock/128)
    {
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    }

    return SPI_BaudRatePrescaler;
}

static err_t configure(struct spi_device* device, struct spi_configuration* configuration)
{
    struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
    SPI_InitTypeDef SPI_InitStructure;

    SPI_StructInit(&SPI_InitStructure);

    /* data_width */
    if(configuration->data_width <= 8)
    {
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    }
    else if(configuration->data_width <= 16)
    {
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    }
    else
    {
        return ERROR_IO;
    }
    /* baudrate */
    SPI_InitStructure.SPI_BaudRatePrescaler = get_spi_BaudRatePrescaler(configuration->max_hz);
    /* CPOL */
    if(configuration->mode & SPI_CPOL)
    {
        #ifdef SPI_CPOL
        #undef SPI_CPOL
        #endif
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    }
    else
    {
        #ifdef SPI_CPOL
        #undef SPI_CPOL
        #endif
        SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    }
    /* CPHA */
    if(configuration->mode & SPI_CPHA)
    {
        #ifdef SPI_CPHA
        #undef SPI_CPHA
        #endif
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    }
    else
    {
        #ifdef SPI_CPHA
        #undef SPI_CPHA
        #endif
        SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    }
    /* MSB or LSB */
    if(configuration->mode & SPI_MSB)
    {
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    }
    else
    {
        SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    }
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS  = SPI_NSS_Soft;

    /* init SPI */
    SPI_I2S_DeInit(stm32_spi_bus->SPI);
    SPI_Init(stm32_spi_bus->SPI, &SPI_InitStructure);
    /* Enable SPI_MASTER */
    SPI_Cmd(stm32_spi_bus->SPI, ENABLE);
    SPI_CalculateCRC(stm32_spi_bus->SPI, DISABLE);

    return ERROR_OK;
};

static uint32_t xfer(struct spi_device* device, struct spi_message* message)
{
    struct stm32_spi_bus * stm32_spi_bus = (struct stm32_spi_bus *)device->bus;
    struct spi_configuration * config = &device->config;
    SPI_TypeDef * SPI = stm32_spi_bus->SPI;
    struct stm32_spi_cs * stm32_spi_cs = device->parent.user_data;
    uint32_t size = message->length;

    /* take CS */
    if(message->cs_take)
    {
        GPIO_ResetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
    }

#ifdef CONFIG_SPI_USE_DMA
    if(message->length > 32)
    {
        if(config->data_width <= 8)
        {
            DMA_Configuration(stm32_spi_bus, message->send_buf, message->recv_buf, message->length);
            SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);
            while (DMA_GetFlagStatus(stm32_spi_bus->DMA_Channel_RX_FLAG_TC) == RESET
                    || DMA_GetFlagStatus(stm32_spi_bus->DMA_Channel_TX_FLAG_TC) == RESET);
            SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, DISABLE);
        }
//        memcpy(buffer,_spi_flash_buffer,DMA_BUFFER_SIZE);
//        buffer += DMA_BUFFER_SIZE;
    }
    else
#endif
    {
        if(config->data_width <= 8)
        {
            const uint8_t * send_ptr = message->send_buf;
            uint8_t * recv_ptr = message->recv_buf;

            while(size--)
            {
                uint8_t data = 0xFF;

                if(send_ptr != NULL)
                {
                    data = *send_ptr++;
                }

                //Wait until the transmit buffer is empty
                while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
                // Send the byte
                SPI_I2S_SendData(SPI, data);

                //Wait until a data is received
                while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
                // Get the received data
                data = SPI_I2S_ReceiveData(SPI);

                if(recv_ptr != NULL)
                {
                    *recv_ptr++ = data;
                }
            }
        }
        else if(config->data_width <= 16)
        {
            const uint16_t * send_ptr = message->send_buf;
            uint16_t * recv_ptr = message->recv_buf;

            while(size--)
            {
                uint16_t data = 0xFF;

                if(send_ptr != NULL)
                {
                    data = *send_ptr++;
                }

                //Wait until the transmit buffer is empty
                while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);
                // Send the byte
                SPI_I2S_SendData(SPI, data);

                //Wait until a data is received
                while (SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);
                // Get the received data
                data = SPI_I2S_ReceiveData(SPI);

                if(recv_ptr != NULL)
                {
                    *recv_ptr++ = data;
                }
            }
        }
    }

    /* release CS */
    if(message->cs_release)
    {
        GPIO_SetBits(stm32_spi_cs->GPIOx, stm32_spi_cs->GPIO_Pin);
    }

    return message->length;
};

/** \brief init and register stm32 spi bus.
 *
 * \param SPI: STM32 SPI, e.g: SPI1,SPI2,SPI3.
 * \param stm32_spi: stm32 spi bus struct.
 * \param spi_bus_name: spi bus name, e.g: "spi1"
 * \return
 *
 */
static err_t stm32_spi_register(SPI_TypeDef * SPI,
                            struct stm32_spi_bus * stm32_spi,
                            const char * spi_bus_name)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    if(SPI == SPI1)
    {
    	stm32_spi->SPI = SPI1;
#ifdef CONFIG_SPI_USE_DMA
        /* Enable the DMA1 Clock */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        stm32_spi->DMA_Channel_RX = DMA1_Channel2;
        stm32_spi->DMA_Channel_TX = DMA1_Channel3;
        stm32_spi->DMA_Channel_RX_FLAG_TC = DMA1_FLAG_TC2;
        stm32_spi->DMA_Channel_RX_FLAG_TE = DMA1_FLAG_TE2;
        stm32_spi->DMA_Channel_TX_FLAG_TC = DMA1_FLAG_TC3;
        stm32_spi->DMA_Channel_TX_FLAG_TE = DMA1_FLAG_TE3;
#endif
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    }
    else if(SPI == SPI2)
    {
        stm32_spi->SPI = SPI2;
#ifdef CONFIG_SPI_USE_DMA
        /* Enable the DMA1 Clock */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

        stm32_spi->DMA_Channel_RX = DMA1_Channel4;
        stm32_spi->DMA_Channel_TX = DMA1_Channel5;
        stm32_spi->DMA_Channel_RX_FLAG_TC = DMA1_FLAG_TC4;
        stm32_spi->DMA_Channel_RX_FLAG_TE = DMA1_FLAG_TE4;
        stm32_spi->DMA_Channel_TX_FLAG_TC = DMA1_FLAG_TC5;
        stm32_spi->DMA_Channel_TX_FLAG_TE = DMA1_FLAG_TE5;
#endif
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    }
    else if(SPI == SPI3)
    {
    	stm32_spi->SPI = SPI3;
#ifdef CONFIG_SPI_USE_DMA
        /* Enable the DMA2 Clock */
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

        stm32_spi->DMA_Channel_RX = DMA2_Channel1;
        stm32_spi->DMA_Channel_TX = DMA2_Channel2;
        stm32_spi->DMA_Channel_RX_FLAG_TC = DMA2_FLAG_TC1;
        stm32_spi->DMA_Channel_RX_FLAG_TE = DMA2_FLAG_TE1;
        stm32_spi->DMA_Channel_TX_FLAG_TC = DMA2_FLAG_TC2;
        stm32_spi->DMA_Channel_TX_FLAG_TE = DMA2_FLAG_TE2;
#endif
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    }
    else
    {
        return ERROR_NOSYS;
    }

    return spi_bus_register(&stm32_spi->parent, spi_bus_name, &stm32_spi_ops);
}

#ifdef CONFIG_USING_SPI1
static struct stm32_spi_bus stm32_spi1_bus;
err_t stm32_spi1_register(const char * spi_bus_name)
{
	return stm32_spi_register(SPI1, &stm32_spi1_bus, spi_bus_name);
}
#endif /* #ifdef CONFIG_USING_SPI1 */

#ifdef CONFIG_USING_SPI2
static struct stm32_spi_bus stm32_spi2_bus;
err_t stm32_spi2_register(const char * spi_bus_name)
{
	return stm32_spi_register(SPI2, &stm32_spi2_bus, spi_bus_name);
}
#endif /* #ifdef CONFIG_USING_SPI2 */

#ifdef CONFIG_USING_SPI3
static struct stm32_spi_bus stm32_spi3_bus;
err_t stm32_spi3_register(const char * spi_bus_name)
{
	return stm32_spi_register(SPI3, &stm32_spi3_bus, spi_bus_name);
}
#endif /* #ifdef CONFIG_USING_SPI3 */
