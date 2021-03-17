/***************************************************************************//**
 * @file
 * @brief UART header file
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

#ifndef UART_H
#define UART_H

/***********************************************************************************************//**
 * \defgroup uart UART
 * \brief UART API
 **************************************************************************************************/

/***********************************************************************************************//**
 * \defgroup platform_hw Platform HW
 * \brief Platform HW
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup platform_hw
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup uart
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Open the serial port.
 *  \param[in]  port Serial port to use.
 *  \param[in]  baudRate Baud rate to use.
 *  \param[in]  rtsCts Enable/disable hardware flow control.
 *  \param[in]  timeout Constant used to calculate the total time-out period for read operations, in
 *              milliseconds.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
int32_t uartOpen(int8_t* port, uint32_t baudRate, uint32_t rtsCts, int32_t timeout);

/***********************************************************************************************//**
 *  \brief  Close the serial port.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
int32_t uartClose(void);

/***********************************************************************************************//**
 *  \brief  Blocking read data from serial port. The function will block until the desired amount
 *          has been read or an error occurs.
 *  \note  In order to use this function the serial port has to be configured blocking. This can be
 *         done by calling uartOpen() with 'timeout = 0'.
 *  \param[in]  dataLength The amount of bytes to read.
 *  \param[out]  data Buffer used for storing the data.
 *  \return  The amount of bytes read or -1 on failure.
 **************************************************************************************************/
int32_t uartRx(uint32_t dataLength, uint8_t* data);

/***********************************************************************************************//**
 *  \brief  Non-blocking read from serial port.
 *  \note  A truly non-blocking operation is possible only if uartOpen() is called with timeout
 *         parameter set to 0.
 *  \param[in]  dataLength The amount of bytes to read.
 *  \param[out]  data Buffer used for storing the data.
 *  \return  The amount of bytes read, 0 if configured serial blocking time interval elapses or -1
 *           on failure.
 **************************************************************************************************/
int32_t uartRxNonBlocking(uint32_t dataLength, uint8_t* data);

/***********************************************************************************************//**
 *  \brief  Return the number of bytes in the input buffer.
 *  \return  The number of bytes in the input buffer or -1 on failure.
 **************************************************************************************************/
int32_t uartRxPeek(void);

/***********************************************************************************************//**
 *  \brief  Write data to serial port. The function will block until
 *          the desired amount has been written or an error occurs.
 *  \param[in]  dataLength The amount of bytes to write.
 *  \param[in]  data Buffer used for storing the data.
 *  \return  The amount of bytes written or -1 on failure.
 **************************************************************************************************/
int32_t uartTx(uint32_t dataLength, uint8_t* data);

/** @} (end addtogroup uart) */
/** @} (end addtogroup platform_hw) */

#endif /* UART_H */
