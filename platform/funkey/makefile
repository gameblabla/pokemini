# PokeMini Makefile for SDL

CC = /opt/funkey-toolchain/bin/arm-linux-gcc
LD = /opt/funkey-toolchain/bin/arm-linux-gcc
STRIP = /opt/funkey-toolchain/bin/arm-linux-strip
POKEROOT = ../../
BUILD = Build
TARGET = PokeMini
PROFILE = APPLY

FLAGS_OPT = -Ofast -fdata-sections -ffunction-sections -fno-common -flto -fsingle-precision-constant -fno-PIC

CFLAGS	= $(FLAGS_OPT) -Wall $(INCLUDE) -DPERFORMANCE -DOPENDINGUX
SLFLAGS = -nodefaultlibs -lc -lgcc -lm -lSDL -lasound -lz -no-pie -Wl,--as-needed -Wl,--gc-sections -s -flto

ifeq ($(PROFILE), YES)
CFLAGS 		+= -fprofile-generate="/mnt/poke"
SLFLAGS 	+= -lgcov
else ifeq ($(PROFILE), APPLY)
CFLAGS		+= -fprofile-use -fbranch-probabilities
endif

INCDIRS = source sourcex resource freebios dependencies/minizip

OBJS = \
 PokeMini_SDL.o	\
 sourcex/KeybMapSDL.o	\
 freebios/freebios.o	\
 source/PMCommon.o	\
 source/PokeMini.o	\
 source/Multicart.o	\
 source/Hardware.o	\
 source/Video.o	\
 source/Video_x2.o	\
 source/CommandLine.o	\
 source/MinxCPU.o	\
 source/MinxCPU_XX.o	\
 source/MinxCPU_CE.o	\
 source/MinxCPU_CF.o	\
 source/MinxCPU_SP.o \
 source/MinxTimers.o	\
 source/MinxIO.o	\
 source/MinxIRQ.o	\
 source/MinxPRC.o	\
 source/MinxColorPRC.o	\
 source/MinxLCD.o	\
 source/MinxAudio.o	\
 source/UI.o	\
 source/Joystick.o	\
 source/Keyboard.o	\
 resource/PokeMini_ColorPal.o	\
 resource/PokeMini_Font12.o	\
 resource/PokeMini_Icons12.o	\
 dependencies/unzip.o	\
 dependencies/ioapi.o	\
 resource/PokeMini_BG2.o

DEPENDS = \
 sourcex/KeybMapSDL.h	\
 freebios/freebios.h	\
 source/IOMap.h	\
 source/PMCommon.h	\
 source/PokeMini.h	\
 source/PokeMini_Version.h	\
 source/Multicart.h	\
 source/Hardware.h	\
 source/Video.h	\
 source/Video_x2.h	\
 source/CommandLine.h	\
 source/MinxCPU.h	\
 source/MinxTimers.h	\
 source/MinxIO.h	\
 source/MinxIRQ.h	\
 source/MinxPRC.h	\
 source/MinxColorPRC.h	\
 source/MinxLCD.h	\
 source/MinxAudio.h	\
 source/UI.h	\
 source/Joystick.h	\
 source/Keyboard.h	\
 resource/PokeMini_ColorPal.h	\
 resource/PokeMini_Font12.h	\
 resource/PokeMini_Icons12.h	\
 resource/PokeMini_BG2.h	\
 dependencies/minizip/unzip.h	\
 dependencies/minizip/ioapi.h	
 
 
BUILDOBJS = $(addprefix $(BUILD)/, $(notdir $(OBJS)))
DEPENDSHDR = $(addprefix $(POKEROOT), $(DEPENDS))
INCLUDE = $(foreach inc, $(INCDIRS), -I$(POKEROOT)$(inc))
VPATH = $(addprefix $(POKEROOT),$(INCDIRS))

.PHONY: all clean

all: $(BUILD) $(TARGET)

$(BUILD):
	@[ -d @ ] || mkdir -p $@

$(BUILD)/%.o: %.c $(DEPENDSHDR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(BUILDOBJS)
	$(LD) -o $(TARGET) $(BUILDOBJS) $(SLFLAGS)

clean:
	-rm -f $(BUILDOBJS) $(TARGET) $(WINTARGET) $(WINRES_TRG)
	-rmdir --ignore-fail-on-non-empty $(BUILD)
