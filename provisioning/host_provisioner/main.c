/*************************************************************************
    > File Name: main_async.c
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

#include "utils.h"
#include "bg_stack/gecko_bglib.h"
#include "config.h"
#include "uart.h"
#include "socket_handler.h"
#include "app_tasks.h"
#include "utest.h"
#include "json_parser.h"

/* Defines  *********************************************************** */

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
/** The serial port to use for BGAPI communication. */
char* uart_port = NULL;

/* Static Functions Declaractions ************************************* */
#if (DEBUG == 0)
static void printUsage(const char *pName);
#endif
static void on_message_send(uint32_t msg_len, uint8_t* msg_data);
static void resetTargetOnExit(void);

int main(int argc, char *argv[])
{
  /* int ret = 0; */
  interfaceArgs_t connArg;

  /*
   * The first parameter determines if to do the utest,
   * the second parameter determines if to exit the program after the utest
   */
  utestAll(0, 1);

  /* uint32_t s = 0x00000000; */
  /* CS_LOG("%d - %d\n", getRightFirstOne(s), getLeftFirstOne(s)); */
  /* s = 0x00000001; */
  /* CS_LOG("%d - %d\n", getRightFirstOne(s), getLeftFirstOne(s)); */
  /* s = 0x00000002; */
  /* CS_LOG("%d - %d\n", getRightFirstOne(s), getLeftFirstOne(s)); */
  /* s = 0x00000003; */
  /* CS_LOG("%d - %d\n", getRightFirstOne(s), getLeftFirstOne(s)); */
  /* sleep(100000); */
  /* char a[5] = { '0', 'x', '1', '2', 0 }; */
  /* printf("%c%c\n", a[0], a[1]); */

  /* jsonTest(); */
  /* sleep(100000); */

#define __EXAMPLE__ 0
#if (__EXAMPLE__ == 1)
  CS_HLE("Test hightlight %s - %s\n", "highlight", "non-highlight");
  CS_HLB("Test hightlight %s - %s\n", "highlight", "non-highlight");

  int ret = 100;
  GET_HL_STR(tmp, "%d", ret);
  dbgPrint("Send Mesh Command Enqueue error - %s\n", hlstr_tmp);
  CS_LOG("Send Mesh Command Enqueue error - %s\n", hlstr_tmp);
  CS_MSG("Send Mesh Command Enqueue error - %s\n", hlstr_tmp);
  CS_ERR("Send Mesh Command Enqueue error - %s\n", hlstr_tmp);
  sleep(10);
#endif
  memset(&connArg, 0, sizeof(interfaceArgs_t));
#if (DEBUG == 0)
  int c = -1;
  if (argc < 3 || !strcmp(argv[1], "--help")) {
    printUsage(argv[0]);
  }
  connArg.configFilePath = INPUT_FILENAME;
  while (-1 != (c = getopt(argc, argv, "m:p:b:s:e:f:"))) {
    switch (c) {
      case 'm':
        if (optarg[0] == 's') {
          connArg.smode = secureEnum;
          connArg.bglib_input = onMessageReceive;
          connArg.bglib_output = onMessageSend;
          connArg.bglib_peek = messagePeek;
        } else if (optarg[0] == 'u') {
          connArg.smode = unsecureEnum;
          connArg.bglib_input = uartRx;
          connArg.bglib_output = on_message_send;
          connArg.bglib_peek = uartRxPeek;
        } else {
          printUsage(argv[0]);
        }
        break;
      case 'p':
        connArg.pSerialPort = optarg;
        break;
      case 'b':
        connArg.baudRate = atoi(optarg);
        break;
      case 's':
        connArg.pSerSockPath = optarg;
        break;
      case 'e':
        connArg.encrypted = atoi(optarg);
        break;
      case 'f':
        connArg.configFilePath = optarg;
        break;
      default:
        break;
    }
  }

  dbgPrint("\n-m %s\n-p %s\n-b %d\n-s %s\n-e %d\n-f %s\n",
           connArg.smode ? "s" : "u",
           connArg.pSerialPort, connArg.baudRate,
           connArg.pSerSockPath, connArg.encrypted,
           connArg.configFilePath);
  if (connArg.smode == noInitEnum
      || (connArg.smode == secureEnum && connArg.pSerSockPath == NULL)
      || (connArg.smode == unsecureEnum && (connArg.pSerialPort == NULL || connArg.baudRate == 0))) {
    printUsage(argv[0]);
  }
#else
  connArg.smode         = unsecureEnum;
  /* connArg.smode         = secureEnum; */
  connArg.pSerialPort   = SERIAL_PORT;
  connArg.baudRate      = BAUD_RATE;
  connArg.pSerSockPath  = SOCKET_PATH;
  connArg.encrypted     = IS_SOCK_ENC;
  connArg.configFilePath  = INPUT_FILENAME;
  connArg.bglib_input = uartRx;
  connArg.bglib_output = on_message_send;
  connArg.bglib_peek = uartRxPeek;
#endif
  /* setbuf(stdout, NULL); */

  /* setupGuardAlarm(guardExpiredHandler); */

  atexit(resetTargetOnExit);

  startAppTasks(&connArg);

  return 0;
}

#if (DEBUG == 0)
static void printUsage(const char *pName)
{
  fprintf(stderr, "Usage: %s -m mode [-p serial_port] [-b baud_rate]"
                  " [-s server_domain_socket_path] [-e is_domain_socket_encrypted? 0/1]"
                  " [-f XML_file_path]\n",
          pName);
  fprintf(stderr, "     -m mode                         s - secure NCP with UNIX domain socket, u - unsecure NCP, connect to UART directly\n");
  fprintf(stderr, "     -p serial_port                  Which UART port to connect to    - [Valid when in 'u' mode]\n");
  fprintf(stderr, "     -b baud_rate                    UART baud rate                   - [Valid when in 'u' mode]\n");
  fprintf(stderr, "     -s server_domain_socket_path    Server UNIX domain socket path   - [Valid when in 's' mode]\n");
  fprintf(stderr, "     -e is_domain_socket_encrypted   1 - encrypted, 0 - not encrypted - [Valid when in 's' mode]\n");
  fprintf(stderr, "     -f XML_file_path                XML node configuration file path\n");

  exit(EXIT_FAILURE);
}
#endif

/**
 * Function called when a message needs to be written to the serial port.
 * @param msg_len Length of the message.
 * @param msg_data Message data, including the header.
 * @param data_len Optional variable data length.
 * @param data Optional variable data.
 */
static void on_message_send(uint32_t msg_len, uint8_t* msg_data)
{
  /** Variable for storing function return values. */
  int ret;

  ret = uartTx(msg_len, msg_data);
  if (ret < 0) {
    CS_ERR("on_message_send() - failed to write to serial port %s, ret: %d, errno: %d\n", uart_port, ret, errno);
    exit(EXIT_FAILURE);
  }
}

static void resetTargetOnExit(void)
{
  gecko_cmd_system_reset(0);
}
