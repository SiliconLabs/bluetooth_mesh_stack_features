/***************************************************************************//**
 * @file
 * @brief Functions and data related to Mesh PS store
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
#include "bg_types.h"     /* blue gecko types defs */
#include "mesh_nvm.h"     /* app mesh nvm defns */
#include "em_device.h"    /* device defns */
#include "native_gecko.h" /* for ps erase */

/******************************************************************************
**                             Typedef and macros
***************************************************************************** */
#define NVM_MESH_KEY_START 0x1000 /* Start key for BT Mesh entries */
#define NVM_MESH_KEY_END 0x3FFF /* End of BT Mesh, start of user application keys */

/******************************************************************************
**                               Local defns
***************************************************************************** */

/******************************************************************************
**                                   API
***************************************************************************** */
eMesh_NVM_status mesh_erase_nvm_settings(void)
{
  eMesh_NVM_status status;
  uint16 key;

  /* Initialise locals */
  status = eMESH_NVM_OK;

  printf("erasing keys in range 0x1000-0x3FFF...\r\n");
  for(key=NVM_MESH_KEY_START;key<NVM_MESH_KEY_END;key++)
  {
	  if(SL_STATUS_OK != sl_bt_nvm_erase(key))
	  {
		  /* Erase error */
  		  status = eMESH_NVM_ERASE_ERROR;
		  printf("error erasing key %x\r\n", key);
	  }
  }
  printf("done.\r\n");

  return status;
}
