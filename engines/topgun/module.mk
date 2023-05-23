MODULE := engines/topgun

MODULE_OBJS = \
	topgun.o \
	console.o \
	convertKey.o \
	metaengine.o \
	Resource.o \
	ResourceFile.o \
	Savestate.o \
	Scene.o \
	graphics/Bitmap.o \
	graphics/Cell.o \
	graphics/Sprite.o \
	graphics/SpriteContext.o \
	graphics/SpriteMessageHandler.o \
	graphics/SpriteMessageQueue.o \
	graphics/Text.o \
	plugins/loadPlugin.o \
	plugins/TamaPlugin.o \
	script/IPlugin.o \
	script/Script.o \
	script/Script_Calc.o \
	script/Script_Procedure.o \
	script/Script_Root.o \
	script/ScriptDebugger.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOPGUN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
