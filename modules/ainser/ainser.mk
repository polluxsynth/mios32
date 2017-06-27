# $Id: ainser.mk 1373 2011-12-19 19:14:49Z tk $

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/ainser


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/ainser/ainser.c




# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/ainser
