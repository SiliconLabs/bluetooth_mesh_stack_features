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

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

#define BASE_DEC                                    0
#define BASE_HEX                                    1

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(int argc, char *argv[]);

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void);

/**************************************************************************//**
 * Application Deinit.
 *****************************************************************************/
void app_deinit(void);

/**************************************************************************//**
 * Command handling.
 *****************************************************************************/
typedef int (*pPrmCheckAndExecCB_t)(int argc, const char *argv[]);
typedef struct CmdItem{
  char *command;
  char *p1;
  char *usage;
  char *p2;
  char *desc;
  pPrmCheckAndExecCB_t pCb;
}CmdItem_t;

/**************************************************************************//**
 * LPN
 *****************************************************************************/
void lpn_init(void);
void lpn_deinit(void);

/**************************************************************************//**
 * Init
 *****************************************************************************/
void switch_node_init(void);
void initiate_factory_reset(int type);
static inline void resetSwitchVariables(void);

void send_onoff_request(int retrans);
void send_lightness_request(int retrans);
void send_ctl_request(int retrans);

/**************************************************************************//**
 * Threads for timing.
 *****************************************************************************/
void *pConsoleThread(void *pIn);
void *pAppMainThread(void *pIn);
void timerHandle(int handle);

/**************************************************************************//**
 * Helper functions.
 *****************************************************************************/
int str2uint(const char *input, size_t length, uint32_t *p_ret);
int uint2str(uint64_t input, uint8_t base_type, size_t length, char str[]);

#ifdef __cplusplus
};
#endif

#endif
