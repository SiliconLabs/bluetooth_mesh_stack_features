/***************************************************************************//**
 * @file
 * @brief socket_handler.c
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/param.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <poll.h>
#include "socket_handler.h"

static int enc_client_socket = -1;
static int unenc_client_socket = -1;
static bool encrypted = false;

static uint8_t buf[MAX_PACKET_SIZE];
static uint8_t* bufPointer;
static uint8_t unhandledDataSize;

static struct pollfd pollStruct;

static int readDomainSocket(int fd, int revents);

int connect_domain_socket_server(char *fnameServer, char *fnameClient, int encrypted)
{
  int len, rc;
  struct sockaddr_un server_sockaddr;
  struct sockaddr_un client_sockaddr;
  static int * client_socket;

  if (encrypted) {
    client_socket = &enc_client_socket;
  } else {
    client_socket = &unenc_client_socket;
  }

  //create socket
  memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
  memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

  // Create UNIX domain stream socket for the Client
  *client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
  if (*client_socket == -1) {
    printf("Socket Error: %d\n", errno);
    return -1;
  }

  client_sockaddr.sun_family = AF_UNIX;
  strcpy(client_sockaddr.sun_path, fnameClient);
  len = sizeof(client_sockaddr);

  // Add filepath to UNIX sockaddr structure, unlink (delete
  // any existing socket with same path) and bind to socket
  unlink(fnameClient);
  rc = bind(*client_socket, (struct sockaddr *) &client_sockaddr, len);
  if (rc == -1) {
    printf("BIND ERROR: %d\n", errno);
    close(*client_socket);
    return -1;
  }

  // Create UNIX domain stream socket for the Server
  server_sockaddr.sun_family = AF_UNIX;
  strcpy(server_sockaddr.sun_path, fnameServer);
  rc = connect(*client_socket, (struct sockaddr *) &server_sockaddr, len);
  if (rc == -1) {
    printf("CONNECT ERROR = %d (%s)\n", errno, strerror(errno));
    close(*client_socket);
    return -1;
  }

  /* Update poll structure with client domain socket file descriptor */
  pollStruct.fd = *client_socket;
  pollStruct.events = POLLIN;
  pollStruct.revents = 0;

  return 0;
}

void onMessageSend(uint32_t msg_len, uint8_t* msg_data)
{
  if (encrypted) {
    send(enc_client_socket, msg_data, msg_len, 0);
  } else {
    send(unenc_client_socket, msg_data, msg_len, 0);
  }
}

int32_t onMessageReceive(uint32_t msg_len, uint8_t* msg_data)
{
  poll_update(50);

  if (unhandledDataSize > 0) {
    if (unhandledDataSize >= msg_len) {
      unhandledDataSize -= msg_len;
      for (int i = 0; i < msg_len; i++) {
        *(msg_data + i) = *bufPointer;
        bufPointer++;
      }
      return msg_len;
    } else {
      unhandledDataSize = 0;
      return -1;
    }
  }
  return -1;
}

int32_t messagePeek()
{
  return unhandledDataSize > 0;
}

void turnEncryptionOn(void)
{
  encrypted = true;
}

void turnEncryptionOff(void)
{
  encrypted = false;
}

void poll_update(int timeout)
{
  poll(&pollStruct, 1, timeout == 0 ? -1 : timeout);
  if (pollStruct.revents != 0) {
    readDomainSocket(pollStruct.fd, pollStruct.revents);
  }
}

//this callback is called when file descriptor has an event
static int readDomainSocket(int fd, int revents)
{
  if ((!unhandledDataSize) && ((revents & POLLIN) && ((fd == enc_client_socket) || (fd == unenc_client_socket)))) {
    //receive data after the packet
    bufPointer = buf;
    int len = recv(fd, buf, sizeof(buf), 0);

    if (len == 0) {
      //socket closed
      printf("Host unencrypted disconnected\n");
      close(fd);
      if (fd == enc_client_socket) {
        enc_client_socket = -1;
      } else {
        unenc_client_socket = -1;
      }
      unhandledDataSize = 0;
      return -1;
    } else if (len < 0) {
      unhandledDataSize = 0;
      return 0;
    } else {
      unhandledDataSize = len;
      return len;
    }
  }
  return 0;
}
