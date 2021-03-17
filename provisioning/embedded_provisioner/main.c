/***************************************************************************//**
 * @file
 * @brief Silicon Labs Bluetooth mesh Embedded provisioner.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

/* System includes */
#include <stdio.h>
#include <string.h> /* memset */

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "board_features.h"
#include "retargetserial.h"

/* Bluetooth stack headers */
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include <mesh_sizes.h>

#include "stack.h"
#include "gpiointerrupt.h"

/* Libraries containing default Gecko configuration values */
#include <em_gpio.h>

/* Coex header */
#include "coexistence-ble.h"

/* Device initialization header */
#include "hal-config.h"

/* Display Interface header */
#include "display_interface.h"

/* LED driver with support for PWM dimming */
#include "led_driver.h"

/* Application code */
#include "src/app.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/
static void Button_Event(const uint8_t pin); /* Button int handler */
static void background(void);
static void btherm_cmd_bkgnd(void);

bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

/// Maximum number of simultaneous Bluetooth connections
#define MAX_CONNECTIONS 2

/// Heap for Bluetooth stack
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 1760];

/// Bluetooth advertisement set configuration
///
/// At minimum the following is required:
/// * One advertisement set for Bluetooth LE stack (handle number 0)
/// * One advertisement set for Mesh data (handle number 1)
/// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
/// * One advertisement set for Mesh unprovisioned URI (handle number 3)
/// * N advertisement sets for Mesh GATT service advertisements
/// (one for each network key, handle numbers 4 .. N+3)
///
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

/// Priorities for bluetooth link layer operations
static gecko_bluetooth_ll_priorities linklayer_priorities = GECKO_BLUETOOTH_PRIORITIES_DEFAULT;

/// Bluetooth stack configuration
const gecko_configuration_t config =
{
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
#if defined(FEATURE_LFXO)
  .bluetooth.sleep_clock_accuracy = 100, // ppm
#elif defined(PLFRCO_PRESENT)
  .bluetooth.sleep_clock_accuracy = 500, // ppm
#endif
  .bluetooth.linklayer_priorities = &linklayer_priorities,
  .gattdb = &bg_gattdb_data,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
  .pa.config_enable = 1, // Set this to be a valid PA config
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#else
  .pa.input = GECKO_RADIO_PA_INPUT_DCDC,
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
  .max_timers = 16,
  .rf.flags = APP_RF_CONFIG_ANTENNA,   // Enable antenna configuration.
  .rf.antenna = APP_RF_ANTENNA         // Select antenna path!
};

/***************************************************************************//**
 * Button initialization. Configure pushbuttons PB0, PB1 as inputs.
 ******************************************************************************/
static void button_init(void)
{
  // configure pushbutton PB0 and PB1 as inputs, with pull-up enabled
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
  GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPull, 1);
}

/***************************************************************************//**
 * Initialise used bgapi classes.
 ******************************************************************************/
static void gecko_bgapi_classes_init(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  gecko_bgapi_class_sm_init();
  gecko_bgapi_class_mesh_node_init();
  gecko_bgapi_class_mesh_prov_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_vendor_model_init();
  gecko_bgapi_class_mesh_health_client_init();
  gecko_bgapi_class_mesh_health_server_init();
  gecko_bgapi_class_mesh_test_init();
  gecko_bgapi_class_mesh_config_client_init();
}

/***************************************************************************//**
 * Main function.
 ******************************************************************************/
int main(void)
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();
  initVcomEnable();

  // Minimize advertisement latency by allowing the advertiser to always
  // interrupt the scanner.
  linklayer_priorities.scan_max = linklayer_priorities.adv_min + 1;

  gecko_stack_init(&config);
  gecko_bgapi_classes_init();

  // Initialize the random number generator which is needed for proper radio work.
  gecko_cmd_system_get_random_data(16);

  // Initialize coexistence interface. Parameters are taken from HAL config.
  gecko_initCoexHAL();

  RETARGET_SerialInit();

  /* initialize LEDs and buttons. Note: some radio boards share the same GPIO for button & LED.
   * Initialization is done in this order so that default configuration will be "button" for those
   * radio boards with shared pins. LEDS_init() is called later as needed to (re)initialize the LEDs
   * */
  LEDS_init();
  button_init();

  GPIOINT_Init();

  /* enable interrupt for the button pins */
  GPIO_ExtIntConfig( BSP_BUTTON0_PORT,    /* button port */
                     BSP_BUTTON0_PIN,     /* button pin  */
                     BSP_BUTTON0_PIN,     /* button "ID" */
                     true,                /* enable for rising edge */
                     false,               /* enable for falling edge */
                     true );              /* enable interrupt */

  /* enable interrupt for the button pins */
  GPIO_ExtIntConfig( BSP_BUTTON1_PORT,    /* button port */
                     BSP_BUTTON1_PIN,     /* button pin  */
                     BSP_BUTTON1_PIN,     /* button "ID" */
                     true,                /* enable for rising edge */
                     false,               /* enable for falling edge */
                     true );              /* enable interrupt */

  /* define callback functions for the button pins */
  GPIOINT_CallbackRegister( BSP_BUTTON0_PIN, /* button pin */
                            Button_Event );  /* callback function */

  /* define callback functions for the button pins */
  GPIOINT_CallbackRegister( BSP_BUTTON1_PIN, /* button pin */
                            Button_Event );  /* callback function */

  // Initialize BLE stack app data
  initBLEMeshStack();

  // Display Interface initialization
  DI_Init();

  printf("Running background\r\n");

  /* do background operations */
  background();
}

static void background(void)
{
  for(;;)
  {
    /* The BLE mesh machinery, remember that is a blocking call */
    bkgndBLEMeshStack_app();

    /* command background */
    btherm_cmd_bkgnd();
  }
}

static void btherm_cmd_bkgnd(void)
{
  int key;

  /* provisioning can be accepted/rejected either by pushbuttons PB1/PB0
   * or alternatively by entering 'y' / 'n' on keyboard */
  key = RETARGET_ReadChar();

  if( key == '\r' )
  {
    printf("-> confirm? (use buttons or keys 'f' / 'e' / 'p')\r\n");
  }

  /* clean up traces */
  if( key == 'f' )
  {
    /* flush UART */
    printf("Flushing\r\n");
    RETARGET_SerialFlush();
  }

  /* Erase BT mesh settings */
  if( key == 'e' )
  {
    /* flush UART */
    printf("Erasing BT mesh settings\r\n");
#if 0
    mesh_erase_psstore_settings();
#else
    /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
       that have been configured for this node */
    gecko_cmd_flash_ps_erase_all();

    // reset the device to finish factory reset
    gecko_cmd_system_reset(0);
#endif
  }

  /* provision next device in line */
  if( key == 'p' )
  {
    /* Indicate to the stack the it has to provision the next in the list */
    provisionBLEMeshStack_app(eMESH_PROV_NEXT);
  }

  return;
}

/* push button generates external signal event for the stacks */
static void Button_Event(const uint8_t pin)
{
  /* If Button 0 erase mesh entries */
  if(BSP_BUTTON0_PIN == pin)
  {
    printf("Button Press NVM erase\r\n");
    /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
       that have been configured for this node */
    gecko_cmd_flash_ps_erase_all();

    // reset the device to finish factory reset
    gecko_cmd_system_reset(0);
  }

  /* If Button 1, provision */
  if(BSP_BUTTON1_PIN == pin)
  {
    printf("Button Press Provisioning\r\n");
    /* Indicate to the stack the it has to provision the next in the list */
    provisionBLEMeshStack_app(eMESH_PROV_NEXT);
  }

  /* Print ps store status after write BT mesh stuff */
  GPIO_IntClear(0x00000000);
  return;
}
