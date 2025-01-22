/***************************************************************************//**
 * @file
 * @brief BTmesh NCP-host Switch Example Project.
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

/* Includes */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <signal.h>

#include "sl_bt_api.h"
#include "sl_btmesh_api.h"
#include "sl_btmesh_generic_model_capi_types.h"
#include "sl_btmesh_lighting_model_capi_types.h"
#include "sl_btmesh_lib.h"
#include "sl_bt_types.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_btmesh_ncp_host.h"
#include "ncp_host.h"
#include "sl_bt_ncp_host.h"

#include "utils/timer.h"
#include "app.h"

// Optstring argument for getopt.
#define OPTSTRING      NCP_HOST_OPTSTRING "h"

// Usage info.
#define USAGE          "\r\n%s " NCP_HOST_USAGE " [-h]\r\n"

// Options info.
#define OPTIONS    \
  "\r\nOPTIONS\r\n"    \
  NCP_HOST_OPTIONS \
  "    -h  Print this help message.\r\n"

/* Defines  *********************************************************** */
#define SPLIT_TOKENS                            "\t\r\n\a "

#define MAX_BUF_NUM                             5
#define CONSOLE_RX_BUF_SIZE                     64
#define MAX_CONSOLE_PARAM_NUM                   12

#define BUF_PENDING()                           ((bufReadOffset) == (bufWriteOffset) ? 0 : 1)
#define BUF_FULL()                              ((bufReadOffset) == (((bufWriteOffset) + 1) % MAX_BUF_NUM) ? 1 : 0)
#define ID_INCREMENT(x)            \
  do {                             \
    (x) = ((x) + 1) % MAX_BUF_NUM; \
  } while (0)

/// Minimum color temperature 800K
#define TEMPERATURE_MIN                         0x0320
/// Maximum color temperature 20000K
#define TEMPERATURE_MAX                         0x4e20
/// Delta UV is hardcoded to 0 in this example
#define DELTA_UV                                0

#define TIMER_ID_RETRANS          10
#define TIMER_ID_FRIEND_FIND      20
#define TIMER_ID_NODE_CONFIGURED  30
#define NO_HANDS  40

/// Timer Frequency used
#define TIMER_CLK_FREQ ((uint32_t)32768)
/// Convert miliseconds to timer ticks
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#define SL_BTMESH_GENERIC_BASE_REGISTRY_INIT_SIZE 0
#define SL_BTMESH_GENERIC_BASE_INCREMENT_CFG_VAL 3

/* Static Variables *************************************************** */

enum {
  noneReq,
  onoffReq,
  lightnessReq,
  ctlReq
};

static pthread_mutex_t commandBufMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t syncFlagMetex = PTHREAD_MUTEX_INITIALIZER;

static char commandBuf[MAX_BUF_NUM][CONSOLE_RX_BUF_SIZE];
static int  bufReadOffset, bufWriteOffset;

static bool hostTargetSynchronized = false;
static bool provisioned = false;
static uint16_t primAddr = 0x0000;
static uint16_t elemIndex = 0xFFFF;

/// Flag for indicating that lpn feature is active
static uint8_t lpn_active = 0;
static uint8_t trid = 0;
/// number of active Bluetooth connections
static uint8_t num_connections = 0;
static uint8_t conn_handle = 0;
static uint8_t switch_pos = 0;
static uint8_t lightness_percent = 0;
static uint8_t colorTemperaturePercent = 0;
static uint16_t lightness_level = 0;
static uint16_t temperature_level = 0;
/// number of on/off requests to be sent
static uint8_t request_count;
static int operation = noneReq;

static pthread_t consoleThreadId, appMainThreadId;

/* Global Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
static int getCMD(void);
static int execCMD(void);
static void consoleInit(void);

static int onOffExec(int argc, const char *argv[]);
static int lightnessExec(int argc, const char *argv[]);
static int ctlExec(int argc, const char *argv[]);
static int usageExec(int argc, const char *argv[]);
static int exitExec(int argc, const char *argv[]);
static int factoryResetExec(int argc, const char *argv[]);

static const CmdItem_t CMDs[] = {
  { "l",
    "\t\t",
    "l [1/0]",
    "\t\t",
    "Set light on[1] or off[0]",
    &onOffExec },

  { "ln",
    "\t\t",
    "ln [0-100]",
    "\t",
    "Set lightness [0% - 100%]",
    &lightnessExec },

  { "ct",
    "\t\t",
    "ct [0-100]",
    "\t",
    "Set Color Temperature [0% - 100%]",
    &ctlExec },

  { "fr",
    "\t\t",
    "fr [1/0]",
    "\t",
    "1 - Factory Reset, 0 - normal reset",
    &factoryResetExec },

  { "h",
    "\t\t",
    "h",
    "\t\t",
    "Print usage",
    &usageExec },

  { "exit",
    "\t\t",
    "exit",
    "\t\t",
    "Exit program",
    &exitExec },
};

#define CMD_NUM()                                     (sizeof(CMDs) / sizeof(CmdItem_t))

void app_init(int argc, char *argv[])
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  sl_status_t sc;
  int opt;

  // Process command line options.
  while ((opt = getopt(argc, argv, OPTSTRING)) != -1) {
    switch (opt) {
      // Print help.
      case 'h':
        app_log(USAGE, argv[0]);
        app_log(OPTIONS);
        exit(EXIT_SUCCESS);

      // Process options for other modules.
      default:
        sc = ncp_host_set_option((char)opt, optarg);
        if (sc != SL_STATUS_OK) {
          app_log(USAGE, argv[0]);
          exit(EXIT_FAILURE);
        }
        break;
    }
  }

  resetSwitchVariables();
  // Initialize NCP connection.
  sc = ncp_host_init();
  if (sc == SL_STATUS_INVALID_PARAMETER) {
    app_log(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }
  app_assert_status(sc);

  SL_BTMESH_API_REGISTER();

  app_log("Empty NCP-host initialised.\r\n");
  app_log("Resetting NCP...\r\n");
  // Reset NCP to ensure it gets into a defined state.
  // Once the chip successfully boots, boot event should be received.
  sl_bt_system_reboot();

  if (-1 == pthread_create(&consoleThreadId,
                            NULL,
                            pConsoleThread,
                            NULL)) {
     perror("Error creating console thread.\r\n");
     exit(1);
  }

  if (-1 == pthread_create(&appMainThreadId,
                           NULL,
                           pAppMainThread,
                           NULL)) {
    perror("Error creating App Main thread.\n");
    exit(1);
  }

  //startTimer(30000000, NO_HANDS);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Deinit.
 *****************************************************************************/
void app_deinit(void)
{
  ncp_host_deinit();

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application deinit code here!                       //
  // This is called once during termination.                                 //
  /////////////////////////////////////////////////////////////////////////////
}

void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      resetSwitchVariables();
      // Print boot message.
      app_log("Bluetooth stack booted: v%d.%d.%d-b%d\r\n",
                   evt->data.evt_system_boot.major,
                   evt->data.evt_system_boot.minor,
                   evt->data.evt_system_boot.patch,
                   evt->data.evt_system_boot.build);

      // Initialize Mesh stack in Node operation mode,
      // wait for initialized event
      app_log("Node init\r\n");
      sc = sl_btmesh_node_init();
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%d] Failed to init node\r\n",
                 (int)sc);
      pthread_mutex_lock(&syncFlagMetex);
      hostTargetSynchronized = true;
      pthread_mutex_unlock(&syncFlagMetex);
      app_log("Node init done\r\n");

      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    case sl_bt_evt_connection_opened_id:
      app_log("evt:sl_bt_evt_connection_opened_id\r\n");
      num_connections++;
      conn_handle = evt->data.evt_connection_opened.connection;

      // turn off lpn feature after GATT connection is opened
      lpn_deinit();
      break;

    case sl_bt_evt_connection_closed_id:
      app_log("evt:conn closed, reason 0x%x\r\n", evt->data.evt_connection_closed.reason);
      conn_handle = 0xFF;
      if (num_connections > 0) {
        if (--num_connections == 0) {
          // initialize lpn when there is no active connection
          lpn_init();
        }
      }
      break;

    case sl_bt_evt_connection_parameters_id:
      app_log("Connection params: interval %d, timeout %d\r\n",
                evt->data.evt_connection_parameters.interval,
                evt->data.evt_connection_parameters.timeout);
      break;

    case sl_bt_evt_advertiser_timeout_id:
      // these events silently discarded
      break;

    default:
      app_log_debug("Unhandled event [0x%08x]\r\n",
                SL_BT_MSG_ID(evt->header));
      break;
    // -------------------------------
    // Default event handler.
  }
}

/**************************************************************************//**
 * Bluetooth Mesh stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth Mesh stack.
 *****************************************************************************/

void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  uint16_t result = 0;
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_node_initialized_id:
    {
      // Initialize generic client models
      sl_btmesh_generic_client_init();

      sl_btmesh_evt_node_initialized_t *pData = (sl_btmesh_evt_node_initialized_t *)&(evt->data);

      if (pData->provisioned) {
        app_log("Node is provisioned. address:%x, ivi:%d\r\n", pData->address, pData->iv_index);

        primAddr = pData->address;
        elemIndex = 0; // index of primary element is zero. This example has only one element.
        provisioned = true;
        switch_node_init();
        // Initialize Low Power Node functionality
        lpn_init();
      } else {
        app_log("Node is unprovisioned\r\n");

        app_log("Starting unprovisioned beaconing...\r\n");
        sl_btmesh_node_start_unprov_beaconing(0x3); // enable ADV and GATT provisioning bearer
      }
    }
    break;

    case sl_btmesh_evt_node_provisioned_id:
      elemIndex = 0; // index of primary element is zero. This example has only one element.
      primAddr = evt->data.evt_node_provisioned.address;
      provisioned = true;
      switch_node_init();
      // try to initialize lpn after 30 seconds, if no configuration messages come
      result = startTimer(30000000, TIMER_ID_NODE_CONFIGURED);
      if (result) {
        app_log("Timer failure?!  %x\r\n", result);
      }
      app_log("Node provisioned, got address=%x\r\n", evt->data.evt_node_provisioned.address);
      break;

    case sl_btmesh_evt_node_provisioning_failed_id:
      app_log_error("Provisioning failed, code %x\r\n", evt->data.evt_node_provisioning_failed.result);
      initiate_factory_reset(0);
      break;

    case sl_btmesh_evt_node_provisioning_started_id:
      app_log("Started provisioning\r\n");
      break;

    case sl_btmesh_evt_node_key_added_id:
      app_log("Got new %s key with index %x\r\n", evt->data.evt_node_key_added.type == 0 ? "network" : "application",
                evt->data.evt_node_key_added.index);

      app_log("Timer added");
      // try to init lpn 5 seconds after adding key
      result = startTimer(5000000, TIMER_ID_NODE_CONFIGURED);
      if (result) {
        app_log("Timer failure?!  %x\r\n", result);
      }
      break;

    case sl_btmesh_evt_node_model_config_changed_id:
    {
      app_log("Model config changed\r\n");
      // try to init lpn 5 seconds after configuration change
      result = startTimer(5000000, TIMER_ID_NODE_CONFIGURED);
      if (result) {
        app_log("Timer failure?! %x\r\n", result);
      }
    }
    break;

    case sl_btmesh_evt_node_reset_id:
      app_log("evt sl_btmesh_evt_node_reset_id\r\n");
      initiate_factory_reset(1);
      break;

    case sl_btmesh_evt_lpn_friendship_established_id:
      app_log("Friendship established\r\n");
      break;

    case sl_btmesh_evt_lpn_friendship_failed_id:
      app_log("Friendship failed\r\n");
      // try again in 2 seconds
      result = startTimer(2000000, TIMER_ID_FRIEND_FIND);
      if (result) {
        app_log("Timer failure?!  %x\r\n", result);
      }
      break;

    case sl_btmesh_evt_lpn_friendship_terminated_id:
      app_log("Friendship terminated\r\n");
      if (num_connections == 0) {
        // try again in 2 seconds
        result = startTimer(2000000, TIMER_ID_FRIEND_FIND);
        if (result) {
          app_log("Timer failure?! %x\r\n", result);
        }
      }
      break;

    default:
      app_log_debug("Unhandled event [0x%08x]\r\n",
                SL_BT_MSG_ID(evt->header));
      break;
    // -------------------------------
    // Default event handler.
  }
}

static void consoleInit(void)
{
  pthread_mutex_lock(&commandBufMutex);
  memset(commandBuf, 0, CONSOLE_RX_BUF_SIZE * MAX_BUF_NUM);
  bufReadOffset = 0;
  bufWriteOffset = 0;
  pthread_mutex_unlock(&commandBufMutex);
}

static inline void resetSwitchVariables(void)
{
  pthread_mutex_lock(&syncFlagMetex);
  hostTargetSynchronized = false;
  pthread_mutex_unlock(&syncFlagMetex);
  provisioned = false;
  lpn_active = 0;
  primAddr = 0x0000;
  elemIndex = 0xFFFF;
  trid = 0;
  num_connections = 0;
  conn_handle = 0xFF;
  switch_pos = 0;
  lightness_percent = 0;
  colorTemperaturePercent = 0;
  lightness_level = 0;
  temperature_level = 0;
}

void initiate_factory_reset(int type)
{
  if (conn_handle != 0xFF) {
    sl_bt_connection_close(conn_handle);
  }

  if (type) {
    sl_bt_nvm_erase_all();
    sleep(1);
  }
}

void switch_node_init(void)
{
  app_log("Mesh lib init\r\n");
  mesh_lib_init(SL_BTMESH_GENERIC_BASE_REGISTRY_INIT_SIZE, SL_BTMESH_GENERIC_BASE_INCREMENT_CFG_VAL);
  app_log("Mesh lib init done\r\n");
}

/***************************************************************************//**
 * Initialize LPN functionality with configuration and friendship establishment.
 ******************************************************************************/
void lpn_init(void)
{
  uint16_t result;

  // Do not initialize LPN if lpn is currently active
  // or any GATT connection is opened
  if (lpn_active || num_connections) {
    return;
  }

  // Initialize LPN functionality.
  result = sl_btmesh_lpn_init();
  if (result) {
    app_log("LPN init failed (0x%x)\r\n", result);
    return;
  }
  lpn_active = 1;
  app_log("LPN initialized\r\n");

  // Configure the lpn with following parameters:
  // - Minimum friend queue length = 2
  // - Poll timeout = 5 seconds
  
  // Configure LPN minimum friend queue length
  result = sl_btmesh_lpn_config(0, 2);
  if (result) {
    app_log("LPN queue configuration failed (0x%hx)\r\n", result);
    return;
  }

  // Configure LPN poll timeout
  result = sl_btmesh_lpn_config(1, 5 * 1000);
  if (result) {
    app_log("LPN poll timeout configuration failed (0x%hx)\r\n", result);
    return;
  }

  app_log("Trying to find friend...\r\n");
  result = sl_btmesh_lpn_establish_friendship(0);

  if (result != 0) {
    app_log("ret.code %x\r\n", result);
  }
}

/***************************************************************************//**
 * Deinitialize LPN functionality.
 ******************************************************************************/
void lpn_deinit(void)
{
  uint16_t result;

  if (!lpn_active) {
    return; // lpn feature is currently inactive
  }

  result = startTimer(0, TIMER_ID_FRIEND_FIND);

  // Terminate friendship if exist
  result = sl_btmesh_lpn_terminate_friendship(0);
  if (result) {
    app_log("Friendship termination failed (0x%x)\r\n", result);
  }
  // turn off lpn feature
  result = sl_btmesh_lpn_deinit();
  if (result) {
    app_log("LPN deinit failed (0x%x)\r\n", result);
  }
  lpn_active = 0;
  app_log("LPN deinitialized\r\n");
}

void *pConsoleThread(void *pIn)
{
  consoleInit();
  for (;;) {
    app_log("$ ");
    getCMD();
    usleep(80 * 1000);
  }
  return NULL;
}

void *pAppMainThread(void *pIn)
{
  for (;;) {
    execCMD();
    if(isTimerElapsed()) {
      timerHandle(getElapsedTimer());
    }
  };

  return NULL;
}

void timerHandle(int handle) {
  uint16_t result = 0;
  switch (handle) {
    case TIMER_ID_RETRANS:
      switch (operation) {
        case onoffReq:
          send_onoff_request(1); // param 1 indicates that this is a retransmission
          break;
        case lightnessReq:
          send_lightness_request(1); // param 1 indicates that this is a retransmission
          break;
        case ctlReq:
          send_ctl_request(1); // param 1 indicates that this is a retransmission
          break;
        default:
          break;
      }
      // stop retransmission timer if it was the last attempt
        app_log("ret.code %d\r\n", request_count);
      if (request_count == 0) {
        startTimer(0, TIMER_ID_RETRANS);
      }
      break;

    case TIMER_ID_NODE_CONFIGURED:
      if (!lpn_active) {
        app_log("Trying to initialize lpn...\r\n");
        lpn_init();
      }
      break;

    case TIMER_ID_FRIEND_FIND:
    {
      app_log("Trying to find friend...\r\n");
      result = sl_btmesh_lpn_establish_friendship(0);

      if (result != 0) {
        app_log("ret.code %x\r\n", result);
      }
    }
    break;

    case NO_HANDS:
    {
      switch_pos = !switch_pos;
      send_onoff_request(0);
      startTimer(30000000, NO_HANDS);
    }
    break;

    default:
      break;
  }
}

static char **splitCmd(char *pIn, int *argc)
{
  int pos = 0;
  char **buf = (char **)malloc(MAX_CONSOLE_PARAM_NUM * sizeof(char *));
  memset(buf, 0, MAX_CONSOLE_PARAM_NUM * sizeof(char *));

  char *p = strtok(pIn,
                   SPLIT_TOKENS);

  while (p) {
    buf[pos++] = p;
    if (pos == MAX_CONSOLE_PARAM_NUM - 1) {
      break;
    }
    p = strtok(NULL, SPLIT_TOKENS);
  }

  *argc = pos;
  return buf;
}

static void outputUsage(void)
{
  app_log("-------------------------------------------------------------\r\n");
  app_log("Command\t\tUsage\t\tDescription\r\n");
  for (int i = 0; i < CMD_NUM(); i++) {
    app_log("%s%s%s%s%s\r\n",
              CMDs[i].command,
              CMDs[i].p1,
              CMDs[i].usage,
              CMDs[i].p2,
              CMDs[i].desc);
  }
  app_log("-------------------------------------------------------------\r\n");
}

static int findCmdLoc(uint8_t argc, const char *argv[])
{
  int pos = 0;

  for (int i = 0; i < argc; i++) {
    app_log_debug("PARAM[%d] = [%s]\r\n",
             i,
             argv[i]);
  }

  for (; pos < CMD_NUM(); pos++) {
    if (!strcmp(argv[0],
                CMDs[pos].command)) {
      break;
    }
  }

  return pos;
}

static int execCMD(void)
{
  char locBuf[CONSOLE_RX_BUF_SIZE];
  char **argv = NULL;
  int valid = 0, argc, err = 0;

  pthread_mutex_lock(&commandBufMutex);
  if (BUF_PENDING()) {
    strcpy(locBuf, commandBuf[bufReadOffset]);
    app_log_debug("Echo CMD [%s]\r\n",
             commandBuf[bufReadOffset]);
    ID_INCREMENT(bufReadOffset);
    valid = 1;
  }
  pthread_mutex_unlock(&commandBufMutex);

  if (!valid) {
    return 0;
  }

  argv = splitCmd(locBuf,
                  &argc);

  if (!argc) {
    err = 0;
    goto out;
  }

  int pos = findCmdLoc(argc, (const char **)argv);

  if (pos == CMD_NUM()) {
    /* CMD not supported */
    app_log_error("CMD not supported.\r\n");
    outputUsage();
    err = 1;
    goto out;
  }

  int ret = CMDs[pos].pCb(argc,
                          (const char **)argv);

  if (ret) {
    app_log_error("[[%s]] CMD failed with error code [0x%04x]\r\n",
             argv[0],
             ret);
    err = 2;
    goto out;
  }

  app_log("[[%s]] CMD success\r\n",
            argv[0]);

  out:
  if (argv) {
    free(argv);
  }
  return err;
}

static int getCMD(void)
{
  char c, locBuf[CONSOLE_RX_BUF_SIZE];
  int err = 0, wOffset = 0;

  memset(locBuf, 0, CONSOLE_RX_BUF_SIZE);
  while ((c = getchar()) != '\n') {
    if (!err) {
      locBuf[wOffset++] = c;
    }

    if ((wOffset == CONSOLE_RX_BUF_SIZE) && !err) {
      /* CMD too long */
      app_log_error("CMD too long\r\n");
      err = 1;
    }
  }

  if (!wOffset) {
    return 0;
  }

  if (!err) {
    pthread_mutex_lock(&commandBufMutex);
    if (BUF_FULL()) {
      /* CMD buffer full */
      app_log_error("CMD buffer full\r\n");
      err = 2;
    } else {
      /* Buffer not full, copy the local content to it */
      locBuf[wOffset] = '\0';
      strcpy(commandBuf[bufWriteOffset], locBuf);
      ID_INCREMENT(bufWriteOffset);
      app_log("New command %d %d\r\n", bufWriteOffset, bufReadOffset);
    }
    pthread_mutex_unlock(&commandBufMutex);
  }

  return err;
}

/***************************************************************************//**
 * This function publishes one generic on/off request to change the state
 * of light(s) in the group. Global variable switch_pos holds the latest
 * desired light state, possible values are:
 * switch_pos = 1 -> PB1 was pressed very long (above 1s), turn lights on
 * switch_pos = 0 -> PB0 was pressed very long (above 1s), turn lights off
 *
 * param[in] retrans  Indicates if this is the first request or a retransmission,
 *                    possible values are 0 = first request, 1 = retransmission.
 *
 * @note This application sends multiple generic on/off requests for each
 *       very long button press to improve reliability.
 *       The transaction ID is not incremented in case of a retransmission.
 ******************************************************************************/
void send_onoff_request(int retrans)
{
  uint16_t resp;
  uint16_t delay;
  struct mesh_generic_request req;
  const uint32_t transtime = 0; /* using zero transition time by default */

  req.kind = mesh_generic_request_on_off;
  req.on_off = switch_pos ? MESH_GENERIC_ON_OFF_STATE_ON : MESH_GENERIC_ON_OFF_STATE_OFF;

  // increment transaction ID for each request, unless it's a retransmission
  if (retrans == 0) {
    trid++;
  }

  operation = onoffReq;
  /* delay for the request is calculated so that the last request will have a zero delay and each
   * of the previous request have delay that increases in 50 ms steps. For example, when using three
   * on/off requests per button press the delays are set as 100, 50, 0 ms
   */
  delay = (request_count - 1) * 50;

  resp = mesh_lib_generic_client_publish(
    MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID,
    elemIndex,
    trid,
    &req,
    transtime,   // transition time in ms
    delay,
    0     // flags
    );

  if (resp) {
    app_log("sl_btmesh_generic_client_publish failed,code %x\r\n", resp);
  } else {
    app_log("Request sent, trid = %u, delay = %d\r\n", trid, delay);
  }

  /* keep track of how many requests has been sent */
  if (request_count > 0) {
    request_count--;
  } else {
    operation = noneReq;
  }
}

/***************************************************************************//**
 * This function publishes one light lightness request to change the lightness
 * level of light(s) in the group. Global variable lightness_level holds
 * the latest desired light level.
 *
 * param[in] retrans  Indicates if this is the first request or a retransmission,
 *                    possible values are 0 = first request, 1 = retransmission.
 ******************************************************************************/
void send_lightness_request(int retrans)
{
  uint16_t resp;
  uint16_t delay;
  struct mesh_generic_request req;

  req.kind = mesh_lighting_request_lightness_actual;
  req.lightness = lightness_level;

  operation = lightnessReq;
  // increment transaction ID for each request, unless it's a retransmission
  if (retrans == 0) {
    trid++;
  }

  /* delay for the request is calculated so that the last request will have a zero delay and each
   * of the previous request have delay that increases in 50 ms steps. For example, when using three
   * on/off requests per button press the delays are set as 100, 50, 0 ms
   */
  delay = (request_count - 1) * 50;

  resp = mesh_lib_generic_client_publish(
    MESH_LIGHTING_LIGHTNESS_CLIENT_MODEL_ID,
    elemIndex,
    trid,
    &req,
    0,     // transition
    delay,
    0     // flags
    );

  if (resp) {
    app_log("sl_btmesh_generic_client_publish failed,code %x\r\n", resp);
  } else {
    app_log("Request sent, trid = %u, delay = %d\r\n", trid, delay);
  }

  /* keep track of how many requests has been sent */
  if (request_count > 0) {
    request_count--;
  } else {
    operation = noneReq;
  }
}

/***************************************************************************//**
 * This function publishes one light CTL request to change the temperature level
 * of light(s) in the group. Global variable temperature_level holds the latest
 * desired light temperature level.
 * The CTL request also send lightness_level which holds the latest desired light
 * lightness level and Delta UV which is hardcoded to 0 for this application.
 *
 * param[in] retrans  Indicates if this is the first request or a retransmission,
 *                    possible values are 0 = first request, 1 = retransmission.
 ******************************************************************************/
void send_ctl_request(int retrans)
{
  uint16_t resp;
  uint16_t delay;
  struct mesh_generic_request req;

  req.kind = mesh_lighting_request_ctl;
  req.ctl.lightness = lightness_level;
  req.ctl.temperature = temperature_level;
  req.ctl.deltauv = DELTA_UV; //hardcoded delta uv

  operation = ctlReq;
  // increment transaction ID for each request, unless it's a retransmission
  if (retrans == 0) {
    trid++;
  }

  /* delay for the request is calculated so that the last request will have a zero delay and each
   * of the previous request have delay that increases in 50 ms steps. For example, when using three
   * on/off requests per button press the delays are set as 100, 50, 0 ms
   */
  delay = (request_count - 1) * 50;

  resp = mesh_lib_generic_client_publish(
    MESH_LIGHTING_CTL_CLIENT_MODEL_ID,
    elemIndex,
    trid,
    &req,
    0,     // transition
    delay,
    0     // flags
    );

  if (resp) {
    app_log("sl_btmesh_generic_client_publish failed,code %x\r\n", resp);
  } else {
    app_log("Request sent, trid = %u, delay = %d\r\n", trid, delay);
  }

  /* keep track of how many requests has been sent */
  if (request_count > 0) {
    request_count--;
  } else {
    operation = noneReq;
  }
}

/*
 * Error code definition for all below command callbacks
 * 0 - success
 * 1 - parameter number mismatch
 * 2 - invalid input parameter(s)
 * 3 - console parameter invalid
 * 4 - node unprovisioned			(4, 5 use wrong state instead?)
 * 5 - ncp not synchronized
 * others - #errorcode_t
 */
static int onOffExec(int argc, const char *argv[])
{
  uint8_t on = 0;

  /* Check if parameter(s) valid */
  if (argc != 2) {
    return 1;
  }

  if (!argv[1]) {
    return 2;
  }

  on = argv[1][0] - '0';

  if ((on != 1) && (on != 0)) {
    outputUsage();
    return 3;
  }

  if (!hostTargetSynchronized) {
    app_log_error("NCP host-target is not synchronized yet.\r\n");
    return 5;
  }

  if (!provisioned) {
    app_log("Node is unprovisioned, need to be provisioned first.\r\n");
    return 4;
  }

  app_log("Set light %s\r\n",
            on == 1 ? "on" : "off");

  switch_pos = on;
  lightness_percent = (switch_pos == 1) ? 100 : 0;

#if 0
  return sl_btmesh_generic_client_publish(MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID,
                                          elemIndex,
                                          trid++,
                                          0,
                                          0,
                                          0,
                                          mesh_generic_request_on_off,
                                          1,
                                          &switch_pos);
#else
  request_count = 3; // request is sent 3 times to improve reliability

  /* send the first request */
  send_onoff_request(0);

  /* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
  startTimer(50000, TIMER_ID_RETRANS);
  return 0;
#endif
}

static int lightnessExec(int argc,
                         const char *argv[])
{
  uint32_t per = 0;

  /* Check if parameter(s) valid */
  if (argc != 2) {
    return 1;
  }

  if (!argv[1]) {
    return 2;
  }

  if (str2uint(argv[1],
               strlen(argv[1]),
               &per) != 0) {
    outputUsage();
    return 3;
  }

  if ((per < 0) || (per > 100)) {
    outputUsage();
    return 3;
  }

  if (!hostTargetSynchronized) {
    app_log_error("NCP host-target is not synchronized yet.\r\n");
    return 5;
  }

  if (!provisioned) {
    app_log("Node is unprovisioned, need to be provisioned first.\r\n");
    return 4;
  }

  app_log("Set lightness to %d%%\r\n",
            (uint8_t)per);

  lightness_percent = (uint8_t)per;
  lightness_level = lightness_percent * 0xFFFF / 100;
#if 0
  return sl_btmesh_generic_client_publish(MESH_LIGHTING_LIGHTNESS_CLIENT_MODEL_ID,
                                          elemIndex,
                                          trid++,
                                          0,
                                          0,
                                          0,
                                          mesh_lighting_request_lightness_actual,
                                          2,
                                          (uint8_t *)&lightness_level);
#else
  request_count = 3; // request is sent 3 times to improve reliability

  /* send the first request */
  send_lightness_request(0);

  /* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
  startTimer(50000, TIMER_ID_RETRANS);
  return 0;
#endif
}

static int ctlExec(int argc,
                   const char *argv[])
{
  uint32_t per = 0;

  /* Check if parameter(s) valid */
  if (argc != 2) {
    return 1;
  }

  if (!argv[1]) {
    return 2;
  }

  if (str2uint(argv[1],
               strlen(argv[1]),
               &per) != 0) {
    outputUsage();
    return 3;
  }

  if ((per < 0) || (per > 100)) {
    outputUsage();
    return 3;
  }

  if (!hostTargetSynchronized) {
    app_log_error("NCP host-target is not synchronized yet.\r\n");
    return 5;
  }

  if (!provisioned) {
    app_log("Node is unprovisioned, need to be provisioned first.\r\n");
    return 4;
  }

  app_log("Set Color Temperature to %d%%\r\n",
            (uint8_t)per);

  colorTemperaturePercent = (uint8_t)per;
  temperature_level = TEMPERATURE_MIN + (colorTemperaturePercent * colorTemperaturePercent / 100) * (TEMPERATURE_MAX - TEMPERATURE_MIN) / 100;

  /* uint16_t tBuf[3] = { lightness_level, temperature_level, DELTA_UV }; */

#if 0
  return sl_btmesh_generic_client_publish(MESH_LIGHTING_CTL_CLIENT_MODEL_ID,
                                          elemIndex,
                                          trid++,
                                          0,
                                          0,
                                          0,
                                          mesh_lighting_request_ctl,
                                          6,
                                          (uint8_t *)&tBuf);
#else

  request_count = 3; // request is sent 3 times to improve reliability

  /* send the first request */
  send_ctl_request(0);

  /* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
  startTimer(50000, TIMER_ID_RETRANS);
  return 0;
#endif
}

static int factoryResetExec(int argc, const char *argv[])
{
  /* Check if parameter(s) valid */
  if (argc != 2) {
    return 1;
  }

  if (!argv[1]) {
    return 2;
  }

  int reset = argv[1][0] - '0';

  if ((reset != 1) && (reset != 0)) {
    outputUsage();
    return 3;
  }

  if (!hostTargetSynchronized) {
    app_log_error("NCP host-target is not synchronized yet.\r\n");
    return 5;
  }

  app_log("%s Resetting...\r\n",
            reset == 1 ? "Factory" : "Normal");

  initiate_factory_reset(reset);
  return 0;
}

static int usageExec(int argc,
                     const char *argv[])
{
  outputUsage();
  return 0;
}

static int exitExec(int argc,
                    const char *argv[])
{
  app_log("Exit program\r\n");
  exit(0);
  return 0;
}

/**
 * @brief	str2int - Convert string to unsigned integer
 *
 * @param	input   - input string value
 * @param	length  - length of the input string
 * @param	p_ret   - Converted integer, if return is not #0, it should be ignored.
 *
 * @return        - success on 0, fail on none-zero
 */
int str2uint(const char *input,
             size_t length,
             uint32_t *p_ret)
{
  int base = 10, ret = 0, tmp, i;
  char *x_ret = NULL;
  x_ret = strstr(input, "0x");
  if (x_ret) {
    base = 16;
    /* Format 0x---... */
    x_ret  += 2;
    length -= 2;
    for (i = length - 1; i >= 0; i--) {
      char tmp_c = x_ret[i];
      if (tmp_c >= '0' && tmp_c <= '9') {
        tmp = tmp_c - '0';
      } else if (tmp_c >= 'a' && tmp_c <= 'f') {
        tmp = tmp_c - 'a' + 10;
      } else if (tmp_c >= 'A' && tmp_c <= 'F') {
        tmp = tmp_c - 'A' + 10;
      } else {
        return 1;
      }
      ret += tmp * pow(base, length - 1 - i);
    }
  } else {
    x_ret = (char *)input;
    base = 10;
    for (i = length - 1; i >= 0; i--) {
      char tmp_c = x_ret[i];
      if (tmp_c >= '0' && tmp_c <= '9') {
        tmp = tmp_c - '0';
      } else {
        return 1;
      }
      ret += tmp * pow(base, length - 1 - i);
    }
  }
  *p_ret = ret;
  return 0;
}

/**
 * @brief	int2str   - Conver unsigned integer to string
 *
 * @param	input			- Integer to convert
 * @param	base_type - Hex or Dec
 * @param	length		- Length of #str[], avoid writing overflow
 * @param	str[]			- Output memory, should be ignore if return value is not #0
 *
 * @return	        - success on 0, fail on none-zero
 */
int uint2str(uint64_t input,
             uint8_t base_type,
             size_t length,
             char str[])
{
  uint64_t ret = 0;
  uint8_t base = 10, remaining = 0, idx = 0;
  /* pvPortMalloc(); */
  if (base_type == BASE_DEC) {
    ret = input / base;
    remaining = input % base;
    while (1) {
      if (idx == length) {
        return 2;
      }
      str[length - 1 - idx++] = '0' + remaining;
      if (!ret) {
        memmove(str, str + length - idx, idx);
        break;
      }
      remaining = ret % base;
      ret = ret / base;
    }
  } else if (base_type == BASE_HEX) {
    base = 16;
    ret = input / base;
    remaining = input % base;
    str[0] = '0';
    str[1] = 'x';
    while (1) {
      if (idx > length - 2) {
        return 2;
      }
      if (remaining < 10) {
        str[length - 1 - idx++] = '0' + remaining;
      } else {
        str[length - 1 - idx++] = 'a' + remaining - 10;
      }
      if (!ret) {
        memmove(str + 2, str + length - idx, idx);
        break;
      }
      remaining = ret % base;
      ret = ret / base;
    }
  } else {
    return 3;
  }
  return 0;
}
