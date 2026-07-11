MODULE := engines/edna

MODULE_OBJS = \
	db.o \
	db_validation.o \
	console.o \
	edna.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_EDNA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
