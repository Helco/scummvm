MODULE := engines/edna

MODULE_OBJS = \
	assetcache.o \
	console.o \
	db.o \
	db_overlay.o \
	db_validation.o \
	edna.o \
	graphics.o \
	game\game.o \
	game\intro.o \
	game\scriptonclick.o \
	metaengine.o \
	script.o \
	scriptcommand.o \
	sprite\animation.o \
	sprite\group.o \
	sprite\object.o \
	sprite\player.o \
	sprite\sprite.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_EDNA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
