################################################################################
# General project settings                                                     #
################################################################################

PROJECTNAME = btmesh_host_provisioner
SDK_DIR = ../../../..

# Disable threading as handling more than a few mesh devices can cause data loss
HOST_THREADING = 0

# Still add pthread because user input handling uses it
override LDFLAGS += -pthread

# SL_CATALOG_APP_PROV_PRESENT flag indicates host provisioner application component present
override CFLAGS += -D SL_CATALOG_APP_PROV_PRESENT

################################################################################
# Components                                                                   #
#                                                                              #
# After setting up the toolchain, components can contribute to the project by  #
# appending items to the project variables like INCLUDEPATHS or C_SRC.         #
################################################################################

include $(SDK_DIR)/app/bluetooth/component_host/toolchain.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_log.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_assert.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_signal.mk
include $(SDK_DIR)/app/bluetooth/component_host/app_timer.mk
include $(SDK_DIR)/app/bluetooth/component_host/ncp_reset.mk
include $(SDK_DIR)/app/bluetooth/component_host/ncp_version.mk
include $(SDK_DIR)/app/btmesh/component_host/ncp_host_btmesh.mk
include $(SDK_DIR)/app/btmesh/component_host/btmesh_key_export.mk
include $(SDK_DIR)/app/btmesh/component_host/btmesh_prov.mk
include $(SDK_DIR)/app/btmesh/component_host/btmesh_conf.mk

################################################################################
# Include paths                                                                #
################################################################################

override INCLUDEPATHS += . \
$(SDK_DIR)/app/bluetooth/common/ncp_evt_filter \
$(SDK_DIR)/app/bluetooth/common_host/app_sleep \
$(SDK_DIR)/app/bluetooth/common_host/system \
$(SDK_DIR)/platform/common/inc \
$(SDK_DIR)/protocol/bluetooth/inc

################################################################################
# Input files                                                                  #
################################################################################

override C_SRC += \
$(SDK_DIR)/app/bluetooth/common_host/app_sleep/app_sleep.c \
$(SDK_DIR)/app/bluetooth/common_host/system/system.c \
$(SDK_DIR)/app/bluetooth/common_host/system/sl_btmesh.c \
app.c \
app_conf.c \
btmesh_app_prov.c \
btmesh_app_prov_ui.c \
main.c

################################################################################
# Target rules                                                                 #
################################################################################

include $(SDK_DIR)/app/bluetooth/component_host/targets.mk