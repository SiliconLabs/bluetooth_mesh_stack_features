################################################################################
# General project settings                                                     #
################################################################################

PROJECTNAME = switch
SDK_DIR = ../../../..

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
include $(SDK_DIR)/app/btmesh/component_host/ncp_host_btmesh.mk
include $(SDK_DIR)/app/bluetooth/component_host/memory_manager.mk

################################################################################
# Include paths                                                                #
################################################################################

override INCLUDEPATHS += . \
./utils \
$(SDK_DIR)/app/bluetooth/common_host/system \
$(SDK_DIR)/platform/common/inc \
$(SDK_DIR)/protocol/bluetooth/inc \
$(SDK_DIR)/platform/emlib/host/inc

################################################################################
# Input files                                                                  #
################################################################################

override C_SRC += \
$(SDK_DIR)/app/bluetooth/common_host/system/system.c \
$(SDK_DIR)/app/bluetooth/common_host/system/sl_btmesh.c \
$(SDK_DIR)/protocol/bluetooth/src/sl_btmesh_lib.c \
$(SDK_DIR)/protocol/bluetooth/src/sl_btmesh_serdeser.c \
$(SDK_DIR)/app/bluetooth/common_host/sl_core_mock/sl_core_mock.c \
utils/timer.c \
app.c \
main.c

################################################################################
# Target rules                                                                 #
################################################################################

include $(SDK_DIR)/app/bluetooth/component_host/targets.mk