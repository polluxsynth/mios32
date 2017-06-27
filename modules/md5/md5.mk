# $Id: md5.mk 1584 2012-12-08 17:38:25Z tk $
# defines additional rules for integrating the md5 modules

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/md5


# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/md5/md5.c


# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/md5
