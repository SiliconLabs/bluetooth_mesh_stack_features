/***************************************************************************//**
 * @file
 * @brief Functions and data related to Mesh PS store
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

/******************************************************************************
**                            DISCLAIMER START
***************************************************************************** */
/*
 * This code is not officially supported. use at your own risk.
 */
/******************************************************************************
**                             DISCLAIMER END
***************************************************************************** */

/******************************************************************************
**                               System includes
***************************************************************************** */
#include <stdio.h> /* standard ios */

/******************************************************************************
**                             Non system includes
***************************************************************************** */
#include "mesh_erase_nvm.h"     /* app mesh nvm defns */
#include "sl_status.h"
#include "sl_bt_api.h"

/******************************************************************************
**                             Typedef and macros
***************************************************************************** */
#define NVM_MESH_KEY_START 0x1000 /* Start key for BT Mesh entries */
#define NVM_MESH_KEY_END 0x2FFF /* End of BT Mesh, start of user application keys */

/******************************************************************************
**                               Local defns
***************************************************************************** */

/******************************************************************************
**                                   API
***************************************************************************** */
eMesh_NVM_status mesh_erase_nvm_settings(void)
{
  eMesh_NVM_status status;
  sl_status_t result;
  uint16_t key;

  /* Initialise locals */
  status = eMESH_NVM_OK;

  printf("Erasing keys in range 0x1000-0x2FFF...\r\n");
  for(key = NVM_MESH_KEY_START; key < NVM_MESH_KEY_END; key++)
  {
    result = sl_bt_nvm_erase(key);
	  if(result !=  SL_STATUS_OK)
	  {
		  /* Erase error */
  		status = eMESH_NVM_ERASE_ERROR;
		  //printf("Error erasing key %x - result %lx.\r\n", key, result);
	  } else {
	    printf("Success erasing key %x - result %lx.\r\n", key, result);
	  }
  }
  printf("Done.\r\n");

  return status;
}
