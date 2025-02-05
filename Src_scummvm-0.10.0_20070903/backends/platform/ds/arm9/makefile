#BUILD_PLUGINS = 1
#libndsdir = $(DEVKITPRO)/libnds
libndsdir = /home/neil/devkitpro/libnds

# Select the build you want by uncommenting one of the following lines:

DS_BUILD_A = 1
#DS_BUILD_B = 1
#DS_BUILD_C = 1
#DS_BUILD_D = 1

# Uncomment the following line to build in support for MP3 audio
# using libmad:
USE_MAD = 1

# Uncomment the following line to enable support for the
# ace DS Debugger (remembering to make the same change in the arm7 makefile):
#USE_DEBUGGER = 1
# NOTE: The header and libs for the debugger is assumed to be in the libnds
# folder.

VPATH = $(srcdir)

# Command to build libmad is:
# ./configure --host=arm-elf --enable-speed --enable-sso -enable-fpm=arm CFLAGS='-specs=ds_arm9.specs -mthumb-interwork'


ifdef DS_BUILD_A
	DEFINES = -DDS_SCUMM_BUILD -DDS_BUILD_A
	LOGO = logoa.bmp
	DISABLE_HE = 1
	#DISABLE_SCUMM = 1
	DISABLE_SCUMM_7_8 = 1
	DISABLE_AGOS = 1
	DISABLE_SKY = 1
	DISABLE_SWORD1 = 1
	DISABLE_SWORD2 = 1
	DISABLE_QUEEN = 1
	DISABLE_SAGA = 1
	DISABLE_KYRA = 1
	DISABLE_GOB = 1
	DISABLE_LURE = 1
	DISABLE_CINE = 1
	DISABLE_AGI = 1
	DISABLE_TOUCHE = 1
	DISABLE_PARALLACTION = 1
	BUILD=scummvm-A
endif

ifdef DS_BUILD_B
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_B
	LOGO = logob.bmp
	DISABLE_HE = 1
	DISABLE_SCUMM = 1
	DISABLE_SCUMM_7_8 = 1
	DISABLE_AGOS = 1
	#DISABLE_SKY = 1
	DISABLE_SWORD1 = 1
	DISABLE_SWORD2 = 1
	#DISABLE_QUEEN = 1
	DISABLE_SAGA = 1
	DISABLE_KYRA = 1
	DISABLE_GOB = 1
	DISABLE_LURE = 1
	DISABLE_CINE = 1
	DISABLE_AGI = 1
	DISABLE_TOUCHE = 1
	DISABLE_PARALLACTION = 1
	BUILD=scummvm-B
endif

ifdef DS_BUILD_C	
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_C
	LOGO = logoc.bmp
	DISABLE_HE = 1
	DISABLE_SCUMM = 1
	DISABLE_SCUMM_7_8 = 1
	#DISABLE_AGOS = 1
	DISABLE_SKY = 1
	DISABLE_SWORD1 = 1
	DISABLE_SWORD2 = 1
	DISABLE_QUEEN = 1
	DISABLE_SAGA = 1
	#DISABLE_KYRA = 1
	#DISABLE_GOB = 1
	DISABLE_LURE = 1
	DISABLE_CINE = 1
	DISABLE_AGI = 1
	DISABLE_TOUCHE = 1
	DISABLE_PARALLACTION = 1
	BUILD=scummvm-C
endif

ifdef DS_BUILD_D
	DEFINES = -DDS_NON_SCUMM_BUILD -DDS_BUILD_D
	LOGO = logoc.bmp
	DISABLE_HE = 1
	DISABLE_SCUMM = 1
	DISABLE_SCUMM_7_8 = 1
	DISABLE_AGOS = 1
	DISABLE_SKY = 1
	DISABLE_SWORD1 = 1
	DISABLE_SWORD2 = 1
	DISABLE_QUEEN = 1
	DISABLE_SAGA = 1
	DISABLE_KYRA = 1
	DISABLE_GOB = 1
	DISABLE_LURE = 1
	#DISABLE_CINE = 1
	#DISABLE_AGI = 1
	DISABLE_TOUCHE = 1
	DISABLE_PARALLACTION = 1
	BUILD=scummvm-A
endif


ARM7BIN	:= -7 $(CURDIR)/../../arm7/arm7.bin
ICON 		:= -b ../../../logo.bmp "ScummVM;By Neil Millstone;"

CC      = arm-eabi-gcc
CXX     = arm-eabi-g++

CFLAGS	=	-Wno-multichar -Wall -Os\
		-Wno-multichar -mcpu=arm9tdmi -mtune=arm9tdmi \
 		-mcpu=arm9tdmi -mtune=arm9tdmi -fomit-frame-pointer\
		-ffast-math -mthumb-interwork

ifdef USE_DEBUGGER
	DEFINES += -DUSE_DEBUGGER
	CFLAGS += -g
endif
		
CXXFLAGS=	$(CFLAGS) -Wno-non-virtual-dtor	 -Wno-non-virtual-dtor \
		-fno-exceptions -fno-rtti

ASFLAGS = -mcpu=arm9tdmi -mthumb-interwork
DEFINES += -D__DS__ -DNDS -DARM9 -DNONSTANDARD_PORT -DDISABLE_FANCY_THEMES -DDISABLE_DEFAULT_SAVEFILEMANAGER
ifdef USE_MAD
	DEFINES += -DUSE_MAD
endif


LDFLAGS = -specs=ds_arm9.specs -mthumb-interwork -mno-fpu -Wl,-Map,map.txt

INCLUDES= -I./ -I$(portdir)/$(BUILD) -I$(srcdir) -I$(srcdir)/common -I$(portdir)/source -I$(portdir)/source/compressor -I$(portdir)/source/fat \
			-I$(srcdir)/backends/fs  -I$(srcdir)/backends/fs/ds -I$(portdir)/data -I$(libndsdir)/include -I$(portdir)/../commoninclude\
			-I$(srcdir)/scumm -I$(libndsdir)/include -I$(libndsdir)/include/nds -I$(srcdir)/engines -I$(portdir)/source/mad\
			-I$(portdir)/source/libcartreset
			

LIBS	= -lm -L$(libndsdir)/lib -L$(portdir)/lib -lnds9
ifdef USE_MAD
	LIBS += -lmad
endif
ifdef USE_DEBUGGER
	LIBS += -ldsdebugger -ldswifi9
endif

#-Lscumm -lscumm -Lbase -lbase -Lcommon -lcommon -Lgraphics -lgraphics -Lgui -lgui -Lsound -lsound
EXECUTABLE = scummvm.elf
PLUGIN_PREFIX =
PLUGIN_SUFFIX = .plg
PLUGIN_EXTRA_DEPS = plugin.x plugin.syms scummvm.elf
PLUGIN_LDFLAGS = -nostartfiles -Wl,-q,-Tplugin.x,--just-symbols,scummvm.elf,--retain-symbols-file,plugin.syms -L$(ronindir)/lib
MKDIR = mkdir -p
RM = rm -f
RM_REC = rm -rf
AR = arm-eabi-ar cru
RANLIB = arm-eabi-ranlib
OBJCOPY = arm-eabi-objcopy
AS = arm-eabi-as
HAVE_GCC3 = true
DISABLE_SCALERS = true

ifdef BUILD_PLUGINS
DEFINES += -DDYNAMIC_MODULES
PRE_OBJS_FLAGS = -Wl,--whole-archive
POST_OBJS_FLAGS = -Wl,--no-whole-archive
endif

PORT_OBJS :=	$(portdir)/source/blitters.o $(portdir)/source/cdaudio.o $(portdir)/source/dsmain.o \
		$(portdir)/../../../fs/ds/ds-fs.o $(portdir)/source/gbampsave.o $(portdir)/source/scummhelp.o\
		$(portdir)/source/osystem_ds.o $(portdir)/source/portdefs.o $(portdir)/source/ramsave.o\
		$(portdir)/source/scummconsole.o $(portdir)/source/touchkeyboard.o $(portdir)/source/zipreader.o\
		$(portdir)/source/dsoptions.o $(portdir)/source/keys.o
		
DATA_OBJS := $(portdir)/data/icons.o $(portdir)/data/keyboard.o $(portdir)/data/keyboard_pal.o $(portdir)/data/default_font.o
			
		
COMPRESSOR_OBJS := $(portdir)/source/compressor/lz.o

FAT_OBJS :=  $(portdir)/source/fat/disc_io.o $(portdir)/source/fat/gba_nds_fat.o\
			$(portdir)/source/fat/io_fcsr.o $(portdir)/source/fat/io_m3cf.o\
			$(portdir)/source/fat/io_mpcf.o $(portdir)/source/fat/io_sccf.o\
			$(portdir)/source/fat/io_m3sd.o\
			$(portdir)/source/fat/io_nmmc.o $(portdir)/source/fat/io_scsd.o \
			$(portdir)/source/fat/io_scsd_asm.o \
			$(portdir)/source/fat/io_njsd.o \
			$(portdir)/source/fat/io_mmcf.o \
			$(portdir)/source/fat/io_sd_common.o \
			$(portdir)/source/fat/io_m3_common.o \
			$(portdir)/source/fat/io_dldi.o \
			$(portdir)/source/fat/m3sd.o


#			$(portdir)/source/fat/io_cf_common.o $(portdir)/source/fat/io_m3_common.o\
#			$(portdir)/source/fat/io_sd_common.o $(portdir)/source/fat/io_scsd_s.o \
#			$(portdir)/source/fat/io_sc_common.o $(portdir)/source/fat/io_sd_common.o 

LIBCARTRESET_OBJS := $(portdir)/source/libcartreset/cartreset.o
			
			
			
OBJS := $(DATA_OBJS) $(LIBCARTRESET_OBJS) $(PORT_OBJS) $(COMPRESSOR_OBJS) $(FAT_OBJS) 
		

		
MODULE_DIRS += .

ndsall:
	@[ -d $(BUILD) ] || mkdir -p $(BUILD)
	make -C ./$(BUILD) -f ../makefile scummvm.nds

include $(srcdir)/Makefile.common

clean:
	$(RM) $(OBJS) $(EXECUTABLE)
	rm -fr $(BUILD)

plugin_dist :
	find . -name '*.plg' | while read p; do \
	  sh-elf-strip -g -o "`basename \"$$p\" | tr '[:lower:]' '[:upper:]'`" "$$p"; \
	done

dist : SCUMMVM.BIN plugins plugin_dist


#---------------------------------------------------------------------------------
# canned command sequence for binary data
#---------------------------------------------------------------------------------
#define bin2o
#	bin2s $< | $(AS) -mthumb -mthumb-interwork -o $(@)
#	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_raw_end[];" > `(echo $(<F) | tr . _)`.h
#	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_raw[];" >> `(echo $(<F) | tr . _)`.h
#	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_raw_size";" >> `(echo $(<F) | tr . _)`.h
#endef


define bin2o
	bin2s $< | $(AS) -mthumb -mthumb-interwork -o $(@)
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > `(echo $(<F) | tr . _)`.h
	echo "extern const u8" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> `(echo $(<F) | tr . _)`.h
	echo "extern const u32" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> `(echo $(<F) | tr . _)`.h
endef


#define bin2o
#	@echo $(*)
#	cp $(<) $(*).tmp
#	$(OBJCOPY) -I binary -O elf32-littlearm -B arm \
#	--rename-section .data=.rodata \
#	--redefine-sym _binary_$(subst .,_,$(subst /,_,$(*)))_tmp_start=$(notdir $*)\
#	--redefine-sym _binary_$(subst .,_,$(subst /,_,$(*)))_tmp_end=$(notdir $*)_end\
#	$(*).tmp $(@)
#	echo "extern const u8" $(notdir $*)"[] __attribute__ ((aligned (4)));" > $(*).h
#	echo "extern const u32" $(notdir $(*))_size[]";" >> $(*).h
#	
#	echo $(*).h
#	rm $(*).tmp
#endef

##############
# Replacement rule for the one in makefile.common
##############
ifndef HAVE_GCC3
# If you use GCC, disable the above and enable this for intelligent
# dependency tracking. 
.cpp.o:
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d2" $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o
#	$(ECHO) "$(*D)/" > $(*D)/$(DEPDIR)/$(*F).d
	$(CAT) "$(*D)/$(DEPDIR)/$(*F).d2" >> "$(*D)/$(DEPDIR)/$(*F).d"
	$(RM) "$(*D)/$(DEPDIR)/$(*F).d2"
else
# If you even have GCC 3.x, you can use this build rule, which is safer; the above
# rule can get you into a bad state if you Ctrl-C at the wrong moment.
# Also, with this GCC inserts additional dummy rules for the involved headers,
# which ensures a smooth compilation even if said headers become obsolete.
.cpp.o:
	$(MKDIR) $(*D)/$(DEPDIR)
#	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o
	$(CXX) -MMD -MF "$(*D)/$(DEPDIR)/$(*F).d"              $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o
endif


#---------------------------------------------------------------------------------
 
#---------------------------------------------------------------------------------
%.o	:	%.pcx
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
 
#---------------------------------------------------------------------------------
%.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
 
#---------------------------------------------------------------------------------
%.o	:	%.raw
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
 
#---------------------------------------------------------------------------------
%.o	:	%.pal
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)
 
#---------------------------------------------------------------------------------
%.o	:	%.map
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.o	:	%.mdl
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
%.nds: %.bin
	@echo ndstool -c $@ -9 scummvm.bin $(ARM7BIN) -b ../../$(LOGO) "$(shell basename $@);ScummVM 0.10.0;DS Port"
	ndstool -c $@ -9 scummvm.bin $(ARM7BIN) -b ../../$(LOGO) "$(shell basename $@);ScummVM 0.10.0;DS Port"
	dsbuild $@ -l ../ndsloader.bin

	padbin 16 $(basename $@).ds.gba

#---------------------------------------------------------------------------------
%.bin: %.elf
	$(OBJCOPY) -S scummvm.elf scummvm-stripped.elf
	$(OBJCOPY) -O binary scummvm-stripped.elf scummvm.bin

#%.o: %.s
#	$(MKDIR) $(*D)/$(DEPDIR)
#	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o
	
