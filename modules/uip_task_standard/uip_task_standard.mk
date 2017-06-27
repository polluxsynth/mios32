# $Id: uip_task_standard.mk 1387 2011-12-29 17:49:56Z tk $
# defines additional rules

# enhance include path
C_INCLUDE += -I $(MIOS32_PATH)/modules/uip_task_standard

# add modules to thumb sources (TODO: provide makefile option to add code to ARM sources)
THUMB_SOURCE += \
	$(MIOS32_PATH)/modules/uip_task_standard/uip_task.c \
	$(MIOS32_PATH)/modules/uip_task_standard/dhcpc.c \
	$(MIOS32_PATH)/modules/uip_task_standard/osc_server.c \
	$(MIOS32_PATH)/modules/uip_task_standard/osc_client.c \
	$(MIOS32_PATH)/modules/uip_task_standard/uip_terminal.c


# directories and files that should be part of the distribution (release) package
DIST += $(MIOS32_PATH)/modules/uip_task_standard
