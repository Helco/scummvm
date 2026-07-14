MODULE := engines/edna

MODULE_OBJS = \
	console.o \
	db.o \
	db_validation.o \
	edna.o \
	graphics.o \
	metaengine.o
	metaengine.o \
	sprite\Sprite.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_EDNA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
