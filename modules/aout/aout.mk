# $Id: aout.mk 498 2009-05-09 13:56:37Z tk $

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/aout


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/aout/aout.c




# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/aout
