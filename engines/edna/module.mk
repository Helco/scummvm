MODULE := engines/edna

MODULE_OBJS = \
	assetcache.o \
	console.o \
	db.o \
	db_overlay.o \
	db_validation.o \
	edna.o \
	graphics.o \
	game/ednagame.o \
	game/ednagirl.o \
	game/ednastd.o \
	game/game.o \
	game/intro.o \
	game/scriptonclick.o \
	group/choicelist.o \
	group/group.o \
	group/inventory.o \
	input.o \
	metaengine.o \
	pathfinder.o \
	script.o \
	scriptcommand.o \
	sprite/animation.o \
	sprite/button.o \
	sprite/character.o \
	sprite/commandprompt.o \
	sprite/object.o \
	sprite/sprite.o \
	sprite/text.o \
	translation.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_EDNA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
