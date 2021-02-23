/***************************************************************************//**
 * @file
 * @brief Functions and data related to PTI
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
#ifndef MESH_NVM_H_
#define MESH_NVM_H_

/******************************************************************************
**                             Typedef and macros
***************************************************************************** */
typedef enum {
	eMESH_NVM_OK=0,
	eMESH_NVM_ERASE_ERROR,
	eMESH_NVM_MEMORY_ERROR

} eMesh_NVM_status;

/******************************************************************************
**                                   API
***************************************************************************** */
eMesh_nvm_status mesh_erase_nvm_settings(void);

#endif
