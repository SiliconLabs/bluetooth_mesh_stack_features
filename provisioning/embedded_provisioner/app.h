/***************************************************************************//**
 * @file
 * @brief Application header file
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif


/* compile time options */

// uncomment this to enable configuration of vendor model
//#define CONFIGURE_VENDOR_MODEL

// uncomment this to enable provisioning using PB-GATT (PB-ADV is used by default)
//#define PROVISION_OVER_GATT
// uncomment this to use fixed network and application keys (for debugging only)
//#define USE_FIXED_KEYS

#ifdef USE_FIXED_KEYS
const uint8 fixed_netkey[16] = {0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3};
const uint8 fixed_appkey[16] = {4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7};
#endif

/** Timer Frequency used. */
#define TIMER_CLK_FREQ ((uint32)32768)
/** Convert msec to timer ticks. */
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#define TIMER_ID_RESTART    78
#define TIMER_ID_FACTORY_RESET  77
#define TIMER_ID_BUTTON_POLL              49
#define TIMER_ID_GET_DCD          20
#define TIMER_ID_APPKEY_ADD         21
#define TIMER_ID_APPKEY_BIND          22
#define TIMER_ID_PUB_SET          23
#define TIMER_ID_SUB_ADD          24
// max number of SIG models in the DCD
#define MAX_SIG_MODELS    16
// max number of vendor models in the DCD
#define MAX_VENDOR_MODELS 4

#define LIGHT_CTRL_GRP_ADDR     0xC001
#define LIGHT_STATUS_GRP_ADDR   0xC002
#define VENDOR_GRP_ADDR         0xC003
#define VENDOR_ID_INVALID       0xFFFF


#define PROVISION_ATTN_TIME_DISABLED       0  //attention timer in seconds
#define ADDRESS_ASSIGN_AUTO                0  //let the stack assign addresses automatically.

#define PAGE0 0

/* models used by simple light example (on/off only)
 * The beta SDK 1.0.1 and 1.1.0 examples are based on these
 * */
#define LIGHT_MODEL_ID            0x1000 // Generic On/Off Server
#define SWITCH_MODEL_ID           0x1001 // Generic On/Off Client

/*
 * Lightness models used in the dimming light example of 1.2.0 SDK
 * */
#define DIM_LIGHT_MODEL_ID              0x1300 // Light Lightness Server
#define DIM_SWITCH_MODEL_ID             0x1302 // Light Lightness Client

typedef struct
{
  uint16 model_id;
  uint16 vendor_id;
} tsModel;

/* struct for storing the content of one element in the DCD */
typedef struct
{
  uint16 SIG_models[MAX_SIG_MODELS];
  uint8 numSIGModels;

  tsModel vendor_models[MAX_VENDOR_MODELS];
  uint8_t numVendorModels;
}tsDCD_ElemContent;


typedef struct
{
  // model bindings to be done. for simplicity, all models are bound to same appkey in this example
  // (assuming there is exactly one appkey used and the same appkey is used for all model bindings)
  tsModel bind_model[4];
  uint8 num_bind;
  uint8 num_bind_done;

  // publish addresses for up to 4 models
  tsModel pub_model[4];
  uint16 pub_address[4];
  uint8 num_pub;
  uint8 num_pub_done;

  // subscription addresses for up to 4 models
  tsModel sub_model[4];
  uint16 sub_address[4];
  uint8 num_sub;
  uint8 num_sub_done;

}tsConfig;

/* this struct is used to help decoding the raw DCD data */
typedef struct
{
  uint16 companyID;
  uint16 productID;
  uint16 version;
  uint16 replayCap;
  uint16 featureBitmask;
  uint8 payload[1];
} tsDCD_Header;

/* this struct is used to help decoding the raw DCD data */
typedef struct
{
  uint16 location;
  uint8 numSIGModels;
  uint8 numVendorModels;
  uint8 payload[1];
} tsDCD_Elem;

typedef struct
{
  uint16 err;
  const char *pShortDescription;
} tsErrCode;

#define STATUS_OK                      0
#define STATUS_BUSY                    0x181
#define SOFT_TIMER_SINGLE_SHOT         0
#define SOFT_TIMER_REPEATING           1
/***********************************************************************************************//**
 * \defgroup app Application Code
 * \brief Sample Application Implementation
 **************************************************************************************************/
#define ELEMENT(x)                     x
#define FRIENDSHIP_CREDENTIALS_NONE    0
#define PUB_TTL                        3
#define PUB_PERIOD_MS                  0
#define PUB_RETRANS_COUNT              0
#define PUB_RETRANS_INTERVAL_MS        50
#define KEY_INDEX_INVALID            0xFF
#define ADDRESS_INVALID            0xFFFF
/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

/***************************************************************************//**
 * Initialise used bgapi classes.
 ******************************************************************************/
void gecko_bgapi_classes_init (void);

/***************************************************************************//**
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events
 * are handled here.
 * @param[in] evt_id  Incoming event ID.
 * @param[in] evt     Pointer to incoming event.
 ******************************************************************************/
void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */

#ifdef __cplusplus
};
#endif

#endif /* APP_H */
