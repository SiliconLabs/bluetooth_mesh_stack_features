/*************************************************************************
    > File Name: /home/zhfu/work/projs/provisioner/src/async/event_dispendser.c
    > Author: Kevin
    > Created Time: 2019-07-03
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "config.h"
#include "event_dispenser.h"
#include "err_code.h"
#include "prov_assert.h"
#include "utils.h"
#include "socket_handler.h"
#include "bg_stack/gecko_bglib.h"
#include "uart.h"

#include "adding_devices.h"
#include "async_config_client.h"
#include "blacklisting_devices.h"
#include "network_manage.h"

/* Defines  *********************************************************** */
BGLIB_DEFINE();

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
static volatile int hostTargetSyncUp = false;
static interfaceArgs_t *interface_args_ptr = NULL;
static int initialized = false;

static evt_handler handlers[to_max_invalid] = {
  /* TODO */
  adding_devices_event_handler,
  config_devices_event_handler,
  blacklist_devices_event_handler,
  generic_event_handler
};

/* Static Functions Declaractions ************************************* */
char *get_config_file_path(void)
{
  return interface_args_ptr == NULL ? NULL : interface_args_ptr->configFilePath;
}

static void openConnectionToNcpTarget(void)
{
  /**
   * Initialize BGLIB with our output function for sending messages.
   */
  hardASSERT(interface_args_ptr);
  hardASSERT(interface_args_ptr->bglib_output
             && interface_args_ptr->bglib_input
             && interface_args_ptr->bglib_peek);
  BGLIB_INITIALIZE_NONBLOCK(interface_args_ptr->bglib_output,
                            interface_args_ptr->bglib_input,
                            interface_args_ptr->bglib_peek);
  if (interface_args_ptr->smode == secureEnum) {
    if (interface_args_ptr->encrypted) {
      if (connect_domain_socket_server(interface_args_ptr->pSerSockPath, CLIENT_ENCRYPTED_PATH, 1)) {
        CS_ERR("Connection to encrypted domain socket unsuccessful. Exiting..\n");
        exit(EXIT_FAILURE);
      }
      CS_MSG("Turning on Encryption. All subsequent BGAPI commands and events will be encrypted..\n");
      turnEncryptionOn();
    } else {
      if (connect_domain_socket_server(interface_args_ptr->pSerSockPath, CLIENT_UNENCRYPTED_PATH, 0)) {
        printf("Connection to unencrypted domain socket unsuccessful. Exiting..\n");
        exit(EXIT_FAILURE);
      }
    }
  } else {
    if (0 != uartOpen((int8_t *)interface_args_ptr->pSerialPort, interface_args_ptr->baudRate, 1, 100)) {
      GET_HL_STR(port, "%s", interface_args_ptr->pSerialPort);
      CS_ERR("Open %s failed. Exiting..\n", hlstr_port);
      exit(EXIT_FAILURE);
    }
  }
}

static void sync_host_and_ncp_target(void)
{
  struct gecko_cmd_packet *p;

  hostTargetSyncUp = false;
  CS_MSG("Syncing up NCP Host and Target");
  do {
    printf("."); fflush(stdout);

    if (interface_args_ptr->smode == secureEnum) {
      poll_update(50);
    }
    p = gecko_peek_event();
    if (p) {
      switch (BGLIB_MSG_ID(p->header)) {
        case gecko_evt_system_boot_id:
        {
          CS_MSG("System Booted. NCP Target and Host Sync-ed Up\n");
          hostTargetSyncUp = true;
        }
        break;
        default:
          sleep(1);
          break;
      }
    } else {
      gecko_cmd_system_reset(0);
      CS_LOG("Sent reset signal to NCP target\n");
      sleep(1);
    }

    if (hostTargetSyncUp) {
      break;
    }
  } while (1);
}

void bglib_interface_init_sync(const interfaceArgs_t *p)
{
  hardASSERT(p);

  interface_args_ptr = (interfaceArgs_t *)p;
  hostTargetSyncUp = false;
  openConnectionToNcpTarget();
  /* memset(filters, 0, sizeof(filter_t) * MAX_FILTERS); */
  initialized = true;

  sync_host_and_ncp_target();
}

void event_dispenser(void)
{
  int i;
  struct gecko_cmd_packet *evt = NULL;

  if (!hostTargetSyncUp) {
    sync_host_and_ncp_target();
  } else {
    /* Receive the NCP event and enqueue to the send queue with receiver ID */
    do {
      if (interface_args_ptr->smode == secureEnum) {
        poll_update(50);
      }
      evt = gecko_peek_event();
      if (evt) {
        for (i = 0; i < to_max_invalid; i++) {
          if (!handlers[i] || handlers[i](evt)) {
            break;
          }
        }
      }
    } while (evt);
  }
}

int getSyncUpStatus(void)
{
  return hostTargetSyncUp;
}

void forceReSyncUpHostAndTarget(void)
{
  /* need lock? */
  hostTargetSyncUp = 0;
  sync_host_and_ncp_target();
}
