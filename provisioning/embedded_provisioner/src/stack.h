#ifndef STACK_H
#define STACK_H

/* Provisioning strategy */
typedef enum {
  eMESH_PROV_ALL=0,
  eMESH_PROV_NEXT
} eMesh_Prov_Node_t;

/* Initialize the stack */
void initBLEMeshStack(void);

/* BLE stack device state machine */
void bkgndBLEMeshStack(void);

/* provision list */
void provisionBLEMeshStack(eMesh_Prov_Node_t eStrategy);

#endif
