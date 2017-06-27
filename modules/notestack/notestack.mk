# $Id: notestack.mk 683 2009-08-01 14:29:36Z tk $

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/notestack


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/notestack/notestack.c


# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/notestack
