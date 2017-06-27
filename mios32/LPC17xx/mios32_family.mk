# $Id: mios32_family.mk 1156 2011-03-27 18:45:18Z tk $
# defines additional rules for MIOS32 family

# select driver library
DRIVER_LIB =	$(MIOS32_PATH)/drivers/$(FAMILY)

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/mios32/$(FAMILY) \
	     -I $(DRIVER_LIB)/CMSIS/inc \
	     -I $(DRIVER_LIB)/iap/inc \
	     -I $(DRIVER_LIB)/usbstack/inc

# add modules to thumb sources
THUMB_SOURCE += \
	$(DRIVER_LIB)/CMSIS/src/core_cm3.c \
	$(DRIVER_LIB)/usbstack/src/usbhw_lpc.c \
	$(DRIVER_LIB)/usbstack/src/usbcontrol.c \
	$(DRIVER_LIB)/usbstack/src/usbstdreq.c \
	$(DRIVER_LIB)/usbstack/src/usbinit.c


THUMB_AS_SOURCE += 

# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/mios32/$(FAMILY)
