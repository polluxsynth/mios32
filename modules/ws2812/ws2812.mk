# $Id: ws2812.mk 2223 2015-10-17 20:07:36Z tk $

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/ws2812


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/ws2812/ws2812.c




# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/ws2812
