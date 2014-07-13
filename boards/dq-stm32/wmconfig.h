/*
 * CPU - CONFIG
 */
#define CONFIG_CPU_STM32 1
#undef  CONFIG_CPU_LM3S
#undef  CONFIG_CPU_SAM3U

/*
 * CPU CONFIG - STM32
 */
#define CONFIG_HSE_VALUE              (8000000)
#define CONFIG_SYSCLK_VALUE           (72000000)
#define CONFIG_LSE_VALUE              (32768)
#undef  CONFIG_USE_HSI
#undef  CONFIG_STM32F10X_LD
#undef  CONFIG_STM32F10X_MD 
#define CONFIG_STM32F10X_HD 1
#undef  CONFIG_STM32F10X_CL
#define CONFIG_USE_STDPERIPH_DRIVER   1
#undef  CONFIG_USE_STM32_USB_DRIVER

#ifdef CONFIG_HSE_VALUE
	#define HSE_VALUE CONFIG_HSE_VALUE
#endif

/*
 * sys
 */
#define CONFIG_TICK_HZ (1000)

/*
 * driver serial
 */
#define CONFIG_USING_SERIAL          1
#define CONFIG_USING_UART1           1
#define CONFIG_USING_UART2           1
#define CONFIG_USING_UART3           1

/*
 * driver spi
 */
// #define CONFIG_USING_SPI             1
// #define CONFIG_USING_SPI1            1
// #define CONFIG_USING_SPI2            1
// #define CONFIG_USING_SPI3            1
// #define CONFIG_USING_XPT2046         1

/*
 * components
 */
#define CONFIG_USING_COMMON          1
#define CONFIG_CONSOLE_UART1         1
// #define CONFIG_CONSOLE_UART2         1
// #define CONFIG_CONSOLE_UART3         1

/*
 * components
 */
#define CONFIG_REDIRECT_STDIO       1

/*
 * components shell
 */
#define CONFIG_USING_SHELL          1
#define CONFIG_USING_SHELL_UART1    1
// #define CONFIG_USING_SHELL_UART2    1
// #define CONFIG_USING_SHELL_UART3    1

/*
 * components finsh
 */
// #define CONFIG_USING_FINSH          1
// #define CONFIG_USING_FINSH_UART1    1
// #define CONFIG_USING_FINSH_UART2    1
// #define CONFIG_USING_FINSH_UART3    1

/*
 * components ucgui
 */
// #define CONFIG_USING_UCGUI          1
// #define CONFIG_USING_UCGUI_R61580   1
// #define CONFIG_USING_UCGUI_OS      1
// #define CONFIG_USING_UCGUI_AntiAlias    1
// #define CONFIG_USING_UCGUI_ConvertColor    1
// #define CONFIG_USING_UCGUI_ConvertMono    1
// #define CONFIG_USING_UCGUI_JPEG    1
// #define CONFIG_USING_UCGUI_WM    1
// #define CONFIG_USING_UCGUI_Widget    1
// #define CONFIG_USING_UCGUI_MemDev    1
// #define CONFIG_USING_UCGUI_ILI9320  1
// #define CONFIG_USING_UCGUI_LCDMini2440  1

/*
 * components slre
 */
// #define CONFIG_USING_SLRE          1

/*
 * bsp
 */
// #define CONFIG_USING_SIM900A       1

/*
 * components newlib
 */
#define CONFIG_USING_NEWLIB           1
