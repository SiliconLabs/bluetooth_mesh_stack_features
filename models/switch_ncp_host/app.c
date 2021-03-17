/*************************************************************************
    > File Name: app.c
    > Author: Kevin
    > Created Time: 2019-01-09
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "gecko_bglib.h"
#include "mesh_generic_model_capi_types.h"
#include "mesh_lighting_model_capi_types.h"

#include "mesh_lib.h"
#include "app.h"
#include "uart.h"

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

/// Timer Frequency used
#define TIMER_CLK_FREQ ((uint32)32768)
/// Convert miliseconds to timer ticks
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)
/* Static Variables *************************************************** */
BGLIB_DEFINE();

enum {
  noneReq,
  onoffReq,
  lightnessReq,
  ctlReq
};

static pthread_t synchronizeThreadId;
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
static uint8 request_count;
static int operation = noneReq;

/* Global Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
static int getCMD(void);
static int execCMD(void);
static void ncpEvtHandler(void);
static void *pSyncThread(void *pIn);

void send_onoff_request(int retrans);
void send_lightness_request(int retrans);
void send_ctl_request(int retrans);

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

void switch_node_init(void)
{
  mesh_lib_init(malloc, free, 8);
}

/***************************************************************************//**
 * Initialize LPN functionality with configuration and friendship establishment.
 ******************************************************************************/
void lpn_init(void)
{
  uint16 result;

  // Do not initialize LPN if lpn is currently active
  // or any GATT connection is opened
  if (lpn_active || num_connections) {
    return;
  }

  // Initialize LPN functionality.
  result = gecko_cmd_mesh_lpn_init()->result;
  if (result) {
    CS_OUTPUT("LPN init failed (0x%x)\r\n", result);
    return;
  }
  lpn_active = 1;
  CS_OUTPUT("LPN initialized\r\n");

  // Configure the lpn with following parameters:
  // - Minimum friend queue length = 2
  // - Poll timeout = 5 seconds
  result = gecko_cmd_mesh_lpn_configure(2, 5 * 1000)->result;
  if (result) {
    CS_OUTPUT("LPN conf failed (0x%x)\r\n", result);
    return;
  }

  CS_OUTPUT("trying to find friend...\r\n");
  result = gecko_cmd_mesh_lpn_establish_friendship(0)->result;

  if (result != 0) {
    CS_OUTPUT("ret.code %x\r\n", result);
  }
}

/***************************************************************************//**
 * Deinitialize LPN functionality.
 ******************************************************************************/
void lpn_deinit(void)
{
  uint16 result;

  if (!lpn_active) {
    return; // lpn feature is currently inactive
  }

  result = gecko_cmd_hardware_set_soft_timer(0, // cancel friend finding timer
                                             TIMER_ID_FRIEND_FIND,
                                             1)->result;

  // Terminate friendship if exist
  result = gecko_cmd_mesh_lpn_terminate_friendship()->result;
  if (result) {
    CS_OUTPUT("Friendship termination failed (0x%x)\r\n", result);
  }
  // turn off lpn feature
  result = gecko_cmd_mesh_lpn_deinit()->result;
  if (result) {
    CS_OUTPUT("LPN deinit failed (0x%x)\r\n", result);
  }
  lpn_active = 0;
  CS_OUTPUT("LPN deinitialized\r\n");
}

static void appMainInit(void)
{
  resetSwitchVariables();
  /**
   * Initialize BGLIB with our output function for sending messages.
   */
  BGLIB_INITIALIZE_NONBLOCK(on_message_send, uartRx, uartRxPeek);

  if (-1 == pthread_create(&synchronizeThreadId,
                           NULL,
                           pSyncThread,
                           NULL)) {
    perror("Error creating sync thread.\n");
    exit(1);
  }
}

void *pConsoleThread(void *pIn)
{
  consoleInit();
  for (;;) {
    CS_OUTPUT("$ ");
    getCMD();
    usleep(80 * 1000);
  }
  return NULL;
}

void *pAppMainThread(void *pIn)
{
  appMainInit();
  for (;;) {
    execCMD();
    ncpEvtHandler();
  }
  return NULL;
}

static void *pSyncThread(void *pIn)
{
  struct gecko_cmd_packet *p;
  pthread_detach(pthread_self());

  CS_OUTPUT("Syncing");

  do {
    CS_OUTPUT("."); fflush(stdout);

    p = gecko_peek_event();
    if (p) {
      switch (BGLIB_MSG_ID(p->header)) {
        case gecko_evt_system_boot_id:
        {
          CS_OUTPUT("System booted. NCP target sync up\n");
          // Initialize Mesh stack in Node operation mode, it will generate initialized event
          uint16_t result = gecko_cmd_mesh_node_init()->result;
          if (result) {
            CS_ERROR("init failed (0x%x)", result);
            exit(1);
          }
          pthread_mutex_lock(&syncFlagMetex);
          hostTargetSynchronized = true;
          pthread_mutex_unlock(&syncFlagMetex);
        }
        break;
        default:
          CS_OUTPUT("Unexpected event [ID:%08x]\n", BGLIB_MSG_ID(p->header));
          break;
      }
    } else {
      gecko_cmd_system_reset(0);
      sleep(1);
    }

    if (hostTargetSynchronized) {
      break;
    }
  } while (1);

  pthread_exit(NULL);
  return NULL;
}

static void initiate_factory_reset(int type)
{
  if (conn_handle != 0xFF) {
    gecko_cmd_le_connection_close(conn_handle);
  }

  if (type) {
    gecko_cmd_flash_ps_erase_all();
    sleep(1);
  }

  appMainInit();
}

static void ncpEvtHandler(void)
{
  struct gecko_cmd_packet *evt = NULL;
  uint16_t result = 0;

  if (!hostTargetSynchronized) {
    return;
  }
  do {
    if ((evt = gecko_peek_event()) == NULL) {
      return;
    }

    switch (BGLIB_MSG_ID(evt->header)) {
      case gecko_evt_system_boot_id:
      {
        CS_OUTPUT("Target reset autonomously, boot\n");
        resetSwitchVariables();
        // Initialize Mesh stack in Node operation mode, it will generate initialized event
        result = gecko_cmd_mesh_node_init()->result;
        if (result) {
          CS_ERROR("init failed (0x%x)", result);
          exit(1);
        }
        return;
      }
      break;

      case gecko_evt_mesh_node_initialized_id:
      {
        CS_OUTPUT("node initialized\r\n");

        // Initialize generic client models
        gecko_cmd_mesh_generic_client_init();

        struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);

        if (pData->provisioned) {
          CS_OUTPUT("node is provisioned. address:%x, ivi:%d\r\n", pData->address, pData->ivi);

          primAddr = pData->address;
          elemIndex = 0; // index of primary element is zero. This example has only one element.
          provisioned = true;
          switch_node_init();
          // Initialize Low Power Node functionality
          lpn_init();
        } else {
          CS_OUTPUT("node is unprovisioned\r\n");

          CS_OUTPUT("starting unprovisioned beaconing...\r\n");
          gecko_cmd_mesh_node_start_unprov_beaconing(0x3); // enable ADV and GATT provisioning bearer
        }
      }
      break;

      case gecko_evt_mesh_node_provisioned_id:
        elemIndex = 0; // index of primary element is zero. This example has only one element.
        primAddr = evt->data.evt_mesh_node_provisioned.address;
        provisioned = true;
        switch_node_init();
        // try to initialize lpn after 30 seconds, if no configuration messages come
        result = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(30000),
                                                   TIMER_ID_NODE_CONFIGURED,
                                                   1)->result;
        if (result) {
          CS_OUTPUT("timer failure?!  %x\r\n", result);
        }
        CS_OUTPUT("node provisioned, got address=%x\r\n", evt->data.evt_mesh_node_provisioned.address);
        break;

      case gecko_evt_mesh_node_provisioning_failed_id:
        CS_ERROR("provisioning failed, code %x\r\n", evt->data.evt_mesh_node_provisioning_failed.result);
        initiate_factory_reset(0);
        break;

      case gecko_evt_mesh_node_provisioning_started_id:
        CS_OUTPUT("Started provisioning\r\n");
        break;

      case gecko_evt_hardware_soft_timer_id:
        /* CS_OUTPUT("soft_timer\n"); */
        switch (evt->data.evt_hardware_soft_timer.handle) {
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
            if (request_count == 0) {
              gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_RETRANS, 0);
            }
            break;

          case TIMER_ID_NODE_CONFIGURED:
            if (!lpn_active) {
              CS_OUTPUT("try to initialize lpn...\r\n");
              lpn_init();
            }
            break;

          case TIMER_ID_FRIEND_FIND:
          {
            CS_OUTPUT("trying to find friend...\r\n");
            result = gecko_cmd_mesh_lpn_establish_friendship(0)->result;

            if (result != 0) {
              CS_OUTPUT("ret.code %x\r\n", result);
            }
          }
          break;

          default:
            break;
        }

        break;

      case gecko_evt_mesh_node_key_added_id:
        CS_OUTPUT("got new %s key with index %x\r\n", evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
                  evt->data.evt_mesh_node_key_added.index);

        // try to init lpn 5 seconds after adding key
        result = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(5000),
                                                   TIMER_ID_NODE_CONFIGURED,
                                                   1)->result;
        if (result) {
          CS_OUTPUT("timer failure?!  %x\r\n", result);
        }
        break;

      case gecko_evt_mesh_node_model_config_changed_id:
      {
        CS_OUTPUT("model config changed\r\n");
        // try to init lpn 5 seconds after configuration change
        result = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(5000),
                                                   TIMER_ID_NODE_CONFIGURED,
                                                   1)->result;
        if (result) {
          CS_OUTPUT("timer failure?!  %x\r\n", result);
        }
      }
      break;

      case gecko_evt_le_connection_opened_id:
        CS_OUTPUT("evt:gecko_evt_le_connection_opened_id\r\n");
        num_connections++;
        conn_handle = evt->data.evt_le_connection_opened.connection;

        // turn off lpn feature after GATT connection is opened
        lpn_deinit();
        break;

      case gecko_evt_le_connection_closed_id:
        CS_OUTPUT("evt:conn closed, reason 0x%x\r\n", evt->data.evt_le_connection_closed.reason);
        conn_handle = 0xFF;
        if (num_connections > 0) {
          if (--num_connections == 0) {
            // initialize lpn when there is no active connection
            lpn_init();
          }
        }
        break;

      case gecko_evt_mesh_node_reset_id:
        CS_OUTPUT("evt gecko_evt_mesh_node_reset_id\r\n");
        initiate_factory_reset(1);
        break;

      case gecko_evt_le_connection_parameters_id:
        CS_OUTPUT("connection params: interval %d, timeout %d\r\n",
                  evt->data.evt_le_connection_parameters.interval,
                  evt->data.evt_le_connection_parameters.timeout);
        break;

      case gecko_evt_le_gap_adv_timeout_id:
        // these events silently discarded
        break;

      case gecko_evt_mesh_lpn_friendship_established_id:
        CS_OUTPUT("friendship established\r\n");
        break;

      case gecko_evt_mesh_lpn_friendship_failed_id:
        CS_OUTPUT("friendship failed\r\n");
        // try again in 2 seconds
        result = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(2000),
                                                   TIMER_ID_FRIEND_FIND,
                                                   1)->result;
        if (result) {
          CS_OUTPUT("timer failure?!  %x\r\n", result);
        }
        break;

      case gecko_evt_mesh_lpn_friendship_terminated_id:
        CS_OUTPUT("friendship terminated\r\n");
        if (num_connections == 0) {
          // try again in 2 seconds
          result = gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(2000),
                                                     TIMER_ID_FRIEND_FIND,
                                                     1)->result;
          if (result) {
            CS_OUTPUT("timer failure?!  %x\r\n", result);
          }
        }
        break;

      default:
        dbgPrint("Unhandled event [0x%08x]\n",
                 BGLIB_MSG_ID(evt->header));
        break;
    }
  } while (evt);
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
  CS_OUTPUT("-------------------------------------------------------------\n");
  CS_OUTPUT("Command\tUsage\t\tDescription\n");
  for (int i = 0; i < CMD_NUM(); i++) {
    CS_OUTPUT("%s%s%s%s%s\n",
              CMDs[i].command,
              CMDs[i].p1,
              CMDs[i].usage,
              CMDs[i].p2,
              CMDs[i].desc);
  }
  CS_OUTPUT("-------------------------------------------------------------\n");
}

static int findCmdLoc(uint8_t argc, const char *argv[])
{
  int pos = 0;

  for (int i = 0; i < argc; i++) {
    dbgPrint("PARAM[%d] = [%s]\n",
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
    dbgPrint("Echo CMD [%s]\n",
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
    CS_ERROR("CMD not supported.\n");
    outputUsage();
    err = 1;
    goto out;
  }

  int ret = CMDs[pos].pCb(argc,
                          (const char **)argv);

  if (ret) {
    CS_ERROR("[[%s]] CMD failed with error code [0x%04x]\n",
             argv[0],
             ret);
    err = 2;
    goto out;
  }

  CS_OUTPUT("[[%s]] CMD success\n",
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
      CS_ERROR("CMD too long\n");
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
      CS_ERROR("CMD buffer full\n");
      err = 2;
    } else {
      /* Buffer not full, copy the local content to it */
      locBuf[wOffset] = '\0';
      strcpy(commandBuf[bufWriteOffset], locBuf);
      ID_INCREMENT(bufWriteOffset);
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
  uint16 resp;
  uint16 delay;
  struct mesh_generic_request req;
  const uint32 transtime = 0; /* using zero transition time by default */

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
    CS_OUTPUT("gecko_cmd_mesh_generic_client_publish failed,code %x\r\n", resp);
  } else {
    CS_OUTPUT("request sent, trid = %u, delay = %d\r\n", trid, delay);
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
  uint16 resp;
  uint16 delay;
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
    CS_OUTPUT("gecko_cmd_mesh_generic_client_publish failed,code %x\r\n", resp);
  } else {
    CS_OUTPUT("request sent, trid = %u, delay = %d\r\n", trid, delay);
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
  uint16 resp;
  uint16 delay;
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
    CS_OUTPUT("gecko_cmd_mesh_generic_client_publish failed,code %x\r\n", resp);
  } else {
    CS_OUTPUT("request sent, trid = %u, delay = %d\r\n", trid, delay);
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
    CS_ERROR("NCP host-target is not synchronized yet.\n");
    return 5;
  }

  if (!provisioned) {
    CS_OUTPUT("Node is unprovisioned, need to be provisioned first.\n");
    return 4;
  }

  CS_OUTPUT("Set light %s\n",
            on == 1 ? "on" : "off");

  switch_pos = on;
  lightness_percent = (switch_pos == 1) ? 100 : 0;

#if 0
  return gecko_cmd_mesh_generic_client_publish(MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID,
                                               elemIndex,
                                               trid++,
                                               0,
                                               0,
                                               0,
                                               mesh_generic_request_on_off,
                                               1,
                                               &switch_pos)->result;
#else
  request_count = 3; // request is sent 3 times to improve reliability

  /* send the first request */
  send_onoff_request(0);

  /* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(50), TIMER_ID_RETRANS, 0);
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
    CS_ERROR("NCP host-target is not synchronized yet.\n");
    return 5;
  }

  if (!provisioned) {
    CS_OUTPUT("Node is unprovisioned, need to be provisioned first.\n");
    return 4;
  }

  CS_OUTPUT("Set lightness to %d%%\n",
            (uint8_t)per);

  lightness_percent = (uint8_t)per;
  lightness_level = lightness_percent * 0xFFFF / 100;
#if 0
  return gecko_cmd_mesh_generic_client_publish(MESH_LIGHTING_LIGHTNESS_CLIENT_MODEL_ID,
                                               elemIndex,
                                               trid++,
                                               0,
                                               0,
                                               0,
                                               mesh_lighting_request_lightness_actual,
                                               2,
                                               (uint8_t *)&lightness_level)->result;
#else
  request_count = 3; // request is sent 3 times to improve reliability

  /* send the first request */
  send_lightness_request(0);

  /* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(50), TIMER_ID_RETRANS, 0);
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
    CS_ERROR("NCP host-target is not synchronized yet.\n");
    return 5;
  }

  if (!provisioned) {
    CS_OUTPUT("Node is unprovisioned, need to be provisioned first.\n");
    return 4;
  }

  CS_OUTPUT("Set Color Temperature to %d%%\n",
            (uint8_t)per);

  colorTemperaturePercent = (uint8_t)per;
  temperature_level = TEMPERATURE_MIN + (colorTemperaturePercent * colorTemperaturePercent / 100) * (TEMPERATURE_MAX - TEMPERATURE_MIN) / 100;

  /* uint16_t tBuf[3] = { lightness_level, temperature_level, DELTA_UV }; */

#if 0
  return gecko_cmd_mesh_generic_client_publish(MESH_LIGHTING_CTL_CLIENT_MODEL_ID,
                                               elemIndex,
                                               trid++,
                                               0,
                                               0,
                                               0,
                                               mesh_lighting_request_ctl,
                                               6,
                                               (uint8_t *)&tBuf)->result;
#else

  request_count = 3; // request is sent 3 times to improve reliability

  /* send the first request */
  send_ctl_request(0);

  /* start a repeating soft timer to trigger re-transmission of the request after 50 ms delay */
  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(50), TIMER_ID_RETRANS, 0);
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
    CS_ERROR("NCP host-target is not synchronized yet.\n");
    return 5;
  }

  CS_OUTPUT("%s Resetting...\n",
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
  CS_OUTPUT("Exit program\n");
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
