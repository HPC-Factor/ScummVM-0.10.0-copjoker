# ScummVM Makefile for Windows CE port
# Uses the VLC toolchain
# $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/backends/platform/wince/Makefile $
# $Id: Makefile 27290 2007-06-10 08:23:51Z knakos $

wince_gcc_root = /cygdrive/e/wince-gcc-root
srcdir    = ../../..
VPATH     = $(srcdir)

#DISABLE_SCUMM = 1
#DISABLE_SKY = 1
#DISABLE_QUEEN = 1
#DISABLE_GOB = 1
DISABLE_LURE = 1
#DISABLE_CINE = 1
#DISABLE_SAGA = 1
#DISABLE_KYRA = 1
#DISABLE_AGI = 1
#DISABLE_AGOS = 1
#DISABLE_SWORD1 = 1
#DISABLE_SWORD2 = 1
#DISABLE_TOUCHE = 1
#DISABLE_PARALLACTION = 1
DISABLE_CRUISE = 1

#DISABLE_HQ_SCALERS = 1

CXX    = arm-wince-pe-g++
LD     = arm-wince-pe-g++
AR     = arm-wince-pe-ar cru
RANLIB = arm-wince-pe-ranlib
STRIP  = arm-wince-pe-strip
WINDRES= arm-wince-pe-windres
MKDIR  = mkdir -p
RM     = rm -f
RM_REC = rm -rf
ECHO   = echo -n
CAT    = cat
AS     = arm-wince-pe-as

DEFINES := -D_WIN32_WCE=300 -DARM -D__ARM__ -D_ARM_ -DUNICODE -DSCUMM_NEED_ALIGNMENT -DFPM_DEFAULT -DUSE_MAD -DNONSTANDARD_PORT -DUSE_TREMOR -DUSE_VORBIS -DUSE_MPEG2 -DWIN32 -DUSE_ZLIB -DUSE_FLAC
DEFINES += -D__stdcall= -Dcdecl= -D__cdecl__= -D__cdecl= -Wno-multichar
#DEFINES += -DDEBUG -DUSE_WINDBG -g

INCLUDES := -I$(srcdir) -I. -I$(srcdir)/engines -Imissing/gcc -Ilibs/include -Ilibs/include/sdl -ICEgui -ICEkeys -I$(wince_gcc_root)/include

CFLAGS := -O3 -march=armv4 -mtune=xscale

CXXFLAGS := $(CFLAGS)

LDFLAGS := -Llibs/lib -L$(wince_gcc_root)/lib
LIBS := -lSDL -lzlib -lmad -lmpeg2 -ltremorce -lFLAC --entry WinMainCRTStartup

TARGET = scummvm.exe
OBJS := 
MODULE_DIRS += .

OBJS += CEActionsPocket.o CEDevice.o CEScaler.o \
        CEActionsSmartphone.o CELauncherDialog.o wince-sdl.o 
OBJS += CEgui/GUIElement.o CEgui/Panel.o CEgui/SDL_ImageResource.o \
        CEgui/ItemAction.o CEgui/PanelItem.o CEgui/Toolbar.o \
        CEgui/ItemSwitch.o CEgui/PanelKeyboard.o CEgui/ToolbarHandler.o 
OBJS += CEkeys/EventsBuffer.o
OBJS += $(srcdir)/gui/Actions.o $(srcdir)/gui/Key.o $(srcdir)/gui/KeysDialog.o 
OBJS += ../sdl/sdl.o ../sdl/graphics.o ../sdl/events.o 
OBJS += missing/missing.o
OBJS += PocketSCUMM.o ARMscaler.o

include $(srcdir)/Makefile.common

all: dist

dist: $(TARGET)
	$(STRIP) $(TARGET)

PocketSCUMM.o:
	$(WINDRES) $*.rc $@

ARMscaler.o:
	$(AS) $*.s -o $@

$(TARGET): $(OBJS)
	$(LD) $^ $(LDFLAGS) $(LIBS) -o $@ -Wl,-Map,scummvm.exe.map -Wl,--stack,65536
