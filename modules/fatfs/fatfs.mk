# $Id: fatfs.mk 954 2010-03-08 22:33:22Z tk $
# defines additional rules for integrating FATFS

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/fatfs/src


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/fatfs/src/diskio.c \
	$(MIOS32_PATH)/modules/fatfs/src/option/ccsbcs.c \
	$(MIOS32_PATH)/modules/fatfs/src/ff.c


# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/fatfs
