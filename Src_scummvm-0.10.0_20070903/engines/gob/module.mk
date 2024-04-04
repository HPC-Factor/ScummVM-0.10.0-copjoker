MODULE := engines/gob

MODULE_OBJS := \
	cdrom.o \
	dataio.o \
	detection.o \
	draw.o \
	draw_v1.o \
	draw_v2.o \
	draw_bargon.o \
	driver_vga.o \
	game.o \
	game_v1.o \
	game_v2.o \
	global.o \
	gob.o \
	goblin.o \
	goblin_v1.o \
	goblin_v2.o \
	goblin_v3.o \
	imd.o \
	init.o \
	init_v1.o \
	init_v2.o \
	init_v3.o \
	inter.o \
	inter_v1.o \
	inter_v2.o \
	inter_v3.o \
	inter_bargon.o \
	map.o \
	map_v1.o \
	map_v2.o \
	mult.o \
	mult_v1.o \
	mult_v2.o \
	music.o \
	palanim.o \
	parse.o \
	parse_v1.o \
	parse_v2.o \
	saveload.o \
	saveload_v2.o \
	saveload_v3.o \
	scenery.o \
	scenery_v1.o \
	scenery_v2.o \
	sound.o \
	util.o \
	video.o \
	video_v1.o \
	video_v2.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk
