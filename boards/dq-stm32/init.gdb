# connect to the J-Link gdb server
target remote localhost:2331

# Enable flash download and flash breakpoints.
# Flash download and flash breakpoints are features of
# the J-Link software which require separate licenses 
# from SEGGER.

# Select flash device
monitor flash device = STM32F103ZE

# Enable FlashDL and FlashBPs
monitor flash download = 1
monitor flash breakpoints = 1

# Set gdb server to little endian
monitor endian little

# Set JTAG speed to 5 kHzd
monitor speed 1000

# Reset the target
monitor reset
monitor sleep 10

# Vector table placed in Flash
# monitor writeu32 0xE000ED08 = 0x00000000

# Initializing PC and stack pointer
# monitor reg r13 = (0x00000000)
# monitor reg pc = (0x00000004)

#file dddp.out
file wmdp.axf

break Reset_Handler
break main
load
monitor reset