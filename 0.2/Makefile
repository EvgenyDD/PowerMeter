EXE_NAME=anlz220
MAKE_BINARY=yes
MAKE_EXECUTABLE=yes

TCHAIN = arm-none-eabi-
MCPU += -mcpu=cortex-m0plus -mthumb
CDIALECT = gnu99
OPT_LVL = 2
DBG_OPTS = -gdwarf-2 -ggdb -g

CFLAGS   += -fvisibility=hidden -funsafe-math-optimizations -fdata-sections -ffunction-sections -fno-move-loop-invariants
CFLAGS   += -fmessage-length=0 -fno-exceptions -fno-common -fno-builtin -ffreestanding
CFLAGS   += -fsingle-precision-constant
CFLAGS   += $(C_FULL_FLAGS)
CFLAGS   += -Werror

CXXFLAGS += -fvisibility=hidden -funsafe-math-optimizations -fdata-sections -ffunction-sections -fno-move-loop-invariants
CXXFLAGS += -fmessage-length=0 -fno-exceptions -fno-common -fno-builtin -ffreestanding
CXXFLAGS += -fvisibility-inlines-hidden -fuse-cxa-atexit -felide-constructors -fno-rtti
CXXFLAGS += -fsingle-precision-constant
CXXFLAGS += $(CXX_FULL_FLAGS)
CXXFLAGS += -Werror

LDFLAGS  += -specs=nano.specs
LDFLAGS  += -Wl,--gc-sections
LDFLAGS  += -Wl,--print-memory-usage

EXT_LIBS +=c nosys

PPDEFS += DUSE_HAL_DRIVER STM32G030xx

INCDIR += .
INCDIR += base/Core/Inc
INCDIR += base/Drivers/STM32G0xx_HAL_Driver/Inc
INCDIR += base/Drivers/STM32G0xx_HAL_Driver/Inc/Legacy
INCDIR += base/Drivers/CMSIS/Device/ST/STM32G0xx/Include
INCDIR += base/Drivers/CMSIS/Include
INCDIR += lcd_pico
INCDIR += Fonts
INCDIR += GUI

SOURCES += $(call rwildcard, base, *.c *.S *.s)
SOURCES += $(wildcard Fonts/*.c)
SOURCES += $(wildcard GUI/*.c)
SOURCES += $(wildcard lcd_pico/*.c)
SOURCES += $(wildcard *.c)

LDSCRIPT += base/STM32G030C8Tx_FLASH.ld

include core.mk

flash: $(BINARY)
	openocd -f target/stm32g030.cfg -c "program $< 0x08000000 verify reset exit" 

debug:
	@echo "file $(EXECUTABLE)" > .gdbinit
	@echo "set auto-load safe-path /" >> .gdbinit
	@echo "set confirm off" >> .gdbinit
	@echo "target remote | openocd -c \"gdb_port pipe\" -f target/stm32g030.cfg" >> .gdbinit
	@arm-none-eabi-gdb -q -x .gdbinit
