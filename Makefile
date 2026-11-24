# So Makefile knows what to do:
export DEVKITPRO	:= /opt/devkitpro
export DEVKITPPC	:= /opt/devkitpro/devkitPPC
#---------------------------------------------------------------------------------
# Target settings
#---------------------------------------------------------------------------------
TARGET		:= 32gx
BUILD		:= build
SOURCES		:= source source/ui core core/pico core/cpu core/libretro
INCLUDES	:= include source core core/libretro
DATA		:= 
EXE			:= $(TARGET).elf

#---------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------
# Enable optimizations for performance on the 729 MHz Broadway CPU
CFLAGS		:= -O3 -g -Wall $(MACHDEP) -D__LIBRETRO__ -DGEKKO -DHW_RVL
CXXFLAGS	:= $(CFLAGS) -std=gnu++98

LDFLAGS		:= -g $(MACHDEP)

# Libraries needed for Wii GX, Audio, and Pad/WPAD inputs
LIBS		:= -lasnd -lfat -ogc -lwiiuse -lbte -lm

#---------------------------------------------------------------------------------
# Uncomment/Set devkitPPC paths if not already in your environment
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
export DEVKITPPC	:= $(PORTABLE_DEVKITPPC)
endif

ifeq ($(strip $(DEVKITPRO)),)
export DEVKITPRO	:= $(PORTABLE_DEVKITPRO)
endif

# Include standard devkitPPC rules
include $(DEVKITPRO)/libogc/Makefile_rules

#---------------------------------------------------------------------------------
# Automatically find all C and Assembly source files in directories
#---------------------------------------------------------------------------------
CFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES		:= $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export VPATH	:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

C_OBJS		:= $(CFILES:.c=.o)
S_OBJS		:= $(SFILES:.s=.o)
OBJS		:= $(C_OBJS) $(S_OBJS)

#---------------------------------------------------------------------------------
# Build Rules
#---------------------------------------------------------------------------------
.PHONY: all clean

all: $(BUILD) $(EXE)

$(BUILD):
	@mkdir -p $@

$(EXE): $(OBJS:%=$(BUILD)/%)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) $(OBJS:%=$(BUILD)/%) $(LIBS) -o $@
	@elf2dol $(EXE) boot.dol
	@echo "Done! Generated boot.dol for Homebrew Channel."

clean:
	@echo "Cleaning up..."
	@rm -fr $(BUILD) $(EXE) boot.dol
