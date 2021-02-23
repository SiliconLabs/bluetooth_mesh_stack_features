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
#ifndef MESH_PSSTORE_H_
#define MESH_PSSTORE_H_

/******************************************************************************
**                             Typedef and macros
***************************************************************************** */
typedef enum {
	eMESH_PSSTORE_OK=0,
	eMESH_PSSTORE_ERASE_ERROR,
	eMESH_PSSTORE_MEMORY_ERROR

} eMesh_PSstore_status;

/******************************************************************************
**                                   API
***************************************************************************** */
eMesh_PSstore_status mesh_erase_psstore_settings(void);

#endif
