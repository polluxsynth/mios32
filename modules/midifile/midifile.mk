# $Id: midifile.mk 188 2008-12-16 23:46:09Z tk $
# defines additional rules for integrating the midifile modules

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/midifile


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/midifile/mid_parser.c


# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/midifile
