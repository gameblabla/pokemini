# PokeMini Makefile for SDL

CC =  nspire-gcc
LD =  nspire-gcc
STRIP = 
POKEROOT = ../../
BUILD = Build
TARGET = pokemini.elf

CFLAGS = -DARM9 -Ofast -fomit-frame-pointer -mthumb -mthumb-interwork -marm -march=armv5te -mtune=arm926ej-s -DNO_ZIP -DPERFORMANCE -DNSPIRE -Wall $(INCLUDE)
SLFLAGS = -Wl,--as-needed -flto -O -lSDL -lm

INCDIRS = source sourcex resource freebios

OBJS = \
 n2DLib.o	\
 PokeMini_SDL.o	\
 PokeMini.o	\
 Hardware_nsp.o	\
 freebios/freebios.o	\
 source/PMCommon.o	\
 source/Multicart.o	\
 source/Video.o	\
 source/Video_x3.o	\
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
 source/UI.o	\
 source/Joystick.o	\
 resource/PokeMini_ColorPal.o	\
 resource/PokeMini_Font12.o	\
 resource/PokeMini_Icons12.o	\
 resource/PokeMini_BG3.o	\

DEPENDS = \
 sourcex/ExportBMP.h	\
 sourcex/ExportWAV.h	\
 freebios/freebios.h	\
 source/IOMap.h	\
 source/PMCommon.h	\
 source/PokeMini.h	\
 source/PokeMini_Version.h	\
 source/Multicart.h	\
 source/Hardware.h	\
 source/Video.h	\
 source/Video_x3.h	\
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
 dependencies/minizip/unzip.h	\
 dependencies/minizip/ioapi.h	\
 resource/PokeMini_ColorPal.h	\
 resource/PokeMini_Font12.h	\
 resource/PokeMini_Icons12.h	\
 resource/PokeMini_BG3.h	\

BUILDOBJS = $(addprefix $(BUILD)/, $(notdir $(OBJS)))
DEPENDSHDR = $(addprefix $(POKEROOT), $(DEPENDS))
INCLUDE = $(foreach inc, $(INCDIRS), -I$(POKEROOT)$(inc))
VPATH = $(addprefix $(POKEROOT),$(INCDIRS))

.PHONY: all win clean
 
all: $(BUILD) $(TARGET) gzn prg

gzn:
	genzehn --input ./$(TARGET) --output ./pokemini.tns --name "pokemini" --compress

prg:
	make-prg ./pokemini.tns ./pokemini.prg.tns
	rm ./pokemini.tns
	rm ./$(TARGET)
	mv ./pokemini.prg.tns ./pokemini.tns

$(BUILD):
	@[ -d @ ] || mkdir -p $@

$(BUILD)/%.o: %.c $(DEPENDSHDR)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(BUILDOBJS)
	$(LD) -o $(TARGET) $(BUILDOBJS) $(SLFLAGS)

clean:
	-rm -f $(BUILDOBJS) $(TARGET) $(WINTARGET) $(WINRES_TRG)
	-rmdir --ignore-fail-on-non-empty $(BUILD)
