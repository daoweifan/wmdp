import os

# toolchains options
ARCH='cortex-m3'
CPU='stm32f10x'
CROSS_TOOL='gcc'

if os.getenv('WM_CC'):
	CROSS_TOOL = os.getenv('WM_CC')

#device options
# STM32_TYPE = 
# 'STM32F10X_LD','STM32F10X_LD_VL',
# 'STM32F10X_MD','STM32F10X_MD_VL',
# 'STM32F10X_HD','STM32F10X_HD_VL',
# 'STM32F10X_XL','STM32F10X_CL'
STM32_TYPE = 'STM32F10X_HD'

# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example, CodeSourcery, Keil MDK, IAR

if  CROSS_TOOL == 'gcc':
	PLATFORM 	= 'gcc'
	# EXEC_PATH 	= 'C:/yagarto-20121222/bin'
	EXEC_PATH 	= 'C:/Sourcery_CodeBench_Lite_for_ARM_EABI/bin'
elif CROSS_TOOL == 'keil':
	PLATFORM 	= 'armcc'
	EXEC_PATH 	= 'C:/Keil'
elif CROSS_TOOL == 'iar':
	PLATFORM 	= 'iar'
	IAR_PATH 	= 'C:/Program Files (x86)/IAR Systems/Embedded Workbench 6.0'

if os.getenv('RTT_EXEC_PATH'):
	EXEC_PATH = os.getenv('RTT_EXEC_PATH')

# BUILD = 'release'
BUILD = 'debug'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX = 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    LINK = PREFIX + 'gcc'
    TARGET_EXT = 'axf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = ' -mcpu=cortex-m3 -mthumb -ffunction-sections -fdata-sections'
    CFLAGS = DEVICE
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp'
    LFLAGS = DEVICE + ' -Wall -nostartfiles -Wl,--gc-sections,-Map=wmdp.map,-cref,-u,Reset_Handler -T stm32f103_hd_rom.ld'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2 -g3 -DSTDIO_RETARGET_UART -std=c99'
        AFLAGS += ' -gdwarf-2 -g3'
    else:
        CFLAGS += ' -O2'

    POST_ACTION  = OBJCPY + ' -O binary $TARGET wmdp.bin\n'
    POST_ACTION += SIZE + ' $TARGET \n'
    # POST_ACTION += OBJDUMP + ' -h -S $TARGET'
elif PLATFORM == 'armcc':
    # toolchains
    CC = 'armcc'
    AS = 'armasm'
    AR = 'armar'
    LINK = 'armlink'
    TARGET_EXT = 'axf'

    DEVICE = ' --device DARMSTM'
    CFLAGS = DEVICE + ' --apcs=interwork'
    AFLAGS = DEVICE
    LFLAGS = DEVICE + ' --info sizes --info totals --info unused --info veneers --muldefweak --list wmdp.map --scatter stm32_rom.sct'

    CFLAGS += ' -I' + EXEC_PATH + '/ARM/RV31/INC'
    LFLAGS += ' --libpath ' + EXEC_PATH + '/ARM/RV31/LIB'

    EXEC_PATH += '/ARM/ARMCC/bin/'

    if BUILD == 'debug':
        CFLAGS += ' -g -O0'
        AFLAGS += ' -g'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = 'fromelf --bin $TARGET --output wmdp.bin \nfromelf -z $TARGET'

elif PLATFORM == 'iar':
    # toolchains
    CC = 'iccarm'
    AS = 'iasmarm'
    AR = 'iarchive'
    LINK = 'ilinkarm'
    TARGET_EXT = 'out'

    # DEVICE = ' -D USE_STDPERIPH_DRIVER' + ' -D STM32F10X_HD'
    DEVICE = ''

    CFLAGS = DEVICE
    CFLAGS += ' --diag_suppress Pa050'
    CFLAGS += ' --no_cse' 
    CFLAGS += ' --no_unroll' 
    CFLAGS += ' --no_inline' 
    CFLAGS += ' --no_code_motion' 
    CFLAGS += ' --no_tbaa' 
    CFLAGS += ' --no_clustering' 
    CFLAGS += ' --no_scheduling' 
    CFLAGS += ' --debug' 
    CFLAGS += ' --endian=little' 
    CFLAGS += ' --cpu=Cortex-M3' 
    CFLAGS += ' -e' 
    CFLAGS += ' --fpu=None'
    CFLAGS += ' --dlib_config "' + IAR_PATH + '/arm/INC/c/DLib_Config_Normal.h"'    
    CFLAGS += ' -Ol'    
    CFLAGS += ' --use_c++_inline'
        
    AFLAGS = ''
    AFLAGS += ' -s+' 
    AFLAGS += ' -w+' 
    AFLAGS += ' -r' 
    AFLAGS += ' --cpu Cortex-M3' 
    AFLAGS += ' --fpu None' 

    LFLAGS = ' --config stm32f103_hd.icf'
    LFLAGS += ' --redirect _Printf=_PrintfTiny' 
    LFLAGS += ' --redirect _Scanf=_ScanfSmall' 
    LFLAGS += ' --entry __iar_program_start'    

    EXEC_PATH = IAR_PATH + '/arm/bin/'
    POST_ACTION = ''
