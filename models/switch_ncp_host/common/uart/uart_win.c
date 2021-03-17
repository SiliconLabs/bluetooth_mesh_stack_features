/***************************************************************************//**
 * @file
 * @brief UART implementation for windows platform
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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <errno.h>
#include <unistd.h>

#include "uart.h"

#if _WIN32 != 1 && __CYGWIN__ != 1
#error "**** Unsupported OS! This UART driver works on Windows and Cygwin only! ****"
#endif /* _WIN32 != 1 && __CYGWIN__ != 1 */

#ifndef CBR_460800
  #define CBR_460800 460800
#endif /* CBR_460800 */

#ifndef CBR_921600
  #define CBR_921600 921600
#endif /* CBR_921600 */

/***************************************************************************************************
 * Local Variables
 **************************************************************************************************/

static struct {
  uint32_t cbaud;
  uint32_t nspeed;
} speedTab[] = {
  { CBR_300, 300    },
  { CBR_1200, 1200   },
  { CBR_2400, 2400   },
  { CBR_4800, 4800   },
  { CBR_9600, 9600   },
  { CBR_19200, 19200  },
  { CBR_38400, 38400  },
  { CBR_57600, 57600  },
  { CBR_115200, 115200 },
  { CBR_256000, 256000 },
  { CBR_460800, 460800 },
  { CBR_921600, 921600 },
  { 0, 0 }
};

/* Handle of the serial port. */
static HANDLE serialHandle = INVALID_HANDLE_VALUE;

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/
static HANDLE uartOpenSerial(int8_t* device, uint32_t bps, uint32_t dataBits, uint32_t parity,
                             uint32_t stopBits, uint32_t rtsCts, uint32_t xOnXOff, int32_t timeout);
static int32_t uartCloseSerial(HANDLE handle);

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/
int32_t uartOpen(int8_t* port, uint32_t baudRate, uint32_t rtsCts, int32_t timeout)
{
  uint8_t buf[4];

  serialHandle = uartOpenSerial(port, baudRate, 8, 0, 1, rtsCts, 0, timeout);

  if (INVALID_HANDLE_VALUE == serialHandle) {
    return -1;
  }

  // Flush all accumulated data in target
  usleep(50000);
  while (uartRxNonBlocking(4, buf) == 4) {
  }

  return 0;
}

int32_t uartClose(void)
{
  return (uartCloseSerial(serialHandle) == 0) ? -1 : 0;
}

int32_t uartRx(uint32_t dataLength, uint8_t* data)
{
  /** Variable for storing function return values. */
  DWORD ret;
  /** The amount of bytes still needed to be read. */
  DWORD dataToRead = dataLength;
  /** The amount of bytes read. */
  DWORD dataRead;

  if (serialHandle == INVALID_HANDLE_VALUE) {
    return -1;
  }

  while (dataToRead) {
    ret = ReadFile(serialHandle, (LPVOID)data, dataToRead, (LPDWORD)&dataRead, NULL);
    if (!ret) {
      ret = GetLastError();
      if (ret == ERROR_SUCCESS) {
        continue;
      }
      return -1;
    } else {
      if (!dataRead) {
        continue;
      }
    }
    dataToRead -= dataRead;
    data += dataRead;
  }

  return (int32_t)dataLength;
}

int32_t uartRxNonBlocking(uint32_t dataLength, uint8_t* data)
{
  /** Variable for storing function return values. */
  DWORD ret;
  /** The amount of bytes read. */
  DWORD dataRead;

  if (serialHandle == INVALID_HANDLE_VALUE) {
    return -1;
  }

  ret = ReadFile(serialHandle, (LPVOID)data, (DWORD)dataLength, (LPDWORD)&dataRead, NULL);
  if (!ret) {
    ret = GetLastError();
    printf("GetLastError: %ld\n", ret);
    if (ret != ERROR_SUCCESS) {
      return -1;
    }
  }

  return (int32_t)dataRead;
}

int32_t uartRxPeek(void)
{
  COMSTAT comStat = { 0 };
  DWORD err = 0;

  if (serialHandle == INVALID_HANDLE_VALUE) {
    return -1;
  }

  if (!ClearCommError(serialHandle, &err, &comStat) || err) {
    return -1;
  }

  return (int32_t)comStat.cbInQue;
}

int32_t uartTx(uint32_t dataLength, uint8_t* data)
{
  /** Variable for storing function return values. */
  DWORD ret;
  /** The amount of bytes written. */
  DWORD dataWritten;
  /** The amount of bytes still needed to be written. */
  DWORD dataToWrite = dataLength;

  if (serialHandle == INVALID_HANDLE_VALUE) {
    return -1;
  }

  while (dataToWrite) {
    ret = WriteFile(serialHandle, (LPCVOID)data, dataToWrite, (LPDWORD)&dataWritten, NULL);
    if (!ret) {
      return -1;
    }
    dataToWrite -= dataWritten;
    data += dataWritten;
  }
  FlushFileBuffers(serialHandle);

  return (int32_t)dataLength;
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Open a serial port.
 *  \param[in] device Serial Port number.
 *  \param[in] bps Baud Rate.
 *  \param[in] dataBits Number of databits.
 *  \param[in] parity Parity bit used.
 *  \param[in] stopBits Stop bits used.
 *  \param[in] rtsCts Hardware handshaking used.
 *  \param[in] xOnXOff Software Handshaking used.
 *  \param[in] timeout Block until a character is received or for timeout milliseconds. If
 *             timeout < 0, block until character is received, there is no timeout.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
static HANDLE uartOpenSerial(int8_t* device, uint32_t bps, uint32_t dataBits, uint32_t parity,
                             uint32_t stopBits, uint32_t rtsCts, uint32_t xOnXOff, int32_t timeout)
{
  uint32_t i;
  HANDLE serial = INVALID_HANDLE_VALUE;
  char deviceStr[60] = { 0 };
  DCB settings = { 0 };
  COMMTIMEOUTS commTimeouts = { 0 };

  /* Check if baud rate is supported. Return -1 if not. */
  for (i = 0; speedTab[i].nspeed != 0; i++) {
    if (bps == speedTab[i].nspeed) {
      break;
    }
  }
  if (speedTab[i].nspeed == 0) {
    fprintf(stderr, "Baud rate not supported!\n");
    goto error;
  }

  /* Open the serial port for read/write with exclusive access, default security attributes and not
   * overlapped I/O. */
  snprintf(deviceStr, sizeof(deviceStr) - 1, "\\\\.\\%s", (char*)device);
  serial = CreateFileA(deviceStr, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (serial == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Error opening serial port %s - %ld.\n",
            (char*)device,
            GetLastError());
    goto error;
  }

  /* Retrieve all current communication settings. */
  settings.DCBlength = sizeof(DCB);
  if (!GetCommState(serial, &settings)) {
    fprintf(stderr, "Error getting comm state on %s - %ld.\n",
            (char*)device,
            GetLastError());
    goto error;
  }

  /* Configure baud rate. */
  settings.BaudRate = speedTab[i].cbaud;

  /* The minimum number of free bytes allowed in the input buffer before flow control is activated
   * to inhibit the sender. Tune value if you experience issues! */
  settings.XoffLim = 1000;
  /* The minimum number of bytes in use allowed in the input buffer before flow control is activated
   * to allow transmission by the sender. Tune value if you experience issues! */
  settings.XonLim = 1000;

  /* Configure software flow control. */
  if (xOnXOff) {
    settings.fOutX = true;
    settings.fInX  = true;
    settings.XoffChar = 0x13;
    settings.XonChar = 0x11;
    settings.fTXContinueOnXoff = false;
  } else {
    settings.fOutX = false;
    settings.fInX  = false;
  }

  /* Configure word length. Any number other than 5, 6 or 7 defaults to 8. */
  if (dataBits >= 5 && dataBits <= 7) {
    settings.ByteSize = (BYTE)dataBits;
  } else {
    settings.ByteSize = 8;
  }

  /* Configure parity. Any number other than 1 or 2 defaults to 0. */
  if (parity == 1) {
    settings.Parity = ODDPARITY;
  } else if (parity == 2) {
    settings.Parity = EVENPARITY;
  } else {
    /* No parity. */
    settings.Parity = NOPARITY;
  }

  /* Configure number of stop bits. Any number other than 2 defaults to 1. */
  if (stopBits == 2) {
    settings.StopBits = TWOSTOPBITS;
  } else {
    /* 1 stop bit. */
    settings.StopBits = ONESTOPBIT;
  }

  /* Configure hardware flow control. */
  if (rtsCts) {
    settings.fRtsControl = RTS_CONTROL_HANDSHAKE;
    settings.fOutxCtsFlow = true;
  } else {
    settings.fRtsControl = RTS_CONTROL_ENABLE;
    settings.fOutxCtsFlow = false;
  }

  if (!SetCommState(serial, &settings)) {
    fprintf(stderr, "Error setting comm state on %s - %ld.\n",
            (char*)device,
            GetLastError());
    goto error;
  }

  /* Set the time-out parameters for all read and write operations on a specified communications
   * device.
   * ReadIntervalTimeout: The maximum time allowed to elapse before the arrival of the next byte on
   *    the communications line, in milliseconds. If the interval between the arrival of any two
   *    bytes exceeds this amount, the ReadFile operation is completed and any buffered data is
   *    returned. A value of zero indicates that interval time-outs are not used.
   *    A value of MAXDWORD, combined with zero values for both the ReadTotalTimeoutConstant and
   *    ReadTotalTimeoutMultiplier members, specifies that the read operation is to return
   *    immediately with the bytes that have already been received, even if no bytes have been
   *    received.
   * ReadTotalTimeoutMultiplier: The multiplier used to calculate the total time-out period for read
   *    operations, in milliseconds. For each read operation, this value is multiplied by the
   *    requested number of bytes to be read.
   * ReadTotalTimeoutConstant: A constant used to calculate the total time-out period for read
   *    operations, in milliseconds. For each read operation, this value is added to the product of
   *    the ReadTotalTimeoutMultiplier member and the requested number of bytes. A value of zero for
   *    both the ReadTotalTimeoutMultiplier and ReadTotalTimeoutConstant members indicates that
   *    total time-outs are not used for read operations.
   * WriteTotalTimeoutMultiplier: The multiplier used to calculate the total time-out period for
   *    write operations, in milliseconds. For each write operation, this value is multiplied by the
   *    number of bytes to be written.
   * WriteTotalTimeoutConstant: A constant used to calculate the total time-out period for write
   *    operations, in milliseconds. For each write operation, this value is added to the product of
   *    the WriteTotalTimeoutMultiplier member and the number of bytes to be written. A value of
   *    zero for both the WriteTotalTimeoutMultiplier and WriteTotalTimeoutConstant members
   *    indicates that total time-outs are not used for write operations.*/
  if (timeout < 0) {
    /* Block until character is received. No timeout configured. */
    commTimeouts.ReadIntervalTimeout = 0;
    commTimeouts.ReadTotalTimeoutConstant = 0;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
  } else if (timeout == 0) {
    /* Do not block. Return immediately. */
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutConstant = 0;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
  } else {
    /* Block until character is received or timer expires. */
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = (DWORD)timeout;
  }

  commTimeouts.WriteTotalTimeoutMultiplier = 0;
  commTimeouts.WriteTotalTimeoutConstant = (DWORD)timeout;

  if (!SetCommTimeouts(serial, &commTimeouts)) {
    fprintf(stderr, "Error setting comm timeouts on %s - %ld.\n",
            (char*)device,
            GetLastError());
    goto error;
  }

  /* Success */
  return serial;

  /* Failure */
  error:
  if (serial != INVALID_HANDLE_VALUE) {
    CloseHandle(serial);
  }

  return INVALID_HANDLE_VALUE;
}

/* Close a serial port. Return nonzero on success, 0 on failure. */
static int32_t uartCloseSerial(HANDLE handle)
{
  int32_t ret;

  if (!(ret = CloseHandle(handle))) {
    fprintf(stderr, "Error closing serial port - %ld.\n", GetLastError());
  }

  return ret;
}
