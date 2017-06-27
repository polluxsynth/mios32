# $Id: blm.mk 23 2008-09-16 17:34:42Z tk $
# defines additional rules for integrating the button/LED matrix

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/blm


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/blm/blm.c


# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/blm
