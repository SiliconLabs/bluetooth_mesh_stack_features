/***************************************************************************//**
 * @file
 * @brief sl_btmesh_ctl_client.h
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_BTMESH_HSL_CLIENT_H
#define SL_BTMESH_HSL_CLIENT_H
//------------
//autogen/sl_btmesh_dcd.h
#define BTMESH_HSL_CLIENT_MAIN 0

//------------
//config/sl_btmesh_hsl_client_config.h

// <h> HSL Client configuration

// <o HSL_CLIENT_RETRANSMISSION_COUNT> HSL model retransmission count
// <i> Default: 3
// <i> HSL model retransmission count (How many times HSL model messages are to be sent out for reliability).
#define HSL_CLIENT_RETRANSMISSION_COUNT   (3)

// <o HSL_CLIENT_RETRANSMISSION_TIMEOUT> HSL model retransmission timeout [ms] <0-1275:5>
// <i> Default: 50
// <i> HSL model retransmission timeout.
#define HSL_CLIENT_RETRANSMISSION_TIMEOUT   (50)

// <q HSL_CLIENT_TEMPERATURE_WRAP_ENABLED> Enable color temperature wraparound
// <i> Default: 0
// <i> If the color temperature reaches the max or min value then it wraps around.
#define HSL_CLIENT_TEMPERATURE_WRAP_ENABLED   (0)

// <e HSL_CLIENT_LOGGING> Enable Logging
// <i> Default: 1
// <i> Enable / disable Logging for HSL Client model specific messages for this component.
#define HSL_CLIENT_LOGGING   (1)

// <s.128 HSL_CLIENT_LOGGING_NEW_TEMP_SET> Log text when new color temperature has been set.
// <i> Set Log text when new color temperature has been set
#define HSL_CLIENT_LOGGING_NEW_TEMP_SET "Set HSL, H:%d, S:%d, L:%d\r\n"

// <s.128 HSL_CLIENT_LOGGING_CLIENT_PUBLISH_FAIL> Log text when sending a HSL message fails.
// <i> Set Log text in case sending a HSL message fails
#define HSL_CLIENT_LOGGING_CLIENT_PUBLISH_FAIL "HSL Client Publish failed\r\n"

// <s.128 HSL_CLIENT_LOGGING_RECALL_SUCCESS> Log text when recalling a scene recall is successful.
// <i> Set Log text a scene recall is successful.
#define HSL_CLIENT_LOGGING_RECALL_SUCCESS "HSL request sent, trid = %u, delay = %u\r\n"
//------------

/*******************************************************************************
 * This function change the HSL and sends it to the server.
 ******************************************************************************/
void sl_btmesh_change_hsl(void);

/*******************************************************************************
 * This function change the temperature and send it to the server.
 *
 * @param[in] new_hsl  Defines new HSL
 * value as percentage.
 *    Valid values 0-100 %
 *
 ******************************************************************************/
void sl_btmesh_set_hsl(uint8_t new_hsl);

#endif // SL_BTMESH_CTL_CLIENT_H
