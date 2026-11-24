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
export DEVKITPRO	?= /opt/devkitpro
export DEVKITPPC	?= /opt/devkitpro/devkitPPC

MACHDEP		:= -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float
CFLAGS		:= -O3 -g -Wall $(MACHDEP) -D__LIBRETRO__ -DHW_RVL
CXXFLAGS	:= $(CFLAGS) -std=gnu++98

LDFLAGS		:= -g $(MACHDEP)

# Libraries needed for Wii GX, Audio, and Pad/WPAD inputs
LIBS		:= -lasnd -lfat -ogc -lwiiuse -lbte -lm

# Include modern devkitPPC base rules
include $(DEVKITPPC)/base_rules

#---------------------------------------------------------------------------------
# Automatically find all C source files in directories
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
