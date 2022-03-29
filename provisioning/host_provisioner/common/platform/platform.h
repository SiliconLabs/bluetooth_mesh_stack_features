/***************************************************************************//**
 * @file
 * @brief platform.h
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

#ifndef __PLATFORM__
#define __PLATFORM__

/// Usage string
#if defined(BLE_NCP_MOBILE_APP)
#define USAGE "Usage: %s [serial port] [baud rate] [ota file]\n\n"
#else
#define USAGE "Usage: %s [serial port] [baud rate]\n\n"
#endif

/// The serial port to use for BGAPI communication.
extern char* uart_port;

/// The baud rate to use.
extern uint32_t baud_rate;

/// dfu file to upload
extern FILE *dfu_file;

void on_message_send(uint32 msg_len, uint8* msg_data);
void print_address(bd_addr address);
int parse_address(const char *str, bd_addr *addr);
int hw_init(int argc, char* argv[]);

#endif // __PLATFORM__
