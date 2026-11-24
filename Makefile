#---------------------------------------------------------------------------------
# Target settings
#---------------------------------------------------------------------------------
TARGET		:= 32gx
BUILD		:= build
SOURCES		:= source source/ui core core/pico core/cpu core/libretro
INCLUDES	:= include source core core/libretro
EXE			:= $(TARGET).elf

#---------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------
export DEVKITPRO	?= /opt/devkitpro
export DEVKITPPC	?= /opt/devkitpro/devkitPPC

PREFIX		:= powerpc-eabi-
CC			:= $(PREFIX)gcc
CXX			:= $(PREFIX)g++
LD			:= $(PREFIX)g++
ELF2DOL		:= elf2dol

MACHDEP		:= -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float

# Explicit include paths for libogc headers like gccore.h
LIBOGC_INC	:= -I$(DEVKITPRO)/libogc/include -I$(DEVKITPRO)/libogc/include/ogc
CFLAGS		:= -O3 -g -Wall $(MACHDEP) -D__LIBRETRO__ -DHW_RVL $(LIBOGC_INC) $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))
CXXFLAGS	:= $(CFLAGS) -std=gnu++98
LDFLAGS		:= -g $(MACHDEP)

# Libraries needed for Wii GX, Audio, and Controllers
LIBS		:= -lasnd -lfat -ogc -lwiiuse -lbte -lm

# Find all C source files across all source directories
VPATH		:= $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
CFILES		:= $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
C_OBJS		:= $(notdir $(CFILES:.c=.o))
OBJS		:= $(C_OBJS)

#---------------------------------------------------------------------------------
# Build Rules
#---------------------------------------------------------------------------------
.PHONY: all clean

all: $(BUILD) $(EXE)

$(BUILD):
	@mkdir -p $@

$(BUILD)/%.o: %.c
	@echo "Compiling $(notdir $<)..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(OBJS:%=$(BUILD)/%)
	@echo "Linking $(EXE)..."
	@$(LD) $(LDFLAGS) $(OBJS:%=$(BUILD)/%) $(LIBS) -o $@
	@$(ELF2DOL) $(EXE) boot.dol
	@echo "Done! Generated boot.dol for Homebrew Channel."

clean:
	@echo "Cleaning up..."
	@rm -fr $(BUILD) $(EXE) boot.dol
