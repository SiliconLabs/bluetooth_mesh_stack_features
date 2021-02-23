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
#include "mesh_pstore.h"  /* app mesh pstore defns */
#include "em_device.h"    /* device defns */
#include "native_gecko.h" /* for ps erase */

/******************************************************************************
**                             Typedef and macros
***************************************************************************** */
#define PS_MESH_KEY_START 0x1000 /* Start key for BT Mesh entries */
#define PS_MESH_KEY_END 0x3FFF /* End of BT Mesh, start of user application keys */

/******************************************************************************
**                               Local defns
***************************************************************************** */

/******************************************************************************
**                                   API
***************************************************************************** */
eMesh_PSstore_status mesh_erase_psstore_settings(void)
{
  eMesh_PSstore_status status;
  uint16 key;
  uint16 p;

  /* Initialise locals */
  status = eMESH_PSSTORE_OK;

  printf("erasing keys in range 0x1000-0x3FFF...\r\n");
  for(key=PS_MESH_KEY_START;key<PS_MESH_KEY_END;key++)
  {
	  p = gecko_cmd_flash_ps_erase(key)->result;
	  if(p)
	  {
		  /* Erase error */
  		  status = eMESH_PSSTORE_ERASE_ERROR;
		  printf("error erasing key %x -> result: %x\r\n", key, p);
	  }
  }
  printf("done.\r\n");

  return status;
}
