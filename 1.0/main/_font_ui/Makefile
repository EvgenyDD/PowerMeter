EXE_NAME=test_ui

SOURCES += main.cpp ui.c
SOURCES += $(call rwildcard,../fonts,*.c)

CFLAGS   += -fvisibility=hidden -funsafe-math-optimizations -fdata-sections -ffunction-sections -fno-move-loop-invariants
CFLAGS   += -fmessage-length=0 -fno-exceptions -fno-common -fno-builtin -ffreestanding
CFLAGS   += $(C_FULL_FLAGS)
CFLAGS   += -Werror

CXXFLAGS += -fvisibility=hidden -funsafe-math-optimizations -fdata-sections -ffunction-sections -fno-move-loop-invariants
CXXFLAGS += -fmessage-length=0 -fno-exceptions -fno-common -fno-builtin
CXXFLAGS += -fvisibility-inlines-hidden -fuse-cxa-atexit -felide-constructors
# CXXFLAGS += -fno-rtti # causes errors on wxWidgets static build
# CXXFLAGS += $(CXX_FULL_FLAGS)
# CXXFLAGS += -Werror

ifneq (,$(findstring Windows,$(OS)))
	SOURCES += sample.rc
    LDFLAGS += -static
    TCHAIN    = x86_64-w64-mingw32-
    WX_FLAGS := $(subst /mingw64/,C:/msys64/mingw64/, $(shell wx-config --cxxflags))
    WINDRES  := $(subst /mingw64/,C:/msys64/mingw64/, $(shell wx-config --rescomp))
    WX_LD    := $(subst /mingw64/lib/lib, -l, $(shell wx-config --libs))
    WX_LD    := $(subst .a, , $(WX_LD))
else
# don't do a static build!
    WX_FLAGS := $(shell wx-config --cxxflags)
    WX_LD    := $(shell wx-config --libs)
    WINDRES  := $(shell wx-config --rescomp)
endif

LDFLAGS += $(WX_LD)
FLAGS   += $(WX_FLAGS) -gdwarf-2 -ggdb -g3

include core.mk

run: $(EXECUTABLE)
	@$(EXECUTABLE)
