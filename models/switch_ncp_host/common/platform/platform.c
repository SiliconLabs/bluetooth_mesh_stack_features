/***************************************************************************//**
 * @file
 * @brief platform.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include COMMON_HEADER
#include "platform.h"

//// The serial port to use for BGAPI communication.
char* uart_port = NULL;

/// The baud rate to use.
uint32_t baud_rate = 0;

/// dfu file to upload
FILE *dfu_file = NULL;

void on_message_send(uint32 msg_len, uint8* msg_data)
{
  /// Variable for storing function return values.
  int ret;

#if DEBUG
  printf("on_message_send()\n");
#endif // DEBUG

  ret = uartTx(msg_len, msg_data);
  if (ret < 0) {
    printf("on_message_send() - failed to write to serial port %s, ret: %d, errno: %d\n", uart_port, ret, errno);
    exit(EXIT_FAILURE);
  }
}

void print_address(bd_addr address)
{
  for (int i = 5; i >= 0; i--) {
    printf("%02x", address.addr[i]);
    if (i > 0) {
      printf(":");
    }
  }
}

int parse_address(const char *str, bd_addr *addr)
{
  int a[6];
  int i;
  i = sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
             &a[5],
             &a[4],
             &a[3],
             &a[2],
             &a[1],
             &a[0]
             );
  if (i != 6) {
    return -1;
  }

  for (i = 0; i < 6; i++) {
    addr->addr[i] = (uint8_t)a[i];
  }

  return 0;
}

int hw_init(int argc, char* argv[])
{
#if defined(BLE_NCP_MOBILE_APP)
  if (argc < 4)
#else
  if (argc < 3)
#endif
  {
    printf(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }

#if defined(BLE_NCP_MOBILE_APP)
  //filename
  dfu_file = fopen(argv[3], "rb");
  if (dfu_file == NULL) {
    printf("cannot open file %s\n", argv[3]);
    exit(-1);
  }
#endif

  baud_rate = atoi(argv[2]);
  uart_port = argv[1];

  // Initialise the serial port.
  return uartOpen((int8_t*)uart_port, baud_rate, 1, 100);
}
