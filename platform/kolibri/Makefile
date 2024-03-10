CC = kos32-gcc
LD = kos32-ld
OBJCOPY = kos32-objcopy
KPACK = kpack

SDK_DIR = $(abspath ../../../kolibrios/contrib/sdk)

CFLAGS = -c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -D_KOLIBRI -DNO_ZIP -DPERFORMANCE
LDFLAGS = -static -S -nostdlib -T $(SDK_DIR)/sources/newlib/app.lds --image-base 0 --subsystem native

INCLUDES = -I$(SDK_DIR)/sources/newlib/libc/include -I$(SDK_DIR)/sources/SDL-1.2.2_newlib/include -I. -I ../../source -I ../../sourcex -I ../../resource -I ../../freebios
LIBPATH = -L $(SDK_DIR)/lib -L /home/autobuild/tools/win32/mingw32/lib

TARGET = pokemini

OBJS =  PokeMini_SDL.o                     \
	../../sourcex/KeybMapSDL.o         \
	../../sourcex/ExportBMP.o          \
	../../sourcex/ExportWAV.o          \
	../../source/PMCommon.o            \
	../../source/PokeMini.o            \
	../../source/Multicart.o           \
	../../source/Hardware.o            \
	../../source/Video.o               \
	../../source/Video_x1.o            \
	../../source/Video_x2.o            \
	../../source/Video_x3.o            \
	../../source/Video_x4.o            \
	../../source/Video_x5.o            \
	../../source/Video_x6.o            \
	../../source/CommandLine.o         \
	../../source/MinxCPU.o             \
	../../source/MinxCPU_XX.o          \
	../../source/MinxCPU_CE.o          \
	../../source/MinxCPU_CF.o          \
	../../source/MinxCPU_SP.o          \
	../../source/MinxTimers.o          \
	../../source/MinxIO.o              \
	../../source/MinxIRQ.o             \
	../../source/MinxPRC.o             \
	../../source/MinxColorPRC.o        \
	../../source/MinxLCD.o             \
	../../source/MinxAudio.o           \
	../../source/UI.o                  \
	../../source/Joystick.o            \
	../../source/Keyboard.o            \
	../../resource/PokeMini_ColorPal.o \
	../../resource/PokeMini_Font12.o   \
	../../resource/PokeMini_Icons12.o  \
	../../resource/PokeMini_BG2.o      \
	../../resource/PokeMini_BG3.o      \
	../../resource/PokeMini_BG4.o      \
	../../resource/PokeMini_BG5.o      \
	../../resource/PokeMini_BG6.o      \
	../../freebios/freebios.o

LIBS = -lSDLn -lsound -lgcc -lc.dll

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(LIBPATH) -o $(TARGET) $(OBJS) $(LIBS)
	$(OBJCOPY) $(TARGET) -O binary
	$(KPACK) --nologo $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
