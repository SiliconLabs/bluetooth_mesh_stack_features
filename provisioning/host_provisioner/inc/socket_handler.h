/***************************************************************************//**
 * @file
 * @brief socket_handler.h
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

#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

/* Max number of bytes in receive buffer */
#define MAX_PACKET_SIZE 512

/**
 * Function to connect to a server domain socket.
 * @param fnameServer Name of Server domain socket.
 * @param fnameClient Name of Client domain socket.
 * @param encrypted indicates whether the ncp daemon encrypts data going through that socket.
 *  \return  0 on success, -1 on failure.
 */
int connect_domain_socket_server(char *fnameServer, char *fnameClient, int encrypted);

/**
 * Function to be called when a BGAPI message is to be sent out to a domain socket.
 * @param msg_len Name Number of bytes to send.
 * @param msg_data Pointer to bytes to send.
 */
void onMessageSend(uint32_t msg_len, uint8_t* msg_data);

/**
 * Function to be called when a BGAPI message is to be received from a domain socket.
 * @param msg_len Name Number of bytes to received.
 * @param msg_data Pointer to bytes to receive.
 */
int32_t onMessageReceive(uint32_t msg_len, uint8_t* msg_data);

/**
 * Function to determine whether there is new data in a socket.
 *  \return  0 if there is no data, any other value indicates the number of sockets with new data
 */
int32_t messagePeek(void);

/**
 * Function to turn on encryption.
 *  \return  0 if there is no data, any other value indicates the number of sockets with new data
 */
void turnEncryptionOn(void);

/**
 * Function to turn off encryption.
 *  \return  0 if there is no data, any other value indicates the number of sockets with new data
 */
void turnEncryptionOff(void);

/**
 * Function to read data from the domain socket if there is any.
 * @param timeout timeout to wait for data to become available, if there isn't any.
 */
void poll_update(int timeout);

#endif
