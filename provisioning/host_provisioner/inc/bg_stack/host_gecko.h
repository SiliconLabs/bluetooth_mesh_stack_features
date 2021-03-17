/***************************************************************************//**
 * @brief Bluetooth BGAPI for host applications in NCP mode
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

#ifndef HOST_GECKO_H
#define HOST_GECKO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "bg_types.h"
#include "gecko_configuration.h"
#include "bg_errorcodes.h"


/* Compatibility */
#ifndef PACKSTRUCT
/*Default packed configuration*/
#ifdef __GNUC__
#ifdef _WIN32
#define PACKSTRUCT( decl ) decl __attribute__((__packed__,gcc_struct))
#else
#define PACKSTRUCT( decl ) decl __attribute__((__packed__))
#endif
#define ALIGNED __attribute__((aligned(0x4)))
#elif __IAR_SYSTEMS_ICC__

#define PACKSTRUCT( decl ) __packed decl

#define ALIGNED
#elif _MSC_VER  /*msvc*/

#define PACKSTRUCT( decl ) __pragma( pack(push, 1) ) decl __pragma( pack(pop) )
#define ALIGNED
#else 
#define PACKSTRUCT(a) a PACKED 
#endif
#endif


#define BGLIB_DEPRECATED_API __attribute__((deprecated))
#define BGLIB_MSG_ID(HDR) ((HDR)&0xffff00f8)
#define BGLIB_MSG_HEADER_LEN (4)
#define BGLIB_MSG_LEN(HDR) ((((HDR)&0x7)<<8)|(((HDR)&0xff00)>>8))

/**
 * The maximum BGAPI command payload size.
 */
#define BGLIB_MSG_MAX_PAYLOAD 256


#define BGLIB_BIT_ENCRYPTED (1 << 6) // Bit indicating whether the packet is encrypted
#define BGLIB_MSG_ENCRYPTED(HDR) ((HDR) & BGLIB_BIT_ENCRYPTED)
/**
 * Blocks until new event arrives which requires processing by user application.
 * 
 * @return pointer to received event
 */
struct gecko_cmd_packet* gecko_wait_event(void);

/**
 * Same as gecko_wait_event but does not block if no events waiting, instead returns NULL
 *
 * @return pointer to received event or NULL if no event waiting
 */
struct gecko_cmd_packet* gecko_peek_event(void);

/**
 * Events are in queue waiting for processing
 * Call gecko_wait_event or gecko_peek_event to process pending events
 *
 * @return nonzero if processing required
 */
int gecko_event_pending(void);

/**
 * Initialize stack.
 * @param config The pointer to configuration parameters, cannot be NULL.
 * @return bg_err_success if the initialization was successful; Other error code
 *         indicates a failure on initializing persistent storage.
 */
errorcode_t gecko_stack_init(const gecko_configuration_t *config);







enum le_gap_address_type
{
    le_gap_address_type_public                                   = 0x0,
    le_gap_address_type_random                                   = 0x1,
    le_gap_address_type_public_identity                          = 0x2,
    le_gap_address_type_random_identity                          = 0x3
};

enum le_gap_phy_type
{
    le_gap_phy_1m                                                = 0x1,
    le_gap_phy_2m                                                = 0x2,
    le_gap_phy_coded                                             = 0x4
};

enum le_gap_connectable_mode
{
    le_gap_non_connectable                                       = 0x0,
    le_gap_directed_connectable                                  = 0x1,
    le_gap_undirected_connectable                                = 0x2,
    le_gap_connectable_scannable                                 = 0x2,
    le_gap_scannable_non_connectable                             = 0x3,
    le_gap_connectable_non_scannable                             = 0x4
};

enum le_gap_discoverable_mode
{
    le_gap_non_discoverable                                      = 0x0,
    le_gap_limited_discoverable                                  = 0x1,
    le_gap_general_discoverable                                  = 0x2,
    le_gap_broadcast                                             = 0x3,
    le_gap_user_data                                             = 0x4
};

enum le_gap_discover_mode
{
    le_gap_discover_limited                                      = 0x0,
    le_gap_discover_generic                                      = 0x1,
    le_gap_discover_observation                                  = 0x2
};

enum le_gap_adv_address_type
{
    le_gap_identity_address                                      = 0x0,
    le_gap_non_resolvable                                        = 0x1
};

enum sync_advertiser_clock_accuracy
{
    sync_clock_accuracy_500                                      = 0x1f4,
    sync_clock_accuracy_250                                      = 0xfa,
    sync_clock_accuracy_150                                      = 0x96,
    sync_clock_accuracy_100                                      = 0x64,
    sync_clock_accuracy_75                                       = 0x4b,
    sync_clock_accuracy_50                                       = 0x32,
    sync_clock_accuracy_30                                       = 0x1e,
    sync_clock_accuracy_20                                       = 0x14
};

enum le_connection_security
{
    le_connection_mode1_level1                                   = 0x0,
    le_connection_mode1_level2                                   = 0x1,
    le_connection_mode1_level3                                   = 0x2,
    le_connection_mode1_level4                                   = 0x3
};

enum gatt_att_opcode
{
    gatt_read_by_type_request                                    = 0x8,
    gatt_read_by_type_response                                   = 0x9,
    gatt_read_request                                            = 0xa,
    gatt_read_response                                           = 0xb,
    gatt_read_blob_request                                       = 0xc,
    gatt_read_blob_response                                      = 0xd,
    gatt_read_multiple_request                                   = 0xe,
    gatt_read_multiple_response                                  = 0xf,
    gatt_write_request                                           = 0x12,
    gatt_write_response                                          = 0x13,
    gatt_write_command                                           = 0x52,
    gatt_prepare_write_request                                   = 0x16,
    gatt_prepare_write_response                                  = 0x17,
    gatt_execute_write_request                                   = 0x18,
    gatt_execute_write_response                                  = 0x19,
    gatt_handle_value_notification                               = 0x1b,
    gatt_handle_value_indication                                 = 0x1d
};

enum gatt_client_config_flag
{
    gatt_disable                                                 = 0x0,
    gatt_notification                                            = 0x1,
    gatt_indication                                              = 0x2
};

enum gatt_execute_write_flag
{
    gatt_cancel                                                  = 0x0,
    gatt_commit                                                  = 0x1
};

enum gatt_server_characteristic_status_flag
{
    gatt_server_client_config                                    = 0x1,
    gatt_server_confirmation                                     = 0x2
};


enum test_packet_type
{
    test_pkt_prbs9                                               = 0x0,
    test_pkt_11110000                                            = 0x1,
    test_pkt_10101010                                            = 0x2,
    test_pkt_11111111                                            = 0x4,
    test_pkt_00000000                                            = 0x5,
    test_pkt_00001111                                            = 0x6,
    test_pkt_01010101                                            = 0x7,
    test_pkt_pn9                                                 = 0xfd,
    test_pkt_carrier                                             = 0xfe
};

enum test_phy
{
    test_phy_1m                                                  = 0x1,
    test_phy_2m                                                  = 0x2,
    test_phy_125k                                                = 0x3,
    test_phy_500k                                                = 0x4
};

enum sm_bonding_key
{
    sm_bonding_key_ltk                                           = 0x1,
    sm_bonding_key_addr_public                                   = 0x2,
    sm_bonding_key_addr_static                                   = 0x4,
    sm_bonding_key_irk                                           = 0x8,
    sm_bonding_key_edivrand                                      = 0x10,
    sm_bonding_key_csrk                                          = 0x20,
    sm_bonding_key_masterid                                      = 0x40
};

enum sm_io_capability
{
    sm_io_capability_displayonly                                 = 0x0,
    sm_io_capability_displayyesno                                = 0x1,
    sm_io_capability_keyboardonly                                = 0x2,
    sm_io_capability_noinputnooutput                             = 0x3,
    sm_io_capability_keyboarddisplay                             = 0x4
};

enum homekit_category
{
    homekit_not_allowed                                          = 0x0,
    homekit_other                                                = 0x1,
    homekit_bridge                                               = 0x2,
    homekit_fan                                                  = 0x3,
    homekit_garage                                               = 0x4,
    homekit_lightbulb                                            = 0x5,
    homekit_doorlock                                             = 0x6,
    homekit_outlet                                               = 0x7,
    homekit_switch_accessory                                     = 0x8,
    homekit_thermostat                                           = 0x9,
    homekit_sensor                                               = 0xa,
    homekit_security_system                                      = 0xb,
    homekit_door                                                 = 0xc,
    homekit_window                                               = 0xd,
    homekit_window_covering                                      = 0xe,
    homekit_programmable_switch                                  = 0xf,
    homekit_ip_camera                                            = 0x11,
    homekit_video_door_bell                                      = 0x12,
    homekit_air_purifier                                         = 0x13,
    homekit_heater                                               = 0x14,
    homekit_air_conditioner                                      = 0x15,
    homekit_humidifier                                           = 0x16,
    homekit_dehumidifier                                         = 0x17,
    homekit_sprinkler                                            = 0x1c,
    homekit_faucet                                               = 0x1d,
    homekit_shower_system                                        = 0x1e,
    homekit_remote                                               = 0x20
};

enum homekit_status_code
{
    homekit_success                                              = 0x0,
    homekit_invalid_request                                      = 0x6
};

enum mesh_node_auth_method_flag
{
    mesh_node_auth_method_flag_none                              = 0x1,
    mesh_node_auth_method_flag_static                            = 0x2,
    mesh_node_auth_method_flag_input                             = 0x4,
    mesh_node_auth_method_flag_output                            = 0x8
};

enum mesh_node_oob_input_action_flag
{
    mesh_node_oob_input_action_flag_push                         = 0x1,
    mesh_node_oob_input_action_flag_twist                        = 0x2,
    mesh_node_oob_input_action_flag_numeric                      = 0x4,
    mesh_node_oob_input_action_flag_alpha                        = 0x8
};

enum mesh_node_oob_input_action
{
    mesh_node_oob_input_action_push                              = 0x0,
    mesh_node_oob_input_action_twist                             = 0x1,
    mesh_node_oob_input_action_numeric                           = 0x2,
    mesh_node_oob_input_action_alpha                             = 0x3
};

enum mesh_node_oob_output_action_flag
{
    mesh_node_oob_output_action_flag_blink                       = 0x1,
    mesh_node_oob_output_action_flag_beep                        = 0x2,
    mesh_node_oob_output_action_flag_vibrate                     = 0x4,
    mesh_node_oob_output_action_flag_numeric                     = 0x8,
    mesh_node_oob_output_action_flag_alpha                       = 0x10
};

enum mesh_node_oob_output_action
{
    mesh_node_oob_output_action_blink                            = 0x0,
    mesh_node_oob_output_action_beep                             = 0x1,
    mesh_node_oob_output_action_vibrate                          = 0x2,
    mesh_node_oob_output_action_numeric                          = 0x3,
    mesh_node_oob_output_action_alpha                            = 0x4
};

enum mesh_node_config_state
{
    mesh_node_dcd                                                = 0x8008,
    mesh_node_beacon                                             = 0x8009,
    mesh_node_default_ttl                                        = 0x800c,
    mesh_node_friendship                                         = 0x800f,
    mesh_node_gatt_proxy                                         = 0x8012,
    mesh_node_key_refresh                                        = 0x8015,
    mesh_node_relay                                              = 0x8023,
    mesh_node_identity                                           = 0x8042,
    mesh_node_nettx                                              = 0x8024
};




enum coex_option
{
    coex_option_enable                                           = 0x100,
    coex_option_tx_abort                                         = 0x400,
    coex_option_high_priority                                    = 0x800
};

enum mesh_test_key_type
{
    mesh_test_key_type_net                                       = 0x0,
    mesh_test_key_type_app                                       = 0x1
};

enum mesh_lpn_settings
{
    mesh_lpn_queue_length                                        = 0x0,
    mesh_lpn_poll_timeout                                        = 0x1,
    mesh_lpn_receive_delay                                       = 0x2,
    mesh_lpn_request_retries                                     = 0x3,
    mesh_lpn_retry_interval                                      = 0x4
};

enum l2cap_coc_connection_result
{
    l2cap_connection_successful                                  = 0x0,
    l2cap_le_psm_not_supported                                   = 0x2,
    l2cap_no_resources_available                                 = 0x4,
    l2cap_insufficient_authentication                            = 0x5,
    l2cap_insufficient_authorization                             = 0x6,
    l2cap_insufficient_encryption_key_size                       = 0x7,
    l2cap_insufficient_encryption                                = 0x8,
    l2cap_invalid_source_cid                                     = 0x9,
    l2cap_source_cid_already_allocated                           = 0xa,
    l2cap_unacceptable_parameters                                = 0xb
};

enum l2cap_command_reject_reason
{
    l2cap_command_not_understood                                 = 0x0,
    l2cap_signaling_mtu_exceeded                                 = 0x1,
    l2cap_invalid_cid_request                                    = 0x2
};

enum l2cap_command_code
{
    l2cap_disconnection_request                                  = 0x6,
    l2cap_connection_request                                     = 0x14,
    l2cap_flow_control_credit                                    = 0x16
};

enum gecko_parameter_types
{
    gecko_msg_parameter_uint8=2,
    gecko_msg_parameter_int8=3,
    gecko_msg_parameter_uint16=4,
    gecko_msg_parameter_int16=5,
    gecko_msg_parameter_uint32=6,
    gecko_msg_parameter_int32=7,
    gecko_msg_parameter_uint8array=8,
    gecko_msg_parameter_string=9,
    gecko_msg_parameter_hwaddr=10,
    gecko_msg_parameter_uint16array=11
};

enum gecko_msg_types
{
    gecko_msg_type_cmd=0x00,
    gecko_msg_type_rsp=0x00,
    gecko_msg_type_evt=0x80
};
enum gecko_dev_types
{
    gecko_dev_type_gecko   =0x20
};



#define FLASH_PS_KEY_LOCAL_BD_ADDR                                   0x2c
#define FLASH_PS_KEY_TX_POWER                                        0x31
#define FLASH_PS_KEY_CTUNE                                           0x32
#define FLASH_PS_KEY_APPLICATION_GSN                                 0x33
#define FLASH_PS_KEY_OTA_FLAGS                                       0x35
#define FLASH_PS_KEY_OTA_DEVICE_NAME                                 0x36
#define FLASH_PS_KEY_DEVICE_IRK                                      0x37
#define FLASH_PS_KEY_BONDING_PRIORITY_LIST                           0x38
#define FLASH_PS_KEY_OTA_ADVERTISEMENT_PACKET                        0x39
#define FLASH_PS_KEY_OTA_SCAN_RESPONSE_PACKET                        0x3a
#define FLASH_PS_KEY_APPLICATION_AI                                  0x3b
#define FLASH_PS_KEY_IDENTITY_ADDR_TYPE                              0x3c
#define FLASH_PS_KEY_GATT_DB_HASH                                    0x3d
#define FLASH_PS_KEY_BONDING_DB_CONFIG                               0x3fff
#define MESH_PROV_OOB_OTHER                                          0x1
#define MESH_PROV_OOB_URI                                            0x2
#define MESH_PROV_OOB_2D_MR_CODE                                     0x4
#define MESH_PROV_OOB_BAR_CODE                                       0x8
#define MESH_PROV_OOB_NFC                                            0x10
#define MESH_PROV_OOB_NUMBER                                         0x20
#define MESH_PROV_OOB_STRING                                         0x40
#define MESH_PROV_OOB_RFU_7                                          0x80
#define MESH_PROV_OOB_RFU_8                                          0x100
#define MESH_PROV_OOB_RFU_9                                          0x200
#define MESH_PROV_OOB_RFU_A                                          0x400
#define MESH_PROV_OOB_LOC_ON_BOX                                     0x800
#define MESH_PROV_OOB_LOC_IN_BOX                                     0x1000
#define MESH_PROV_OOB_LOC_PAPER                                      0x2000
#define MESH_PROV_OOB_LOC_MANUAL                                     0x4000
#define MESH_PROV_OOB_LOC_DEVICE                                     0x8000
#define MESH_PROV_OOB_RFU_MASK                                       0x780
#define MESH_GENERIC_CLIENT_STATE_ON_OFF                             0x0
#define MESH_GENERIC_CLIENT_STATE_ON_POWER_UP                        0x1
#define MESH_GENERIC_CLIENT_STATE_LEVEL                              0x2
#define MESH_GENERIC_CLIENT_STATE_POWER_LEVEL                        0x3
#define MESH_GENERIC_CLIENT_STATE_POWER_LEVEL_LAST                   0x4
#define MESH_GENERIC_CLIENT_STATE_POWER_LEVEL_DEFAULT                0x5
#define MESH_GENERIC_CLIENT_STATE_POWER_LEVEL_RANGE                  0x6
#define MESH_GENERIC_CLIENT_STATE_TRANSITION_TIME                    0x6
#define MESH_GENERIC_CLIENT_STATE_BATTERY                            0x8
#define MESH_GENERIC_CLIENT_STATE_LOCATION_GLOBAL                    0x9
#define MESH_GENERIC_CLIENT_STATE_LOCATION_LOCAL                     0xa
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_USER                      0xb
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_ADMIN                     0xc
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_MANUF                     0xd
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_LIST_USER                 0xe
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_LIST_ADMIN                0xf
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_LIST_MANUF                0x10
#define MESH_GENERIC_CLIENT_STATE_PROPERTY_LIST_CLIENT               0x11
#define MESH_GENERIC_CLIENT_STATE_LIGHTNESS_ACTUAL                   0x80
#define MESH_GENERIC_CLIENT_STATE_LIGHTNESS_LINEAR                   0x81
#define MESH_GENERIC_CLIENT_STATE_LIGHTNESS_LAST                     0x82
#define MESH_GENERIC_CLIENT_STATE_LIGHTNESS_DEFAULT                  0x83
#define MESH_GENERIC_CLIENT_STATE_LIGHTNESS_RANGE                    0x84
#define MESH_GENERIC_CLIENT_STATE_CTL                                0x85
#define MESH_GENERIC_CLIENT_STATE_CTL_TEMPERATURE                    0x86
#define MESH_GENERIC_CLIENT_STATE_CTL_DEFAULT                        0x87
#define MESH_GENERIC_CLIENT_STATE_CTL_RANGE                          0x88
#define MESH_GENERIC_CLIENT_STATE_CTL_LIGHTNESS_TEMPERATURE          0x89
#define MESH_GENERIC_CLIENT_REQUEST_ON_OFF                           0x0
#define MESH_GENERIC_CLIENT_REQUEST_ON_POWER_UP                      0x1
#define MESH_GENERIC_CLIENT_REQUEST_LEVEL                            0x2
#define MESH_GENERIC_CLIENT_REQUEST_LEVEL_DELTA                      0x3
#define MESH_GENERIC_CLIENT_REQUEST_LEVEL_MOVE                       0x4
#define MESH_GENERIC_CLIENT_REQUEST_LEVEL_HALT                       0x5
#define MESH_GENERIC_CLIENT_REQUEST_POWER_LEVEL                      0x6
#define MESH_GENERIC_CLIENT_REQUEST_POWER_LEVEL_DEFAULT              0x7
#define MESH_GENERIC_CLIENT_REQUEST_POWER_LEVEL_RANGE                0x8
#define MESH_GENERIC_CLIENT_REQUEST_TRANSITION_TIME                  0x9
#define MESH_GENERIC_CLIENT_REQUEST_LOCATION_GLOBAL                  0xa
#define MESH_GENERIC_CLIENT_REQUEST_LOCATION_LOCAL                   0xb
#define MESH_GENERIC_CLIENT_REQUEST_PROPERTY_USER                    0xc
#define MESH_GENERIC_CLIENT_REQUEST_PROPERTY_ADMIN                   0xd
#define MESH_GENERIC_CLIENT_REQUEST_PROPERTY_MANUF                   0xe
#define MESH_GENERIC_CLIENT_REQUEST_LIGHTNESS_ACTUAL                 0x80
#define MESH_GENERIC_CLIENT_REQUEST_LIGHTNESS_LINEAR                 0x81
#define MESH_GENERIC_CLIENT_REQUEST_LIGHTNESS_DEFAULT                0x82
#define MESH_GENERIC_CLIENT_REQUEST_LIGHTNESS_RANGE                  0x83
#define MESH_GENERIC_CLIENT_REQUEST_CTL                              0x84
#define MESH_GENERIC_CLIENT_REQUEST_CTL_TEMPERATURE                  0x85
#define MESH_GENERIC_CLIENT_REQUEST_CTL_DEFAULT                      0x86
#define MESH_GENERIC_CLIENT_REQUEST_CTL_RANGE                        0x87


#define gecko_cmd_dfu_reset_id                                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00000000)
#define gecko_cmd_dfu_flash_set_address_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01000000)
#define gecko_cmd_dfu_flash_upload_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02000000)
#define gecko_cmd_dfu_flash_upload_finish_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03000000)
#define gecko_cmd_system_hello_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00010000)
#define gecko_cmd_system_reset_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01010000)
#define gecko_cmd_system_get_bt_address_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03010000)
#define gecko_cmd_system_set_bt_address_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04010000)
#define gecko_cmd_system_set_tx_power_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a010000)
#define gecko_cmd_system_get_random_data_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b010000)
#define gecko_cmd_system_halt_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c010000)
#define gecko_cmd_system_set_device_name_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d010000)
#define gecko_cmd_system_linklayer_configure_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e010000)
#define gecko_cmd_system_get_counters_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f010000)
#define gecko_cmd_system_data_buffer_write_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12010000)
#define gecko_cmd_system_set_identity_address_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13010000)
#define gecko_cmd_system_data_buffer_clear_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x14010000)
#define gecko_cmd_le_gap_open_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00030000)
#define gecko_cmd_le_gap_set_mode_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01030000)
#define gecko_cmd_le_gap_discover_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02030000)
#define gecko_cmd_le_gap_end_procedure_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03030000)
#define gecko_cmd_le_gap_set_adv_parameters_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04030000)
#define gecko_cmd_le_gap_set_conn_parameters_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05030000)
#define gecko_cmd_le_gap_set_scan_parameters_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06030000)
#define gecko_cmd_le_gap_set_adv_data_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07030000)
#define gecko_cmd_le_gap_set_adv_timeout_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08030000)
#define gecko_cmd_le_gap_bt5_set_mode_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a030000)
#define gecko_cmd_le_gap_bt5_set_adv_parameters_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b030000)
#define gecko_cmd_le_gap_bt5_set_adv_data_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c030000)
#define gecko_cmd_le_gap_set_privacy_mode_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d030000)
#define gecko_cmd_le_gap_set_advertise_timing_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e030000)
#define gecko_cmd_le_gap_set_advertise_channel_map_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f030000)
#define gecko_cmd_le_gap_set_advertise_report_scan_request_id         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x10030000)
#define gecko_cmd_le_gap_set_advertise_phy_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x11030000)
#define gecko_cmd_le_gap_set_advertise_configuration_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12030000)
#define gecko_cmd_le_gap_clear_advertise_configuration_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13030000)
#define gecko_cmd_le_gap_start_advertising_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x14030000)
#define gecko_cmd_le_gap_stop_advertising_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x15030000)
#define gecko_cmd_le_gap_set_discovery_timing_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x16030000)
#define gecko_cmd_le_gap_set_discovery_type_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x17030000)
#define gecko_cmd_le_gap_start_discovery_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x18030000)
#define gecko_cmd_le_gap_set_data_channel_classification_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x19030000)
#define gecko_cmd_le_gap_connect_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1a030000)
#define gecko_cmd_le_gap_set_advertise_tx_power_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1b030000)
#define gecko_cmd_le_gap_set_discovery_extended_scan_response_id      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1c030000)
#define gecko_cmd_le_gap_start_periodic_advertising_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1d030000)
#define gecko_cmd_le_gap_stop_periodic_advertising_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1f030000)
#define gecko_cmd_le_gap_set_long_advertising_data_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x20030000)
#define gecko_cmd_le_gap_enable_whitelisting_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x21030000)
#define gecko_cmd_le_gap_set_conn_timing_parameters_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x22030000)
#define gecko_cmd_sync_open_id                                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00420000)
#define gecko_cmd_sync_close_id                                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01420000)
#define gecko_cmd_le_connection_set_parameters_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00080000)
#define gecko_cmd_le_connection_get_rssi_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01080000)
#define gecko_cmd_le_connection_disable_slave_latency_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02080000)
#define gecko_cmd_le_connection_set_phy_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03080000)
#define gecko_cmd_le_connection_close_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04080000)
#define gecko_cmd_le_connection_set_timing_parameters_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05080000)
#define gecko_cmd_gatt_set_max_mtu_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00090000)
#define gecko_cmd_gatt_discover_primary_services_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01090000)
#define gecko_cmd_gatt_discover_primary_services_by_uuid_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02090000)
#define gecko_cmd_gatt_discover_characteristics_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03090000)
#define gecko_cmd_gatt_discover_characteristics_by_uuid_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04090000)
#define gecko_cmd_gatt_set_characteristic_notification_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05090000)
#define gecko_cmd_gatt_discover_descriptors_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06090000)
#define gecko_cmd_gatt_read_characteristic_value_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07090000)
#define gecko_cmd_gatt_read_characteristic_value_by_uuid_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08090000)
#define gecko_cmd_gatt_write_characteristic_value_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09090000)
#define gecko_cmd_gatt_write_characteristic_value_without_response_id  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a090000)
#define gecko_cmd_gatt_prepare_characteristic_value_write_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b090000)
#define gecko_cmd_gatt_execute_characteristic_value_write_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c090000)
#define gecko_cmd_gatt_send_characteristic_confirmation_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d090000)
#define gecko_cmd_gatt_read_descriptor_value_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e090000)
#define gecko_cmd_gatt_write_descriptor_value_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f090000)
#define gecko_cmd_gatt_find_included_services_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x10090000)
#define gecko_cmd_gatt_read_multiple_characteristic_values_id         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x11090000)
#define gecko_cmd_gatt_read_characteristic_value_from_offset_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12090000)
#define gecko_cmd_gatt_prepare_characteristic_value_reliable_write_id  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13090000)
#define gecko_cmd_gatt_server_read_attribute_value_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x000a0000)
#define gecko_cmd_gatt_server_read_attribute_type_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x010a0000)
#define gecko_cmd_gatt_server_write_attribute_value_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x020a0000)
#define gecko_cmd_gatt_server_send_user_read_response_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x030a0000)
#define gecko_cmd_gatt_server_send_user_write_response_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x040a0000)
#define gecko_cmd_gatt_server_send_characteristic_notification_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x050a0000)
#define gecko_cmd_gatt_server_find_attribute_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x060a0000)
#define gecko_cmd_gatt_server_set_capabilities_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x080a0000)
#define gecko_cmd_gatt_server_set_max_mtu_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a0a0000)
#define gecko_cmd_hardware_set_soft_timer_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x000c0000)
#define gecko_cmd_hardware_get_time_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b0c0000)
#define gecko_cmd_hardware_set_lazy_soft_timer_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c0c0000)
#define gecko_cmd_flash_ps_erase_all_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x010d0000)
#define gecko_cmd_flash_ps_save_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x020d0000)
#define gecko_cmd_flash_ps_load_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x030d0000)
#define gecko_cmd_flash_ps_erase_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x040d0000)
#define gecko_cmd_test_dtm_tx_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x000e0000)
#define gecko_cmd_test_dtm_rx_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x010e0000)
#define gecko_cmd_test_dtm_end_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x020e0000)
#define gecko_cmd_sm_set_bondable_mode_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x000f0000)
#define gecko_cmd_sm_configure_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x010f0000)
#define gecko_cmd_sm_store_bonding_configuration_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x020f0000)
#define gecko_cmd_sm_increase_security_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x040f0000)
#define gecko_cmd_sm_delete_bonding_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x060f0000)
#define gecko_cmd_sm_delete_bondings_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x070f0000)
#define gecko_cmd_sm_enter_passkey_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x080f0000)
#define gecko_cmd_sm_passkey_confirm_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x090f0000)
#define gecko_cmd_sm_set_oob_data_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a0f0000)
#define gecko_cmd_sm_list_all_bondings_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b0f0000)
#define gecko_cmd_sm_bonding_confirm_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e0f0000)
#define gecko_cmd_sm_set_debug_mode_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f0f0000)
#define gecko_cmd_sm_set_passkey_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x100f0000)
#define gecko_cmd_sm_use_sc_oob_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x110f0000)
#define gecko_cmd_sm_set_sc_remote_oob_data_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x120f0000)
#define gecko_cmd_sm_add_to_whitelist_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x130f0000)
#define gecko_cmd_sm_set_minimum_key_size_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x140f0000)
#define gecko_cmd_homekit_configure_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00130000)
#define gecko_cmd_homekit_advertise_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01130000)
#define gecko_cmd_homekit_delete_pairings_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02130000)
#define gecko_cmd_homekit_check_authcp_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03130000)
#define gecko_cmd_homekit_get_pairing_id_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04130000)
#define gecko_cmd_homekit_send_write_response_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05130000)
#define gecko_cmd_homekit_send_read_response_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06130000)
#define gecko_cmd_homekit_gsn_action_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07130000)
#define gecko_cmd_homekit_event_notification_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08130000)
#define gecko_cmd_homekit_broadcast_action_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09130000)
#define gecko_cmd_mesh_node_init_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00140000)
#define gecko_cmd_mesh_node_start_unprov_beaconing_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01140000)
#define gecko_cmd_mesh_node_input_oob_request_rsp_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02140000)
#define gecko_cmd_mesh_node_get_uuid_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03140000)
#define gecko_cmd_mesh_node_set_provisioning_data_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04140000)
#define gecko_cmd_mesh_node_init_oob_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05140000)
#define gecko_cmd_mesh_node_set_ivrecovery_mode_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06140000)
#define gecko_cmd_mesh_node_get_ivrecovery_mode_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07140000)
#define gecko_cmd_mesh_node_set_adv_event_filter_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08140000)
#define gecko_cmd_mesh_node_get_statistics_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09140000)
#define gecko_cmd_mesh_node_clear_statistics_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a140000)
#define gecko_cmd_mesh_node_set_net_relay_delay_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b140000)
#define gecko_cmd_mesh_node_get_net_relay_delay_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c140000)
#define gecko_cmd_mesh_node_get_ivupdate_state_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d140000)
#define gecko_cmd_mesh_node_request_ivupdate_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e140000)
#define gecko_cmd_mesh_node_get_seq_remaining_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f140000)
#define gecko_cmd_mesh_node_save_replay_protection_list_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x10140000)
#define gecko_cmd_mesh_node_set_uuid_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x11140000)
#define gecko_cmd_mesh_node_get_element_address_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12140000)
#define gecko_cmd_mesh_node_static_oob_request_rsp_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13140000)
#define gecko_cmd_mesh_node_reset_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x15140000)
#define gecko_cmd_mesh_prov_init_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00150000)
#define gecko_cmd_mesh_prov_scan_unprov_beacons_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01150000)
#define gecko_cmd_mesh_prov_provision_device_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02150000)
#define gecko_cmd_mesh_prov_create_network_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03150000)
#define gecko_cmd_mesh_prov_get_dcd_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04150000)
#define gecko_cmd_mesh_prov_get_config_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05150000)
#define gecko_cmd_mesh_prov_set_config_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06150000)
#define gecko_cmd_mesh_prov_create_appkey_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07150000)
#define gecko_cmd_mesh_prov_oob_pkey_rsp_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08150000)
#define gecko_cmd_mesh_prov_oob_auth_rsp_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09150000)
#define gecko_cmd_mesh_prov_set_oob_requirements_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a150000)
#define gecko_cmd_mesh_prov_key_refresh_start_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b150000)
#define gecko_cmd_mesh_prov_get_key_refresh_blacklist_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c150000)
#define gecko_cmd_mesh_prov_set_key_refresh_blacklist_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d150000)
#define gecko_cmd_mesh_prov_appkey_add_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e150000)
#define gecko_cmd_mesh_prov_appkey_delete_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f150000)
#define gecko_cmd_mesh_prov_model_app_bind_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x10150000)
#define gecko_cmd_mesh_prov_model_app_unbind_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x11150000)
#define gecko_cmd_mesh_prov_model_app_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12150000)
#define gecko_cmd_mesh_prov_model_sub_add_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13150000)
#define gecko_cmd_mesh_prov_model_pub_set_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x14150000)
#define gecko_cmd_mesh_prov_provision_gatt_device_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x15150000)
#define gecko_cmd_mesh_prov_ddb_get_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x16150000)
#define gecko_cmd_mesh_prov_ddb_delete_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x17150000)
#define gecko_cmd_mesh_prov_ddb_add_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x18150000)
#define gecko_cmd_mesh_prov_ddb_list_devices_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x19150000)
#define gecko_cmd_mesh_prov_network_add_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1a150000)
#define gecko_cmd_mesh_prov_network_delete_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1b150000)
#define gecko_cmd_mesh_prov_nettx_get_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1c150000)
#define gecko_cmd_mesh_prov_nettx_set_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1d150000)
#define gecko_cmd_mesh_prov_model_sub_del_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1e150000)
#define gecko_cmd_mesh_prov_model_sub_add_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1f150000)
#define gecko_cmd_mesh_prov_model_sub_del_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x20150000)
#define gecko_cmd_mesh_prov_model_sub_set_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x21150000)
#define gecko_cmd_mesh_prov_model_sub_set_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x22150000)
#define gecko_cmd_mesh_prov_heartbeat_publication_get_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x23150000)
#define gecko_cmd_mesh_prov_heartbeat_publication_set_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x24150000)
#define gecko_cmd_mesh_prov_heartbeat_subscription_get_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x25150000)
#define gecko_cmd_mesh_prov_heartbeat_subscription_set_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x26150000)
#define gecko_cmd_mesh_prov_relay_get_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x27150000)
#define gecko_cmd_mesh_prov_relay_set_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x28150000)
#define gecko_cmd_mesh_prov_reset_node_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x29150000)
#define gecko_cmd_mesh_prov_appkey_get_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2a150000)
#define gecko_cmd_mesh_prov_network_get_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2b150000)
#define gecko_cmd_mesh_prov_model_sub_clear_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2c150000)
#define gecko_cmd_mesh_prov_model_pub_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2d150000)
#define gecko_cmd_mesh_prov_model_pub_set_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2e150000)
#define gecko_cmd_mesh_prov_model_pub_set_cred_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2f150000)
#define gecko_cmd_mesh_prov_model_pub_set_va_cred_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x30150000)
#define gecko_cmd_mesh_prov_model_sub_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x31150000)
#define gecko_cmd_mesh_prov_friend_timeout_get_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x32150000)
#define gecko_cmd_mesh_prov_get_default_configuration_timeout_id      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x33150000)
#define gecko_cmd_mesh_prov_set_default_configuration_timeout_id      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x34150000)
#define gecko_cmd_mesh_prov_provision_device_with_address_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x35150000)
#define gecko_cmd_mesh_prov_provision_gatt_device_with_address_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x36150000)
#define gecko_cmd_mesh_prov_initialize_network_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x37150000)
#define gecko_cmd_mesh_prov_get_key_refresh_appkey_blacklist_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x38150000)
#define gecko_cmd_mesh_prov_set_key_refresh_appkey_blacklist_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x39150000)
#define gecko_cmd_mesh_prov_stop_scan_unprov_beacons_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x3a150000)
#define gecko_cmd_mesh_proxy_connect_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00180000)
#define gecko_cmd_mesh_proxy_disconnect_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01180000)
#define gecko_cmd_mesh_proxy_set_filter_type_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02180000)
#define gecko_cmd_mesh_proxy_allow_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03180000)
#define gecko_cmd_mesh_proxy_deny_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04180000)
#define gecko_cmd_mesh_vendor_model_send_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00190000)
#define gecko_cmd_mesh_vendor_model_set_publication_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01190000)
#define gecko_cmd_mesh_vendor_model_clear_publication_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02190000)
#define gecko_cmd_mesh_vendor_model_publish_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03190000)
#define gecko_cmd_mesh_vendor_model_init_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04190000)
#define gecko_cmd_mesh_vendor_model_deinit_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05190000)
#define gecko_cmd_mesh_health_client_get_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x001a0000)
#define gecko_cmd_mesh_health_client_clear_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x011a0000)
#define gecko_cmd_mesh_health_client_test_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x021a0000)
#define gecko_cmd_mesh_health_client_get_period_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x031a0000)
#define gecko_cmd_mesh_health_client_set_period_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x041a0000)
#define gecko_cmd_mesh_health_client_get_attention_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x051a0000)
#define gecko_cmd_mesh_health_client_set_attention_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x061a0000)
#define gecko_cmd_mesh_health_server_set_fault_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x001b0000)
#define gecko_cmd_mesh_health_server_clear_fault_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x011b0000)
#define gecko_cmd_mesh_health_server_test_response_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x021b0000)
#define gecko_cmd_mesh_generic_client_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x001e0000)
#define gecko_cmd_mesh_generic_client_set_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x011e0000)
#define gecko_cmd_mesh_generic_client_publish_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x021e0000)
#define gecko_cmd_mesh_generic_client_get_params_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x031e0000)
#define gecko_cmd_mesh_generic_client_init_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x041e0000)
#define gecko_cmd_mesh_generic_server_response_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x001f0000)
#define gecko_cmd_mesh_generic_server_update_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x011f0000)
#define gecko_cmd_mesh_generic_server_publish_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x021f0000)
#define gecko_cmd_mesh_generic_server_init_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x041f0000)
#define gecko_cmd_coex_set_options_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00200000)
#define gecko_cmd_coex_get_counters_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01200000)
#define gecko_cmd_mesh_test_get_nettx_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00220000)
#define gecko_cmd_mesh_test_set_nettx_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01220000)
#define gecko_cmd_mesh_test_get_relay_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02220000)
#define gecko_cmd_mesh_test_set_relay_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03220000)
#define gecko_cmd_mesh_test_set_adv_scan_params_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04220000)
#define gecko_cmd_mesh_test_set_ivupdate_test_mode_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05220000)
#define gecko_cmd_mesh_test_get_ivupdate_test_mode_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06220000)
#define gecko_cmd_mesh_test_set_segment_send_delay_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07220000)
#define gecko_cmd_mesh_test_set_ivupdate_state_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08220000)
#define gecko_cmd_mesh_test_send_beacons_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09220000)
#define gecko_cmd_mesh_test_bind_local_model_app_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a220000)
#define gecko_cmd_mesh_test_unbind_local_model_app_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b220000)
#define gecko_cmd_mesh_test_add_local_model_sub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c220000)
#define gecko_cmd_mesh_test_del_local_model_sub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d220000)
#define gecko_cmd_mesh_test_add_local_model_sub_va_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e220000)
#define gecko_cmd_mesh_test_del_local_model_sub_va_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f220000)
#define gecko_cmd_mesh_test_get_local_model_sub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x10220000)
#define gecko_cmd_mesh_test_set_local_model_pub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x11220000)
#define gecko_cmd_mesh_test_set_local_model_pub_va_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12220000)
#define gecko_cmd_mesh_test_get_local_model_pub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13220000)
#define gecko_cmd_mesh_test_set_local_heartbeat_subscription_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x14220000)
#define gecko_cmd_mesh_test_get_local_heartbeat_subscription_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x15220000)
#define gecko_cmd_mesh_test_get_local_heartbeat_publication_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x16220000)
#define gecko_cmd_mesh_test_set_local_heartbeat_publication_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x17220000)
#define gecko_cmd_mesh_test_set_local_config_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x18220000)
#define gecko_cmd_mesh_test_get_local_config_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x19220000)
#define gecko_cmd_mesh_test_add_local_key_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1a220000)
#define gecko_cmd_mesh_test_del_local_key_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1b220000)
#define gecko_cmd_mesh_test_update_local_key_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1c220000)
#define gecko_cmd_mesh_test_set_sar_config_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1d220000)
#define gecko_cmd_mesh_test_get_element_seqnum_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1e220000)
#define gecko_cmd_mesh_test_set_adv_bearer_state_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1f220000)
#define gecko_cmd_mesh_test_get_key_count_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x20220000)
#define gecko_cmd_mesh_test_get_key_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x21220000)
#define gecko_cmd_mesh_test_prov_get_device_key_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x23220000)
#define gecko_cmd_mesh_test_prov_prepare_key_refresh_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x24220000)
#define gecko_cmd_mesh_test_cancel_segmented_tx_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x25220000)
#define gecko_cmd_mesh_test_set_iv_index_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x26220000)
#define gecko_cmd_mesh_test_set_element_seqnum_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x27220000)
#define gecko_cmd_mesh_lpn_init_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00230000)
#define gecko_cmd_mesh_lpn_deinit_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01230000)
#define gecko_cmd_mesh_lpn_configure_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02230000)
#define gecko_cmd_mesh_lpn_establish_friendship_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03230000)
#define gecko_cmd_mesh_lpn_poll_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04230000)
#define gecko_cmd_mesh_lpn_terminate_friendship_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05230000)
#define gecko_cmd_mesh_lpn_config_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06230000)
#define gecko_cmd_mesh_friend_init_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00240000)
#define gecko_cmd_mesh_friend_deinit_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01240000)
#define gecko_cmd_mesh_config_client_cancel_request_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00270000)
#define gecko_cmd_mesh_config_client_get_request_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01270000)
#define gecko_cmd_mesh_config_client_get_default_timeout_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2e270000)
#define gecko_cmd_mesh_config_client_set_default_timeout_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2f270000)
#define gecko_cmd_mesh_config_client_add_netkey_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02270000)
#define gecko_cmd_mesh_config_client_remove_netkey_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03270000)
#define gecko_cmd_mesh_config_client_list_netkeys_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04270000)
#define gecko_cmd_mesh_config_client_add_appkey_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05270000)
#define gecko_cmd_mesh_config_client_remove_appkey_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06270000)
#define gecko_cmd_mesh_config_client_list_appkeys_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07270000)
#define gecko_cmd_mesh_config_client_bind_model_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08270000)
#define gecko_cmd_mesh_config_client_unbind_model_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09270000)
#define gecko_cmd_mesh_config_client_list_bindings_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a270000)
#define gecko_cmd_mesh_config_client_get_model_pub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0b270000)
#define gecko_cmd_mesh_config_client_set_model_pub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0c270000)
#define gecko_cmd_mesh_config_client_set_model_pub_va_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0d270000)
#define gecko_cmd_mesh_config_client_add_model_sub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0e270000)
#define gecko_cmd_mesh_config_client_add_model_sub_va_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0f270000)
#define gecko_cmd_mesh_config_client_remove_model_sub_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x10270000)
#define gecko_cmd_mesh_config_client_remove_model_sub_va_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x11270000)
#define gecko_cmd_mesh_config_client_set_model_sub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x12270000)
#define gecko_cmd_mesh_config_client_set_model_sub_va_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x13270000)
#define gecko_cmd_mesh_config_client_clear_model_sub_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x14270000)
#define gecko_cmd_mesh_config_client_list_subs_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x15270000)
#define gecko_cmd_mesh_config_client_get_heartbeat_pub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x16270000)
#define gecko_cmd_mesh_config_client_set_heartbeat_pub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x17270000)
#define gecko_cmd_mesh_config_client_get_heartbeat_sub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x19270000)
#define gecko_cmd_mesh_config_client_set_heartbeat_sub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1a270000)
#define gecko_cmd_mesh_config_client_get_beacon_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1b270000)
#define gecko_cmd_mesh_config_client_set_beacon_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1c270000)
#define gecko_cmd_mesh_config_client_get_default_ttl_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1d270000)
#define gecko_cmd_mesh_config_client_set_default_ttl_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1e270000)
#define gecko_cmd_mesh_config_client_get_gatt_proxy_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x1f270000)
#define gecko_cmd_mesh_config_client_set_gatt_proxy_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x20270000)
#define gecko_cmd_mesh_config_client_get_relay_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x21270000)
#define gecko_cmd_mesh_config_client_set_relay_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x22270000)
#define gecko_cmd_mesh_config_client_get_network_transmit_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x23270000)
#define gecko_cmd_mesh_config_client_set_network_transmit_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x24270000)
#define gecko_cmd_mesh_config_client_get_identity_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x25270000)
#define gecko_cmd_mesh_config_client_set_identity_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x26270000)
#define gecko_cmd_mesh_config_client_get_friend_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x27270000)
#define gecko_cmd_mesh_config_client_set_friend_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x28270000)
#define gecko_cmd_mesh_config_client_get_lpn_polltimeout_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2b270000)
#define gecko_cmd_mesh_config_client_get_dcd_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2c270000)
#define gecko_cmd_mesh_config_client_reset_node_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x2d270000)
#define gecko_cmd_l2cap_coc_send_connection_request_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01430000)
#define gecko_cmd_l2cap_coc_send_connection_response_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02430000)
#define gecko_cmd_l2cap_coc_send_le_flow_control_credit_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03430000)
#define gecko_cmd_l2cap_coc_send_disconnection_request_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04430000)
#define gecko_cmd_l2cap_coc_send_data_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05430000)
#define gecko_cmd_cte_transmitter_enable_cte_response_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00440000)
#define gecko_cmd_cte_transmitter_disable_cte_response_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01440000)
#define gecko_cmd_cte_transmitter_start_connectionless_cte_id         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02440000)
#define gecko_cmd_cte_transmitter_stop_connectionless_cte_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03440000)
#define gecko_cmd_cte_transmitter_set_dtm_parameters_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04440000)
#define gecko_cmd_cte_transmitter_clear_dtm_parameters_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05440000)
#define gecko_cmd_cte_receiver_configure_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00450000)
#define gecko_cmd_cte_receiver_start_iq_sampling_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01450000)
#define gecko_cmd_cte_receiver_stop_iq_sampling_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02450000)
#define gecko_cmd_cte_receiver_start_connectionless_iq_sampling_id    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03450000)
#define gecko_cmd_cte_receiver_stop_connectionless_iq_sampling_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04450000)
#define gecko_cmd_cte_receiver_set_dtm_parameters_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05450000)
#define gecko_cmd_cte_receiver_clear_dtm_parameters_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06450000)
#define gecko_cmd_mesh_sensor_server_init_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00470000)
#define gecko_cmd_mesh_sensor_server_deinit_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01470000)
#define gecko_cmd_mesh_sensor_server_send_descriptor_status_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02470000)
#define gecko_cmd_mesh_sensor_server_send_status_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03470000)
#define gecko_cmd_mesh_sensor_server_send_column_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04470000)
#define gecko_cmd_mesh_sensor_server_send_series_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05470000)
#define gecko_cmd_mesh_sensor_setup_server_send_cadence_status_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00480000)
#define gecko_cmd_mesh_sensor_setup_server_send_settings_status_id    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01480000)
#define gecko_cmd_mesh_sensor_setup_server_send_setting_status_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02480000)
#define gecko_cmd_mesh_sensor_client_init_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00490000)
#define gecko_cmd_mesh_sensor_client_deinit_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x01490000)
#define gecko_cmd_mesh_sensor_client_get_descriptor_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x02490000)
#define gecko_cmd_mesh_sensor_client_get_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x03490000)
#define gecko_cmd_mesh_sensor_client_get_column_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x04490000)
#define gecko_cmd_mesh_sensor_client_get_series_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x05490000)
#define gecko_cmd_mesh_sensor_client_get_cadence_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x06490000)
#define gecko_cmd_mesh_sensor_client_set_cadence_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x07490000)
#define gecko_cmd_mesh_sensor_client_get_settings_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x08490000)
#define gecko_cmd_mesh_sensor_client_get_setting_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x09490000)
#define gecko_cmd_mesh_sensor_client_set_setting_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x0a490000)
#define gecko_cmd_user_message_to_target_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_cmd|0x00ff0000)

#define gecko_rsp_dfu_flash_set_address_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01000000)
#define gecko_rsp_dfu_flash_upload_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02000000)
#define gecko_rsp_dfu_flash_upload_finish_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03000000)
#define gecko_rsp_system_hello_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00010000)
#define gecko_rsp_system_get_bt_address_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03010000)
#define gecko_rsp_system_set_bt_address_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04010000)
#define gecko_rsp_system_set_tx_power_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a010000)
#define gecko_rsp_system_get_random_data_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b010000)
#define gecko_rsp_system_halt_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c010000)
#define gecko_rsp_system_set_device_name_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d010000)
#define gecko_rsp_system_linklayer_configure_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e010000)
#define gecko_rsp_system_get_counters_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f010000)
#define gecko_rsp_system_data_buffer_write_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12010000)
#define gecko_rsp_system_set_identity_address_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13010000)
#define gecko_rsp_system_data_buffer_clear_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x14010000)
#define gecko_rsp_le_gap_open_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00030000)
#define gecko_rsp_le_gap_set_mode_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01030000)
#define gecko_rsp_le_gap_discover_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02030000)
#define gecko_rsp_le_gap_end_procedure_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03030000)
#define gecko_rsp_le_gap_set_adv_parameters_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04030000)
#define gecko_rsp_le_gap_set_conn_parameters_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05030000)
#define gecko_rsp_le_gap_set_scan_parameters_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06030000)
#define gecko_rsp_le_gap_set_adv_data_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07030000)
#define gecko_rsp_le_gap_set_adv_timeout_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08030000)
#define gecko_rsp_le_gap_bt5_set_mode_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a030000)
#define gecko_rsp_le_gap_bt5_set_adv_parameters_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b030000)
#define gecko_rsp_le_gap_bt5_set_adv_data_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c030000)
#define gecko_rsp_le_gap_set_privacy_mode_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d030000)
#define gecko_rsp_le_gap_set_advertise_timing_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e030000)
#define gecko_rsp_le_gap_set_advertise_channel_map_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f030000)
#define gecko_rsp_le_gap_set_advertise_report_scan_request_id         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x10030000)
#define gecko_rsp_le_gap_set_advertise_phy_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x11030000)
#define gecko_rsp_le_gap_set_advertise_configuration_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12030000)
#define gecko_rsp_le_gap_clear_advertise_configuration_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13030000)
#define gecko_rsp_le_gap_start_advertising_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x14030000)
#define gecko_rsp_le_gap_stop_advertising_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x15030000)
#define gecko_rsp_le_gap_set_discovery_timing_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x16030000)
#define gecko_rsp_le_gap_set_discovery_type_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x17030000)
#define gecko_rsp_le_gap_start_discovery_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x18030000)
#define gecko_rsp_le_gap_set_data_channel_classification_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x19030000)
#define gecko_rsp_le_gap_connect_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1a030000)
#define gecko_rsp_le_gap_set_advertise_tx_power_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1b030000)
#define gecko_rsp_le_gap_set_discovery_extended_scan_response_id      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1c030000)
#define gecko_rsp_le_gap_start_periodic_advertising_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1d030000)
#define gecko_rsp_le_gap_stop_periodic_advertising_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1f030000)
#define gecko_rsp_le_gap_set_long_advertising_data_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x20030000)
#define gecko_rsp_le_gap_enable_whitelisting_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x21030000)
#define gecko_rsp_le_gap_set_conn_timing_parameters_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x22030000)
#define gecko_rsp_sync_open_id                                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00420000)
#define gecko_rsp_sync_close_id                                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01420000)
#define gecko_rsp_le_connection_set_parameters_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00080000)
#define gecko_rsp_le_connection_get_rssi_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01080000)
#define gecko_rsp_le_connection_disable_slave_latency_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02080000)
#define gecko_rsp_le_connection_set_phy_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03080000)
#define gecko_rsp_le_connection_close_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04080000)
#define gecko_rsp_le_connection_set_timing_parameters_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05080000)
#define gecko_rsp_gatt_set_max_mtu_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00090000)
#define gecko_rsp_gatt_discover_primary_services_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01090000)
#define gecko_rsp_gatt_discover_primary_services_by_uuid_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02090000)
#define gecko_rsp_gatt_discover_characteristics_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03090000)
#define gecko_rsp_gatt_discover_characteristics_by_uuid_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04090000)
#define gecko_rsp_gatt_set_characteristic_notification_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05090000)
#define gecko_rsp_gatt_discover_descriptors_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06090000)
#define gecko_rsp_gatt_read_characteristic_value_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07090000)
#define gecko_rsp_gatt_read_characteristic_value_by_uuid_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08090000)
#define gecko_rsp_gatt_write_characteristic_value_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09090000)
#define gecko_rsp_gatt_write_characteristic_value_without_response_id  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a090000)
#define gecko_rsp_gatt_prepare_characteristic_value_write_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b090000)
#define gecko_rsp_gatt_execute_characteristic_value_write_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c090000)
#define gecko_rsp_gatt_send_characteristic_confirmation_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d090000)
#define gecko_rsp_gatt_read_descriptor_value_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e090000)
#define gecko_rsp_gatt_write_descriptor_value_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f090000)
#define gecko_rsp_gatt_find_included_services_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x10090000)
#define gecko_rsp_gatt_read_multiple_characteristic_values_id         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x11090000)
#define gecko_rsp_gatt_read_characteristic_value_from_offset_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12090000)
#define gecko_rsp_gatt_prepare_characteristic_value_reliable_write_id  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13090000)
#define gecko_rsp_gatt_server_read_attribute_value_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x000a0000)
#define gecko_rsp_gatt_server_read_attribute_type_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x010a0000)
#define gecko_rsp_gatt_server_write_attribute_value_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x020a0000)
#define gecko_rsp_gatt_server_send_user_read_response_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x030a0000)
#define gecko_rsp_gatt_server_send_user_write_response_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x040a0000)
#define gecko_rsp_gatt_server_send_characteristic_notification_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x050a0000)
#define gecko_rsp_gatt_server_find_attribute_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x060a0000)
#define gecko_rsp_gatt_server_set_capabilities_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x080a0000)
#define gecko_rsp_gatt_server_set_max_mtu_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a0a0000)
#define gecko_rsp_hardware_set_soft_timer_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x000c0000)
#define gecko_rsp_hardware_get_time_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b0c0000)
#define gecko_rsp_hardware_set_lazy_soft_timer_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c0c0000)
#define gecko_rsp_flash_ps_erase_all_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x010d0000)
#define gecko_rsp_flash_ps_save_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x020d0000)
#define gecko_rsp_flash_ps_load_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x030d0000)
#define gecko_rsp_flash_ps_erase_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x040d0000)
#define gecko_rsp_test_dtm_tx_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x000e0000)
#define gecko_rsp_test_dtm_rx_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x010e0000)
#define gecko_rsp_test_dtm_end_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x020e0000)
#define gecko_rsp_sm_set_bondable_mode_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x000f0000)
#define gecko_rsp_sm_configure_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x010f0000)
#define gecko_rsp_sm_store_bonding_configuration_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x020f0000)
#define gecko_rsp_sm_increase_security_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x040f0000)
#define gecko_rsp_sm_delete_bonding_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x060f0000)
#define gecko_rsp_sm_delete_bondings_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x070f0000)
#define gecko_rsp_sm_enter_passkey_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x080f0000)
#define gecko_rsp_sm_passkey_confirm_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x090f0000)
#define gecko_rsp_sm_set_oob_data_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a0f0000)
#define gecko_rsp_sm_list_all_bondings_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b0f0000)
#define gecko_rsp_sm_bonding_confirm_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e0f0000)
#define gecko_rsp_sm_set_debug_mode_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f0f0000)
#define gecko_rsp_sm_set_passkey_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x100f0000)
#define gecko_rsp_sm_use_sc_oob_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x110f0000)
#define gecko_rsp_sm_set_sc_remote_oob_data_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x120f0000)
#define gecko_rsp_sm_add_to_whitelist_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x130f0000)
#define gecko_rsp_sm_set_minimum_key_size_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x140f0000)
#define gecko_rsp_homekit_configure_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00130000)
#define gecko_rsp_homekit_advertise_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01130000)
#define gecko_rsp_homekit_delete_pairings_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02130000)
#define gecko_rsp_homekit_check_authcp_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03130000)
#define gecko_rsp_homekit_get_pairing_id_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04130000)
#define gecko_rsp_homekit_send_write_response_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05130000)
#define gecko_rsp_homekit_send_read_response_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06130000)
#define gecko_rsp_homekit_gsn_action_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07130000)
#define gecko_rsp_homekit_event_notification_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08130000)
#define gecko_rsp_homekit_broadcast_action_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09130000)
#define gecko_rsp_mesh_node_init_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00140000)
#define gecko_rsp_mesh_node_start_unprov_beaconing_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01140000)
#define gecko_rsp_mesh_node_input_oob_request_rsp_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02140000)
#define gecko_rsp_mesh_node_get_uuid_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03140000)
#define gecko_rsp_mesh_node_set_provisioning_data_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04140000)
#define gecko_rsp_mesh_node_init_oob_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05140000)
#define gecko_rsp_mesh_node_set_ivrecovery_mode_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06140000)
#define gecko_rsp_mesh_node_get_ivrecovery_mode_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07140000)
#define gecko_rsp_mesh_node_set_adv_event_filter_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08140000)
#define gecko_rsp_mesh_node_get_statistics_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09140000)
#define gecko_rsp_mesh_node_clear_statistics_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a140000)
#define gecko_rsp_mesh_node_set_net_relay_delay_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b140000)
#define gecko_rsp_mesh_node_get_net_relay_delay_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c140000)
#define gecko_rsp_mesh_node_get_ivupdate_state_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d140000)
#define gecko_rsp_mesh_node_request_ivupdate_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e140000)
#define gecko_rsp_mesh_node_get_seq_remaining_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f140000)
#define gecko_rsp_mesh_node_save_replay_protection_list_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x10140000)
#define gecko_rsp_mesh_node_set_uuid_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x11140000)
#define gecko_rsp_mesh_node_get_element_address_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12140000)
#define gecko_rsp_mesh_node_static_oob_request_rsp_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13140000)
#define gecko_rsp_mesh_node_reset_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x15140000)
#define gecko_rsp_mesh_prov_init_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00150000)
#define gecko_rsp_mesh_prov_scan_unprov_beacons_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01150000)
#define gecko_rsp_mesh_prov_provision_device_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02150000)
#define gecko_rsp_mesh_prov_create_network_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03150000)
#define gecko_rsp_mesh_prov_get_dcd_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04150000)
#define gecko_rsp_mesh_prov_get_config_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05150000)
#define gecko_rsp_mesh_prov_set_config_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06150000)
#define gecko_rsp_mesh_prov_create_appkey_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07150000)
#define gecko_rsp_mesh_prov_oob_pkey_rsp_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08150000)
#define gecko_rsp_mesh_prov_oob_auth_rsp_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09150000)
#define gecko_rsp_mesh_prov_set_oob_requirements_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a150000)
#define gecko_rsp_mesh_prov_key_refresh_start_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b150000)
#define gecko_rsp_mesh_prov_get_key_refresh_blacklist_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c150000)
#define gecko_rsp_mesh_prov_set_key_refresh_blacklist_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d150000)
#define gecko_rsp_mesh_prov_appkey_add_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e150000)
#define gecko_rsp_mesh_prov_appkey_delete_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f150000)
#define gecko_rsp_mesh_prov_model_app_bind_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x10150000)
#define gecko_rsp_mesh_prov_model_app_unbind_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x11150000)
#define gecko_rsp_mesh_prov_model_app_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12150000)
#define gecko_rsp_mesh_prov_model_sub_add_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13150000)
#define gecko_rsp_mesh_prov_model_pub_set_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x14150000)
#define gecko_rsp_mesh_prov_provision_gatt_device_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x15150000)
#define gecko_rsp_mesh_prov_ddb_get_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x16150000)
#define gecko_rsp_mesh_prov_ddb_delete_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x17150000)
#define gecko_rsp_mesh_prov_ddb_add_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x18150000)
#define gecko_rsp_mesh_prov_ddb_list_devices_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x19150000)
#define gecko_rsp_mesh_prov_network_add_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1a150000)
#define gecko_rsp_mesh_prov_network_delete_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1b150000)
#define gecko_rsp_mesh_prov_nettx_get_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1c150000)
#define gecko_rsp_mesh_prov_nettx_set_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1d150000)
#define gecko_rsp_mesh_prov_model_sub_del_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1e150000)
#define gecko_rsp_mesh_prov_model_sub_add_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1f150000)
#define gecko_rsp_mesh_prov_model_sub_del_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x20150000)
#define gecko_rsp_mesh_prov_model_sub_set_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x21150000)
#define gecko_rsp_mesh_prov_model_sub_set_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x22150000)
#define gecko_rsp_mesh_prov_heartbeat_publication_get_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x23150000)
#define gecko_rsp_mesh_prov_heartbeat_publication_set_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x24150000)
#define gecko_rsp_mesh_prov_heartbeat_subscription_get_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x25150000)
#define gecko_rsp_mesh_prov_heartbeat_subscription_set_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x26150000)
#define gecko_rsp_mesh_prov_relay_get_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x27150000)
#define gecko_rsp_mesh_prov_relay_set_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x28150000)
#define gecko_rsp_mesh_prov_reset_node_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x29150000)
#define gecko_rsp_mesh_prov_appkey_get_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2a150000)
#define gecko_rsp_mesh_prov_network_get_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2b150000)
#define gecko_rsp_mesh_prov_model_sub_clear_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2c150000)
#define gecko_rsp_mesh_prov_model_pub_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2d150000)
#define gecko_rsp_mesh_prov_model_pub_set_va_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2e150000)
#define gecko_rsp_mesh_prov_model_pub_set_cred_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2f150000)
#define gecko_rsp_mesh_prov_model_pub_set_va_cred_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x30150000)
#define gecko_rsp_mesh_prov_model_sub_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x31150000)
#define gecko_rsp_mesh_prov_friend_timeout_get_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x32150000)
#define gecko_rsp_mesh_prov_get_default_configuration_timeout_id      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x33150000)
#define gecko_rsp_mesh_prov_set_default_configuration_timeout_id      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x34150000)
#define gecko_rsp_mesh_prov_provision_device_with_address_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x35150000)
#define gecko_rsp_mesh_prov_provision_gatt_device_with_address_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x36150000)
#define gecko_rsp_mesh_prov_initialize_network_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x37150000)
#define gecko_rsp_mesh_prov_get_key_refresh_appkey_blacklist_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x38150000)
#define gecko_rsp_mesh_prov_set_key_refresh_appkey_blacklist_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x39150000)
#define gecko_rsp_mesh_prov_stop_scan_unprov_beacons_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x3a150000)
#define gecko_rsp_mesh_proxy_connect_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00180000)
#define gecko_rsp_mesh_proxy_disconnect_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01180000)
#define gecko_rsp_mesh_proxy_set_filter_type_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02180000)
#define gecko_rsp_mesh_proxy_allow_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03180000)
#define gecko_rsp_mesh_proxy_deny_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04180000)
#define gecko_rsp_mesh_vendor_model_send_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00190000)
#define gecko_rsp_mesh_vendor_model_set_publication_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01190000)
#define gecko_rsp_mesh_vendor_model_clear_publication_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02190000)
#define gecko_rsp_mesh_vendor_model_publish_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03190000)
#define gecko_rsp_mesh_vendor_model_init_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04190000)
#define gecko_rsp_mesh_vendor_model_deinit_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05190000)
#define gecko_rsp_mesh_health_client_get_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x001a0000)
#define gecko_rsp_mesh_health_client_clear_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x011a0000)
#define gecko_rsp_mesh_health_client_test_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x021a0000)
#define gecko_rsp_mesh_health_client_get_period_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x031a0000)
#define gecko_rsp_mesh_health_client_set_period_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x041a0000)
#define gecko_rsp_mesh_health_client_get_attention_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x051a0000)
#define gecko_rsp_mesh_health_client_set_attention_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x061a0000)
#define gecko_rsp_mesh_health_server_set_fault_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x001b0000)
#define gecko_rsp_mesh_health_server_clear_fault_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x011b0000)
#define gecko_rsp_mesh_health_server_test_response_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x021b0000)
#define gecko_rsp_mesh_generic_client_get_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x001e0000)
#define gecko_rsp_mesh_generic_client_set_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x011e0000)
#define gecko_rsp_mesh_generic_client_publish_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x021e0000)
#define gecko_rsp_mesh_generic_client_get_params_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x031e0000)
#define gecko_rsp_mesh_generic_client_init_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x041e0000)
#define gecko_rsp_mesh_generic_server_response_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x001f0000)
#define gecko_rsp_mesh_generic_server_update_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x011f0000)
#define gecko_rsp_mesh_generic_server_publish_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x021f0000)
#define gecko_rsp_mesh_generic_server_init_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x041f0000)
#define gecko_rsp_coex_set_options_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00200000)
#define gecko_rsp_coex_get_counters_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01200000)
#define gecko_rsp_mesh_test_get_nettx_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00220000)
#define gecko_rsp_mesh_test_set_nettx_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01220000)
#define gecko_rsp_mesh_test_get_relay_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02220000)
#define gecko_rsp_mesh_test_set_relay_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03220000)
#define gecko_rsp_mesh_test_set_adv_scan_params_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04220000)
#define gecko_rsp_mesh_test_set_ivupdate_test_mode_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05220000)
#define gecko_rsp_mesh_test_get_ivupdate_test_mode_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06220000)
#define gecko_rsp_mesh_test_set_segment_send_delay_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07220000)
#define gecko_rsp_mesh_test_set_ivupdate_state_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08220000)
#define gecko_rsp_mesh_test_send_beacons_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09220000)
#define gecko_rsp_mesh_test_bind_local_model_app_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a220000)
#define gecko_rsp_mesh_test_unbind_local_model_app_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b220000)
#define gecko_rsp_mesh_test_add_local_model_sub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c220000)
#define gecko_rsp_mesh_test_del_local_model_sub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d220000)
#define gecko_rsp_mesh_test_add_local_model_sub_va_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e220000)
#define gecko_rsp_mesh_test_del_local_model_sub_va_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f220000)
#define gecko_rsp_mesh_test_get_local_model_sub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x10220000)
#define gecko_rsp_mesh_test_set_local_model_pub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x11220000)
#define gecko_rsp_mesh_test_set_local_model_pub_va_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12220000)
#define gecko_rsp_mesh_test_get_local_model_pub_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13220000)
#define gecko_rsp_mesh_test_set_local_heartbeat_subscription_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x14220000)
#define gecko_rsp_mesh_test_get_local_heartbeat_subscription_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x15220000)
#define gecko_rsp_mesh_test_get_local_heartbeat_publication_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x16220000)
#define gecko_rsp_mesh_test_set_local_heartbeat_publication_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x17220000)
#define gecko_rsp_mesh_test_set_local_config_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x18220000)
#define gecko_rsp_mesh_test_get_local_config_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x19220000)
#define gecko_rsp_mesh_test_add_local_key_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1a220000)
#define gecko_rsp_mesh_test_del_local_key_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1b220000)
#define gecko_rsp_mesh_test_update_local_key_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1c220000)
#define gecko_rsp_mesh_test_set_sar_config_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1d220000)
#define gecko_rsp_mesh_test_get_element_seqnum_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1e220000)
#define gecko_rsp_mesh_test_set_adv_bearer_state_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1f220000)
#define gecko_rsp_mesh_test_get_key_count_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x20220000)
#define gecko_rsp_mesh_test_get_key_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x21220000)
#define gecko_rsp_mesh_test_prov_get_device_key_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x23220000)
#define gecko_rsp_mesh_test_prov_prepare_key_refresh_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x24220000)
#define gecko_rsp_mesh_test_cancel_segmented_tx_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x25220000)
#define gecko_rsp_mesh_test_set_iv_index_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x26220000)
#define gecko_rsp_mesh_test_set_element_seqnum_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x27220000)
#define gecko_rsp_mesh_lpn_init_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00230000)
#define gecko_rsp_mesh_lpn_deinit_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01230000)
#define gecko_rsp_mesh_lpn_configure_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02230000)
#define gecko_rsp_mesh_lpn_establish_friendship_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03230000)
#define gecko_rsp_mesh_lpn_poll_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04230000)
#define gecko_rsp_mesh_lpn_terminate_friendship_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05230000)
#define gecko_rsp_mesh_lpn_config_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06230000)
#define gecko_rsp_mesh_friend_init_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00240000)
#define gecko_rsp_mesh_friend_deinit_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01240000)
#define gecko_rsp_mesh_config_client_cancel_request_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00270000)
#define gecko_rsp_mesh_config_client_get_request_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01270000)
#define gecko_rsp_mesh_config_client_get_default_timeout_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2e270000)
#define gecko_rsp_mesh_config_client_set_default_timeout_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2f270000)
#define gecko_rsp_mesh_config_client_add_netkey_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02270000)
#define gecko_rsp_mesh_config_client_remove_netkey_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03270000)
#define gecko_rsp_mesh_config_client_list_netkeys_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04270000)
#define gecko_rsp_mesh_config_client_add_appkey_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05270000)
#define gecko_rsp_mesh_config_client_remove_appkey_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06270000)
#define gecko_rsp_mesh_config_client_list_appkeys_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07270000)
#define gecko_rsp_mesh_config_client_bind_model_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08270000)
#define gecko_rsp_mesh_config_client_unbind_model_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09270000)
#define gecko_rsp_mesh_config_client_list_bindings_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a270000)
#define gecko_rsp_mesh_config_client_get_model_pub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0b270000)
#define gecko_rsp_mesh_config_client_set_model_pub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0c270000)
#define gecko_rsp_mesh_config_client_set_model_pub_va_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0d270000)
#define gecko_rsp_mesh_config_client_add_model_sub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0e270000)
#define gecko_rsp_mesh_config_client_add_model_sub_va_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0f270000)
#define gecko_rsp_mesh_config_client_remove_model_sub_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x10270000)
#define gecko_rsp_mesh_config_client_remove_model_sub_va_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x11270000)
#define gecko_rsp_mesh_config_client_set_model_sub_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x12270000)
#define gecko_rsp_mesh_config_client_set_model_sub_va_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x13270000)
#define gecko_rsp_mesh_config_client_clear_model_sub_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x14270000)
#define gecko_rsp_mesh_config_client_list_subs_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x15270000)
#define gecko_rsp_mesh_config_client_get_heartbeat_pub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x16270000)
#define gecko_rsp_mesh_config_client_set_heartbeat_pub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x17270000)
#define gecko_rsp_mesh_config_client_get_heartbeat_sub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x19270000)
#define gecko_rsp_mesh_config_client_set_heartbeat_sub_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1a270000)
#define gecko_rsp_mesh_config_client_get_beacon_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1b270000)
#define gecko_rsp_mesh_config_client_set_beacon_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1c270000)
#define gecko_rsp_mesh_config_client_get_default_ttl_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1d270000)
#define gecko_rsp_mesh_config_client_set_default_ttl_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1e270000)
#define gecko_rsp_mesh_config_client_get_gatt_proxy_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x1f270000)
#define gecko_rsp_mesh_config_client_set_gatt_proxy_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x20270000)
#define gecko_rsp_mesh_config_client_get_relay_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x21270000)
#define gecko_rsp_mesh_config_client_set_relay_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x22270000)
#define gecko_rsp_mesh_config_client_get_network_transmit_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x23270000)
#define gecko_rsp_mesh_config_client_set_network_transmit_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x24270000)
#define gecko_rsp_mesh_config_client_get_identity_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x25270000)
#define gecko_rsp_mesh_config_client_set_identity_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x26270000)
#define gecko_rsp_mesh_config_client_get_friend_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x27270000)
#define gecko_rsp_mesh_config_client_set_friend_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x28270000)
#define gecko_rsp_mesh_config_client_get_lpn_polltimeout_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2b270000)
#define gecko_rsp_mesh_config_client_get_dcd_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2c270000)
#define gecko_rsp_mesh_config_client_reset_node_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x2d270000)
#define gecko_rsp_l2cap_coc_send_connection_request_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01430000)
#define gecko_rsp_l2cap_coc_send_connection_response_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02430000)
#define gecko_rsp_l2cap_coc_send_le_flow_control_credit_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03430000)
#define gecko_rsp_l2cap_coc_send_disconnection_request_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04430000)
#define gecko_rsp_l2cap_coc_send_data_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05430000)
#define gecko_rsp_cte_transmitter_enable_cte_response_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00440000)
#define gecko_rsp_cte_transmitter_disable_cte_response_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01440000)
#define gecko_rsp_cte_transmitter_start_connectionless_cte_id         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02440000)
#define gecko_rsp_cte_transmitter_stop_connectionless_cte_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03440000)
#define gecko_rsp_cte_transmitter_set_dtm_parameters_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04440000)
#define gecko_rsp_cte_transmitter_clear_dtm_parameters_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05440000)
#define gecko_rsp_cte_receiver_configure_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00450000)
#define gecko_rsp_cte_receiver_start_iq_sampling_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01450000)
#define gecko_rsp_cte_receiver_stop_iq_sampling_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02450000)
#define gecko_rsp_cte_receiver_start_connectionless_iq_sampling_id    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03450000)
#define gecko_rsp_cte_receiver_stop_connectionless_iq_sampling_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04450000)
#define gecko_rsp_cte_receiver_set_dtm_parameters_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05450000)
#define gecko_rsp_cte_receiver_clear_dtm_parameters_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06450000)
#define gecko_rsp_mesh_sensor_server_init_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00470000)
#define gecko_rsp_mesh_sensor_server_deinit_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01470000)
#define gecko_rsp_mesh_sensor_server_send_descriptor_status_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02470000)
#define gecko_rsp_mesh_sensor_server_send_status_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03470000)
#define gecko_rsp_mesh_sensor_server_send_column_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04470000)
#define gecko_rsp_mesh_sensor_server_send_series_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05470000)
#define gecko_rsp_mesh_sensor_setup_server_send_cadence_status_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00480000)
#define gecko_rsp_mesh_sensor_setup_server_send_settings_status_id    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01480000)
#define gecko_rsp_mesh_sensor_setup_server_send_setting_status_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02480000)
#define gecko_rsp_mesh_sensor_client_init_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00490000)
#define gecko_rsp_mesh_sensor_client_deinit_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x01490000)
#define gecko_rsp_mesh_sensor_client_get_descriptor_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x02490000)
#define gecko_rsp_mesh_sensor_client_get_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x03490000)
#define gecko_rsp_mesh_sensor_client_get_column_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x04490000)
#define gecko_rsp_mesh_sensor_client_get_series_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x05490000)
#define gecko_rsp_mesh_sensor_client_get_cadence_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x06490000)
#define gecko_rsp_mesh_sensor_client_set_cadence_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x07490000)
#define gecko_rsp_mesh_sensor_client_get_settings_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x08490000)
#define gecko_rsp_mesh_sensor_client_get_setting_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x09490000)
#define gecko_rsp_mesh_sensor_client_set_setting_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x0a490000)
#define gecko_rsp_user_message_to_target_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_rsp|0x00ff0000)

#define gecko_evt_dfu_boot_id                                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00000000)
#define gecko_evt_dfu_boot_failure_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01000000)
#define gecko_evt_system_boot_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00010000)
#define gecko_evt_system_external_signal_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03010000)
#define gecko_evt_system_awake_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04010000)
#define gecko_evt_system_hardware_error_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05010000)
#define gecko_evt_system_error_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06010000)
#define gecko_evt_le_gap_scan_response_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00030000)
#define gecko_evt_le_gap_adv_timeout_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01030000)
#define gecko_evt_le_gap_scan_request_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02030000)
#define gecko_evt_le_gap_extended_scan_response_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04030000)
#define gecko_evt_le_gap_periodic_advertising_status_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05030000)
#define gecko_evt_sync_opened_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00420000)
#define gecko_evt_sync_closed_id                                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01420000)
#define gecko_evt_sync_data_id                                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02420000)
#define gecko_evt_le_connection_opened_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00080000)
#define gecko_evt_le_connection_closed_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01080000)
#define gecko_evt_le_connection_parameters_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02080000)
#define gecko_evt_le_connection_rssi_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03080000)
#define gecko_evt_le_connection_phy_status_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04080000)
#define gecko_evt_gatt_mtu_exchanged_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00090000)
#define gecko_evt_gatt_service_id                                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01090000)
#define gecko_evt_gatt_characteristic_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02090000)
#define gecko_evt_gatt_descriptor_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03090000)
#define gecko_evt_gatt_characteristic_value_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04090000)
#define gecko_evt_gatt_descriptor_value_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05090000)
#define gecko_evt_gatt_procedure_completed_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06090000)
#define gecko_evt_gatt_server_attribute_value_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x000a0000)
#define gecko_evt_gatt_server_user_read_request_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x010a0000)
#define gecko_evt_gatt_server_user_write_request_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x020a0000)
#define gecko_evt_gatt_server_characteristic_status_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x030a0000)
#define gecko_evt_gatt_server_execute_write_completed_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x040a0000)
#define gecko_evt_hardware_soft_timer_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x000c0000)
#define gecko_evt_test_dtm_completed_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x000e0000)
#define gecko_evt_sm_passkey_display_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x000f0000)
#define gecko_evt_sm_passkey_request_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x010f0000)
#define gecko_evt_sm_confirm_passkey_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x020f0000)
#define gecko_evt_sm_bonded_id                                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x030f0000)
#define gecko_evt_sm_bonding_failed_id                                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x040f0000)
#define gecko_evt_sm_list_bonding_entry_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x050f0000)
#define gecko_evt_sm_list_all_bondings_complete_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x060f0000)
#define gecko_evt_sm_confirm_bonding_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x090f0000)
#define gecko_evt_homekit_setupcode_display_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00130000)
#define gecko_evt_homekit_paired_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01130000)
#define gecko_evt_homekit_pair_verified_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02130000)
#define gecko_evt_homekit_connection_opened_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03130000)
#define gecko_evt_homekit_connection_closed_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04130000)
#define gecko_evt_homekit_identify_id                                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05130000)
#define gecko_evt_homekit_write_request_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06130000)
#define gecko_evt_homekit_read_request_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x07130000)
#define gecko_evt_homekit_disconnection_required_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x08130000)
#define gecko_evt_homekit_pairing_removed_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x09130000)
#define gecko_evt_homekit_setuppayload_display_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0a130000)
#define gecko_evt_mesh_node_initialized_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00140000)
#define gecko_evt_mesh_node_provisioned_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01140000)
#define gecko_evt_mesh_node_config_get_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02140000)
#define gecko_evt_mesh_node_config_set_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03140000)
#define gecko_evt_mesh_node_display_output_oob_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04140000)
#define gecko_evt_mesh_node_input_oob_request_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05140000)
#define gecko_evt_mesh_node_provisioning_started_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06140000)
#define gecko_evt_mesh_node_provisioning_failed_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x07140000)
#define gecko_evt_mesh_node_key_added_id                              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x08140000)
#define gecko_evt_mesh_node_model_config_changed_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x09140000)
#define gecko_evt_mesh_node_reset_id                                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0a140000)
#define gecko_evt_mesh_node_ivrecovery_needed_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0b140000)
#define gecko_evt_mesh_node_changed_ivupdate_state_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0c140000)
#define gecko_evt_mesh_node_static_oob_request_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0d140000)
#define gecko_evt_mesh_node_key_removed_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0e140000)
#define gecko_evt_mesh_node_key_updated_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0f140000)
#define gecko_evt_mesh_prov_initialized_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00150000)
#define gecko_evt_mesh_prov_provisioning_failed_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01150000)
#define gecko_evt_mesh_prov_device_provisioned_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02150000)
#define gecko_evt_mesh_prov_unprov_beacon_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03150000)
#define gecko_evt_mesh_prov_dcd_status_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04150000)
#define gecko_evt_mesh_prov_config_status_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05150000)
#define gecko_evt_mesh_prov_oob_pkey_request_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06150000)
#define gecko_evt_mesh_prov_oob_auth_request_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x07150000)
#define gecko_evt_mesh_prov_oob_display_input_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x08150000)
#define gecko_evt_mesh_prov_ddb_list_id                               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x09150000)
#define gecko_evt_mesh_prov_heartbeat_publication_status_id           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0a150000)
#define gecko_evt_mesh_prov_heartbeat_subscription_status_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0b150000)
#define gecko_evt_mesh_prov_relay_status_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0c150000)
#define gecko_evt_mesh_prov_uri_id                                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0d150000)
#define gecko_evt_mesh_prov_node_reset_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0e150000)
#define gecko_evt_mesh_prov_appkey_list_id                            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0f150000)
#define gecko_evt_mesh_prov_appkey_list_end_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x10150000)
#define gecko_evt_mesh_prov_network_list_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x11150000)
#define gecko_evt_mesh_prov_network_list_end_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x12150000)
#define gecko_evt_mesh_prov_model_pub_status_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x13150000)
#define gecko_evt_mesh_prov_key_refresh_phase_update_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x14150000)
#define gecko_evt_mesh_prov_key_refresh_node_update_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x15150000)
#define gecko_evt_mesh_prov_key_refresh_complete_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x16150000)
#define gecko_evt_mesh_prov_model_sub_addr_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x17150000)
#define gecko_evt_mesh_prov_model_sub_addr_end_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x18150000)
#define gecko_evt_mesh_prov_friend_timeout_status_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x19150000)
#define gecko_evt_mesh_proxy_connected_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00180000)
#define gecko_evt_mesh_proxy_disconnected_id                          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01180000)
#define gecko_evt_mesh_proxy_filter_status_id                         (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02180000)
#define gecko_evt_mesh_vendor_model_receive_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00190000)
#define gecko_evt_mesh_health_client_server_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x001a0000)
#define gecko_evt_mesh_health_client_server_status_period_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x011a0000)
#define gecko_evt_mesh_health_client_server_status_attention_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x021a0000)
#define gecko_evt_mesh_health_server_attention_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x001b0000)
#define gecko_evt_mesh_health_server_test_request_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x011b0000)
#define gecko_evt_mesh_generic_client_server_status_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x001e0000)
#define gecko_evt_mesh_generic_server_client_request_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x001f0000)
#define gecko_evt_mesh_generic_server_state_changed_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x011f0000)
#define gecko_evt_mesh_test_local_heartbeat_subscription_complete_id  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00220000)
#define gecko_evt_mesh_lpn_friendship_established_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00230000)
#define gecko_evt_mesh_lpn_friendship_failed_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01230000)
#define gecko_evt_mesh_lpn_friendship_terminated_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02230000)
#define gecko_evt_mesh_friend_friendship_established_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00240000)
#define gecko_evt_mesh_friend_friendship_terminated_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01240000)
#define gecko_evt_mesh_config_client_request_modified_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00270000)
#define gecko_evt_mesh_config_client_netkey_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01270000)
#define gecko_evt_mesh_config_client_netkey_list_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02270000)
#define gecko_evt_mesh_config_client_netkey_list_end_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03270000)
#define gecko_evt_mesh_config_client_appkey_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04270000)
#define gecko_evt_mesh_config_client_appkey_list_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05270000)
#define gecko_evt_mesh_config_client_appkey_list_end_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06270000)
#define gecko_evt_mesh_config_client_binding_status_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x07270000)
#define gecko_evt_mesh_config_client_bindings_list_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x08270000)
#define gecko_evt_mesh_config_client_bindings_list_end_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x09270000)
#define gecko_evt_mesh_config_client_model_pub_status_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0a270000)
#define gecko_evt_mesh_config_client_model_sub_status_id              (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0b270000)
#define gecko_evt_mesh_config_client_subs_list_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0c270000)
#define gecko_evt_mesh_config_client_subs_list_end_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0d270000)
#define gecko_evt_mesh_config_client_heartbeat_pub_status_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0e270000)
#define gecko_evt_mesh_config_client_heartbeat_sub_status_id          (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x0f270000)
#define gecko_evt_mesh_config_client_beacon_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x10270000)
#define gecko_evt_mesh_config_client_default_ttl_status_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x11270000)
#define gecko_evt_mesh_config_client_gatt_proxy_status_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x12270000)
#define gecko_evt_mesh_config_client_relay_status_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x13270000)
#define gecko_evt_mesh_config_client_network_transmit_status_id       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x14270000)
#define gecko_evt_mesh_config_client_identity_status_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x15270000)
#define gecko_evt_mesh_config_client_friend_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x16270000)
#define gecko_evt_mesh_config_client_lpn_polltimeout_status_id        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x18270000)
#define gecko_evt_mesh_config_client_dcd_data_id                      (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x19270000)
#define gecko_evt_mesh_config_client_dcd_data_end_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x1a270000)
#define gecko_evt_mesh_config_client_reset_status_id                  (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x1b270000)
#define gecko_evt_l2cap_coc_connection_request_id                     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01430000)
#define gecko_evt_l2cap_coc_connection_response_id                    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02430000)
#define gecko_evt_l2cap_coc_le_flow_control_credit_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03430000)
#define gecko_evt_l2cap_coc_channel_disconnected_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04430000)
#define gecko_evt_l2cap_coc_data_id                                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05430000)
#define gecko_evt_l2cap_command_rejected_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06430000)
#define gecko_evt_cte_receiver_iq_report_id                           (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00450000)
#define gecko_evt_mesh_sensor_server_get_request_id                   (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01470000)
#define gecko_evt_mesh_sensor_server_get_column_request_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02470000)
#define gecko_evt_mesh_sensor_server_get_series_request_id            (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03470000)
#define gecko_evt_mesh_sensor_server_publish_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04470000)
#define gecko_evt_mesh_sensor_setup_server_get_cadence_request_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00480000)
#define gecko_evt_mesh_sensor_setup_server_set_cadence_request_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01480000)
#define gecko_evt_mesh_sensor_setup_server_get_settings_request_id    (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02480000)
#define gecko_evt_mesh_sensor_setup_server_get_setting_request_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03480000)
#define gecko_evt_mesh_sensor_setup_server_set_setting_request_id     (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04480000)
#define gecko_evt_mesh_sensor_setup_server_publish_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05480000)
#define gecko_evt_mesh_sensor_client_descriptor_status_id             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00490000)
#define gecko_evt_mesh_sensor_client_cadence_status_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x01490000)
#define gecko_evt_mesh_sensor_client_settings_status_id               (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x02490000)
#define gecko_evt_mesh_sensor_client_setting_status_id                (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x03490000)
#define gecko_evt_mesh_sensor_client_status_id                        (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x04490000)
#define gecko_evt_mesh_sensor_client_column_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x05490000)
#define gecko_evt_mesh_sensor_client_series_status_id                 (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x06490000)
#define gecko_evt_mesh_sensor_client_publish_id                       (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x07490000)
#define gecko_evt_user_message_to_host_id                             (((uint32)gecko_dev_type_gecko)|gecko_msg_type_evt|0x00ff0000)


PACKSTRUCT( struct gecko_msg_dfu_reset_cmd_t
{
    uint8               dfu;
});
PACKSTRUCT( struct gecko_msg_dfu_flash_set_address_cmd_t
{
    uint32              address;
});
PACKSTRUCT( struct gecko_msg_dfu_flash_set_address_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_dfu_flash_upload_cmd_t
{
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_dfu_flash_upload_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_dfu_flash_upload_finish_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_dfu_boot_evt_t
{
    uint32              version;
});
PACKSTRUCT( struct gecko_msg_dfu_boot_failure_evt_t
{
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_system_hello_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_reset_cmd_t
{
    uint8               dfu;
});
PACKSTRUCT( struct gecko_msg_system_get_bt_address_rsp_t
{
    bd_addr             address;
});
PACKSTRUCT( struct gecko_msg_system_set_bt_address_cmd_t
{
    bd_addr             address;
});
PACKSTRUCT( struct gecko_msg_system_set_bt_address_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_set_tx_power_cmd_t
{
    int16               power;
});
PACKSTRUCT( struct gecko_msg_system_set_tx_power_rsp_t
{
    int16               set_power;
});
PACKSTRUCT( struct gecko_msg_system_get_random_data_cmd_t
{
    uint8               length;
});
PACKSTRUCT( struct gecko_msg_system_get_random_data_rsp_t
{
    uint16              result;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_system_halt_cmd_t
{
    uint8               halt;
});
PACKSTRUCT( struct gecko_msg_system_halt_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_set_device_name_cmd_t
{
    uint8               type;
    uint8array          name;
});
PACKSTRUCT( struct gecko_msg_system_set_device_name_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_linklayer_configure_cmd_t
{
    uint8               key;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_system_linklayer_configure_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_get_counters_cmd_t
{
    uint8               reset;
});
PACKSTRUCT( struct gecko_msg_system_get_counters_rsp_t
{
    uint16              result;
    uint16              tx_packets;
    uint16              rx_packets;
    uint16              crc_errors;
    uint16              failures;
});
PACKSTRUCT( struct gecko_msg_system_data_buffer_write_cmd_t
{
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_system_data_buffer_write_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_set_identity_address_cmd_t
{
    bd_addr             address;
    uint8               type;
});
PACKSTRUCT( struct gecko_msg_system_set_identity_address_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_data_buffer_clear_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_system_boot_evt_t
{
    uint16              major;
    uint16              minor;
    uint16              patch;
    uint16              build;
    uint32              bootloader;
    uint16              hw;
    uint32              hash;
});
PACKSTRUCT( struct gecko_msg_system_external_signal_evt_t
{
    uint32              extsignals;
});
PACKSTRUCT( struct gecko_msg_system_hardware_error_evt_t
{
    uint16              status;
});
PACKSTRUCT( struct gecko_msg_system_error_evt_t
{
    uint16              reason;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_le_gap_open_cmd_t
{
    bd_addr             address;
    uint8               address_type;
});
PACKSTRUCT( struct gecko_msg_le_gap_open_rsp_t
{
    uint16              result;
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_mode_cmd_t
{
    uint8               discover;
    uint8               connect;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_discover_cmd_t
{
    uint8               mode;
});
PACKSTRUCT( struct gecko_msg_le_gap_discover_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_end_procedure_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_adv_parameters_cmd_t
{
    uint16              interval_min;
    uint16              interval_max;
    uint8               channel_map;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_adv_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_conn_parameters_cmd_t
{
    uint16              min_interval;
    uint16              max_interval;
    uint16              latency;
    uint16              timeout;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_conn_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_scan_parameters_cmd_t
{
    uint16              scan_interval;
    uint16              scan_window;
    uint8               active;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_scan_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_adv_data_cmd_t
{
    uint8               scan_rsp;
    uint8array          adv_data;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_adv_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_adv_timeout_cmd_t
{
    uint8               maxevents;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_adv_timeout_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_bt5_set_mode_cmd_t
{
    uint8               handle;
    uint8               discover;
    uint8               connect;
    uint16              maxevents;
    uint8               address_type;
});
PACKSTRUCT( struct gecko_msg_le_gap_bt5_set_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_bt5_set_adv_parameters_cmd_t
{
    uint8               handle;
    uint16              interval_min;
    uint16              interval_max;
    uint8               channel_map;
    uint8               report_scan;
});
PACKSTRUCT( struct gecko_msg_le_gap_bt5_set_adv_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_bt5_set_adv_data_cmd_t
{
    uint8               handle;
    uint8               scan_rsp;
    uint8array          adv_data;
});
PACKSTRUCT( struct gecko_msg_le_gap_bt5_set_adv_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_privacy_mode_cmd_t
{
    uint8               privacy;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_privacy_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_timing_cmd_t
{
    uint8               handle;
    uint32              interval_min;
    uint32              interval_max;
    uint16              duration;
    uint8               maxevents;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_timing_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_channel_map_cmd_t
{
    uint8               handle;
    uint8               channel_map;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_channel_map_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_report_scan_request_cmd_t
{
    uint8               handle;
    uint8               report_scan_req;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_report_scan_request_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_phy_cmd_t
{
    uint8               handle;
    uint8               primary_phy;
    uint8               secondary_phy;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_phy_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_configuration_cmd_t
{
    uint8               handle;
    uint32              configurations;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_configuration_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_clear_advertise_configuration_cmd_t
{
    uint8               handle;
    uint32              configurations;
});
PACKSTRUCT( struct gecko_msg_le_gap_clear_advertise_configuration_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_start_advertising_cmd_t
{
    uint8               handle;
    uint8               discover;
    uint8               connect;
});
PACKSTRUCT( struct gecko_msg_le_gap_start_advertising_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_stop_advertising_cmd_t
{
    uint8               handle;
});
PACKSTRUCT( struct gecko_msg_le_gap_stop_advertising_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_discovery_timing_cmd_t
{
    uint8               phys;
    uint16              scan_interval;
    uint16              scan_window;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_discovery_timing_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_discovery_type_cmd_t
{
    uint8               phys;
    uint8               scan_type;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_discovery_type_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_start_discovery_cmd_t
{
    uint8               scanning_phy;
    uint8               mode;
});
PACKSTRUCT( struct gecko_msg_le_gap_start_discovery_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_data_channel_classification_cmd_t
{
    uint8array          channel_map;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_data_channel_classification_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_connect_cmd_t
{
    bd_addr             address;
    uint8               address_type;
    uint8               initiating_phy;
});
PACKSTRUCT( struct gecko_msg_le_gap_connect_rsp_t
{
    uint16              result;
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_tx_power_cmd_t
{
    uint8               handle;
    int16               power;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_advertise_tx_power_rsp_t
{
    uint16              result;
    int16               set_power;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_discovery_extended_scan_response_cmd_t
{
    uint8               enable;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_discovery_extended_scan_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_start_periodic_advertising_cmd_t
{
    uint8               handle;
    uint16              interval_min;
    uint16              interval_max;
    uint32              flags;
});
PACKSTRUCT( struct gecko_msg_le_gap_start_periodic_advertising_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_stop_periodic_advertising_cmd_t
{
    uint8               handle;
});
PACKSTRUCT( struct gecko_msg_le_gap_stop_periodic_advertising_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_long_advertising_data_cmd_t
{
    uint8               handle;
    uint8               packet_type;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_long_advertising_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_enable_whitelisting_cmd_t
{
    uint8               enable;
});
PACKSTRUCT( struct gecko_msg_le_gap_enable_whitelisting_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_conn_timing_parameters_cmd_t
{
    uint16              min_interval;
    uint16              max_interval;
    uint16              latency;
    uint16              timeout;
    uint16              min_ce_length;
    uint16              max_ce_length;
});
PACKSTRUCT( struct gecko_msg_le_gap_set_conn_timing_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_gap_scan_response_evt_t
{
    int8                rssi;
    uint8               packet_type;
    bd_addr             address;
    uint8               address_type;
    uint8               bonding;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_le_gap_adv_timeout_evt_t
{
    uint8               handle;
});
PACKSTRUCT( struct gecko_msg_le_gap_scan_request_evt_t
{
    uint8               handle;
    bd_addr             address;
    uint8               address_type;
    uint8               bonding;
});
PACKSTRUCT( struct gecko_msg_le_gap_extended_scan_response_evt_t
{
    uint8               packet_type;
    bd_addr             address;
    uint8               address_type;
    uint8               bonding;
    uint8               primary_phy;
    uint8               secondary_phy;
    uint8               adv_sid;
    int8                tx_power;
    int8                rssi;
    uint8               channel;
    uint16              periodic_interval;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_le_gap_periodic_advertising_status_evt_t
{
    uint8               sid;
    uint32              status;
});
PACKSTRUCT( struct gecko_msg_sync_open_cmd_t
{
    uint8               adv_sid;
    uint16              skip;
    uint16              timeout;
    bd_addr             address;
    uint8               address_type;
});
PACKSTRUCT( struct gecko_msg_sync_open_rsp_t
{
    uint16              result;
    uint8               sync;
});
PACKSTRUCT( struct gecko_msg_sync_close_cmd_t
{
    uint8               sync;
});
PACKSTRUCT( struct gecko_msg_sync_close_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sync_opened_evt_t
{
    uint8               sync;
    uint8               adv_sid;
    bd_addr             address;
    uint8               address_type;
    uint8               adv_phy;
    uint16              adv_interval;
    uint16              clock_accuracy;
});
PACKSTRUCT( struct gecko_msg_sync_closed_evt_t
{
    uint16              reason;
    uint8               sync;
});
PACKSTRUCT( struct gecko_msg_sync_data_evt_t
{
    uint8               sync;
    int8                tx_power;
    int8                rssi;
    uint8               data_status;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_le_connection_set_parameters_cmd_t
{
    uint8               connection;
    uint16              min_interval;
    uint16              max_interval;
    uint16              latency;
    uint16              timeout;
});
PACKSTRUCT( struct gecko_msg_le_connection_set_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_connection_get_rssi_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_le_connection_get_rssi_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_connection_disable_slave_latency_cmd_t
{
    uint8               connection;
    uint8               disable;
});
PACKSTRUCT( struct gecko_msg_le_connection_disable_slave_latency_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_connection_set_phy_cmd_t
{
    uint8               connection;
    uint8               phy;
});
PACKSTRUCT( struct gecko_msg_le_connection_set_phy_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_connection_close_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_le_connection_close_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_connection_set_timing_parameters_cmd_t
{
    uint8               connection;
    uint16              min_interval;
    uint16              max_interval;
    uint16              latency;
    uint16              timeout;
    uint16              min_ce_length;
    uint16              max_ce_length;
});
PACKSTRUCT( struct gecko_msg_le_connection_set_timing_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_le_connection_opened_evt_t
{
    bd_addr             address;
    uint8               address_type;
    uint8               master;
    uint8               connection;
    uint8               bonding;
    uint8               advertiser;
});
PACKSTRUCT( struct gecko_msg_le_connection_closed_evt_t
{
    uint16              reason;
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_le_connection_parameters_evt_t
{
    uint8               connection;
    uint16              interval;
    uint16              latency;
    uint16              timeout;
    uint8               security_mode;
    uint16              txsize;
});
PACKSTRUCT( struct gecko_msg_le_connection_rssi_evt_t
{
    uint8               connection;
    uint8               status;
    int8                rssi;
});
PACKSTRUCT( struct gecko_msg_le_connection_phy_status_evt_t
{
    uint8               connection;
    uint8               phy;
});
PACKSTRUCT( struct gecko_msg_gatt_set_max_mtu_cmd_t
{
    uint16              max_mtu;
});
PACKSTRUCT( struct gecko_msg_gatt_set_max_mtu_rsp_t
{
    uint16              result;
    uint16              max_mtu;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_primary_services_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_primary_services_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_primary_services_by_uuid_cmd_t
{
    uint8               connection;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_characteristics_cmd_t
{
    uint8               connection;
    uint32              service;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_characteristics_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_characteristics_by_uuid_cmd_t
{
    uint8               connection;
    uint32              service;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_set_characteristic_notification_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               flags;
});
PACKSTRUCT( struct gecko_msg_gatt_set_characteristic_notification_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_descriptors_cmd_t
{
    uint8               connection;
    uint16              characteristic;
});
PACKSTRUCT( struct gecko_msg_gatt_discover_descriptors_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_read_characteristic_value_cmd_t
{
    uint8               connection;
    uint16              characteristic;
});
PACKSTRUCT( struct gecko_msg_gatt_read_characteristic_value_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_read_characteristic_value_by_uuid_cmd_t
{
    uint8               connection;
    uint32              service;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_gatt_read_characteristic_value_by_uuid_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_write_characteristic_value_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_write_characteristic_value_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_write_characteristic_value_without_response_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_write_characteristic_value_without_response_rsp_t
{
    uint16              result;
    uint16              sent_len;
});
PACKSTRUCT( struct gecko_msg_gatt_prepare_characteristic_value_write_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_prepare_characteristic_value_write_rsp_t
{
    uint16              result;
    uint16              sent_len;
});
PACKSTRUCT( struct gecko_msg_gatt_execute_characteristic_value_write_cmd_t
{
    uint8               connection;
    uint8               flags;
});
PACKSTRUCT( struct gecko_msg_gatt_execute_characteristic_value_write_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_send_characteristic_confirmation_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_gatt_send_characteristic_confirmation_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_read_descriptor_value_cmd_t
{
    uint8               connection;
    uint16              descriptor;
});
PACKSTRUCT( struct gecko_msg_gatt_read_descriptor_value_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_write_descriptor_value_cmd_t
{
    uint8               connection;
    uint16              descriptor;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_write_descriptor_value_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_find_included_services_cmd_t
{
    uint8               connection;
    uint32              service;
});
PACKSTRUCT( struct gecko_msg_gatt_find_included_services_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_read_multiple_characteristic_values_cmd_t
{
    uint8               connection;
    uint8array          characteristic_list;
});
PACKSTRUCT( struct gecko_msg_gatt_read_multiple_characteristic_values_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_read_characteristic_value_from_offset_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint16              offset;
    uint16              maxlen;
});
PACKSTRUCT( struct gecko_msg_gatt_read_characteristic_value_from_offset_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_prepare_characteristic_value_reliable_write_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_prepare_characteristic_value_reliable_write_rsp_t
{
    uint16              result;
    uint16              sent_len;
});
PACKSTRUCT( struct gecko_msg_gatt_mtu_exchanged_evt_t
{
    uint8               connection;
    uint16              mtu;
});
PACKSTRUCT( struct gecko_msg_gatt_service_evt_t
{
    uint8               connection;
    uint32              service;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_gatt_characteristic_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               properties;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_gatt_descriptor_evt_t
{
    uint8               connection;
    uint16              descriptor;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_gatt_characteristic_value_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               att_opcode;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_descriptor_value_evt_t
{
    uint8               connection;
    uint16              descriptor;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_procedure_completed_evt_t
{
    uint8               connection;
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_server_read_attribute_value_cmd_t
{
    uint16              attribute;
    uint16              offset;
});
PACKSTRUCT( struct gecko_msg_gatt_server_read_attribute_value_rsp_t
{
    uint16              result;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_server_read_attribute_type_cmd_t
{
    uint16              attribute;
});
PACKSTRUCT( struct gecko_msg_gatt_server_read_attribute_type_rsp_t
{
    uint16              result;
    uint8array          type;
});
PACKSTRUCT( struct gecko_msg_gatt_server_write_attribute_value_cmd_t
{
    uint16              attribute;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_server_write_attribute_value_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_server_send_user_read_response_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               att_errorcode;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_server_send_user_read_response_rsp_t
{
    uint16              result;
    uint16              sent_len;
});
PACKSTRUCT( struct gecko_msg_gatt_server_send_user_write_response_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               att_errorcode;
});
PACKSTRUCT( struct gecko_msg_gatt_server_send_user_write_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_server_send_characteristic_notification_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t
{
    uint16              result;
    uint16              sent_len;
});
PACKSTRUCT( struct gecko_msg_gatt_server_find_attribute_cmd_t
{
    uint16              start;
    uint8array          type;
});
PACKSTRUCT( struct gecko_msg_gatt_server_find_attribute_rsp_t
{
    uint16              result;
    uint16              attribute;
});
PACKSTRUCT( struct gecko_msg_gatt_server_set_capabilities_cmd_t
{
    uint32              caps;
    uint32              reserved;
});
PACKSTRUCT( struct gecko_msg_gatt_server_set_capabilities_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_gatt_server_set_max_mtu_cmd_t
{
    uint16              max_mtu;
});
PACKSTRUCT( struct gecko_msg_gatt_server_set_max_mtu_rsp_t
{
    uint16              result;
    uint16              max_mtu;
});
PACKSTRUCT( struct gecko_msg_gatt_server_attribute_value_evt_t
{
    uint8               connection;
    uint16              attribute;
    uint8               att_opcode;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_server_user_read_request_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               att_opcode;
    uint16              offset;
});
PACKSTRUCT( struct gecko_msg_gatt_server_user_write_request_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               att_opcode;
    uint16              offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_gatt_server_characteristic_status_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               status_flags;
    uint16              client_config_flags;
});
PACKSTRUCT( struct gecko_msg_gatt_server_execute_write_completed_evt_t
{
    uint8               connection;
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_hardware_set_soft_timer_cmd_t
{
    uint32              time;
    uint8               handle;
    uint8               single_shot;
});
PACKSTRUCT( struct gecko_msg_hardware_set_soft_timer_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_hardware_get_time_rsp_t
{
    uint32              seconds;
    uint16              ticks;
});
PACKSTRUCT( struct gecko_msg_hardware_set_lazy_soft_timer_cmd_t
{
    uint32              time;
    uint32              slack;
    uint8               handle;
    uint8               single_shot;
});
PACKSTRUCT( struct gecko_msg_hardware_set_lazy_soft_timer_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_hardware_soft_timer_evt_t
{
    uint8               handle;
});
PACKSTRUCT( struct gecko_msg_flash_ps_erase_all_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_flash_ps_save_cmd_t
{
    uint16              key;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_flash_ps_save_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_flash_ps_load_cmd_t
{
    uint16              key;
});
PACKSTRUCT( struct gecko_msg_flash_ps_load_rsp_t
{
    uint16              result;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_flash_ps_erase_cmd_t
{
    uint16              key;
});
PACKSTRUCT( struct gecko_msg_flash_ps_erase_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_test_dtm_tx_cmd_t
{
    uint8               packet_type;
    uint8               length;
    uint8               channel;
    uint8               phy;
});
PACKSTRUCT( struct gecko_msg_test_dtm_tx_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_test_dtm_rx_cmd_t
{
    uint8               channel;
    uint8               phy;
});
PACKSTRUCT( struct gecko_msg_test_dtm_rx_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_test_dtm_end_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_test_dtm_completed_evt_t
{
    uint16              result;
    uint16              number_of_packets;
});
PACKSTRUCT( struct gecko_msg_sm_set_bondable_mode_cmd_t
{
    uint8               bondable;
});
PACKSTRUCT( struct gecko_msg_sm_set_bondable_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_configure_cmd_t
{
    uint8               flags;
    uint8               io_capabilities;
});
PACKSTRUCT( struct gecko_msg_sm_configure_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_store_bonding_configuration_cmd_t
{
    uint8               max_bonding_count;
    uint8               policy_flags;
});
PACKSTRUCT( struct gecko_msg_sm_store_bonding_configuration_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_increase_security_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_sm_increase_security_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_delete_bonding_cmd_t
{
    uint8               bonding;
});
PACKSTRUCT( struct gecko_msg_sm_delete_bonding_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_delete_bondings_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_enter_passkey_cmd_t
{
    uint8               connection;
    int32               passkey;
});
PACKSTRUCT( struct gecko_msg_sm_enter_passkey_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_passkey_confirm_cmd_t
{
    uint8               connection;
    uint8               confirm;
});
PACKSTRUCT( struct gecko_msg_sm_passkey_confirm_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_set_oob_data_cmd_t
{
    uint8array          oob_data;
});
PACKSTRUCT( struct gecko_msg_sm_set_oob_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_list_all_bondings_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_bonding_confirm_cmd_t
{
    uint8               connection;
    uint8               confirm;
});
PACKSTRUCT( struct gecko_msg_sm_bonding_confirm_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_set_debug_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_set_passkey_cmd_t
{
    int32               passkey;
});
PACKSTRUCT( struct gecko_msg_sm_set_passkey_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_use_sc_oob_cmd_t
{
    uint8               enable;
});
PACKSTRUCT( struct gecko_msg_sm_use_sc_oob_rsp_t
{
    uint16              result;
    uint8array          oob_data;
});
PACKSTRUCT( struct gecko_msg_sm_set_sc_remote_oob_data_cmd_t
{
    uint8array          oob_data;
});
PACKSTRUCT( struct gecko_msg_sm_set_sc_remote_oob_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_add_to_whitelist_cmd_t
{
    bd_addr             address;
    uint8               address_type;
});
PACKSTRUCT( struct gecko_msg_sm_add_to_whitelist_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_set_minimum_key_size_cmd_t
{
    uint8               minimum_key_size;
});
PACKSTRUCT( struct gecko_msg_sm_set_minimum_key_size_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_sm_passkey_display_evt_t
{
    uint8               connection;
    uint32              passkey;
});
PACKSTRUCT( struct gecko_msg_sm_passkey_request_evt_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_sm_confirm_passkey_evt_t
{
    uint8               connection;
    uint32              passkey;
});
PACKSTRUCT( struct gecko_msg_sm_bonded_evt_t
{
    uint8               connection;
    uint8               bonding;
});
PACKSTRUCT( struct gecko_msg_sm_bonding_failed_evt_t
{
    uint8               connection;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_sm_list_bonding_entry_evt_t
{
    uint8               bonding;
    bd_addr             address;
    uint8               address_type;
});
PACKSTRUCT( struct gecko_msg_sm_confirm_bonding_evt_t
{
    uint8               connection;
    int8                bonding_handle;
});
PACKSTRUCT( struct gecko_msg_homekit_configure_cmd_t
{
    uint8               i2c_address;
    uint8               support_display;
    uint8               hap_attribute_features;
    uint16              category;
    uint8               configuration_number;
    uint16              fast_advert_interval;
    uint16              fast_advert_timeout;
    uint32              flag;
    uint16              broadcast_advert_timeout;
    uint8array          model_name;
});
PACKSTRUCT( struct gecko_msg_homekit_configure_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_advertise_cmd_t
{
    uint8               enable;
    uint16              interval_min;
    uint16              interval_max;
    uint8               channel_map;
});
PACKSTRUCT( struct gecko_msg_homekit_advertise_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_delete_pairings_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_check_authcp_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_get_pairing_id_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_homekit_get_pairing_id_rsp_t
{
    uint16              result;
    uint8array          pairing_id;
});
PACKSTRUCT( struct gecko_msg_homekit_send_write_response_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               status_code;
});
PACKSTRUCT( struct gecko_msg_homekit_send_write_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_send_read_response_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               status_code;
    uint16              attribute_size;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_homekit_send_read_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_gsn_action_cmd_t
{
    uint8               action;
});
PACKSTRUCT( struct gecko_msg_homekit_gsn_action_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_event_notification_cmd_t
{
    uint8               connection;
    uint16              characteristic;
    uint8               change_originator;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_homekit_event_notification_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_broadcast_action_cmd_t
{
    uint8               action;
    uint8array          params;
});
PACKSTRUCT( struct gecko_msg_homekit_broadcast_action_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_homekit_setupcode_display_evt_t
{
    uint8               connection;
    uint8array          setupcode;
});
PACKSTRUCT( struct gecko_msg_homekit_paired_evt_t
{
    uint8               connection;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_homekit_pair_verified_evt_t
{
    uint8               connection;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_homekit_connection_opened_evt_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_homekit_connection_closed_evt_t
{
    uint8               connection;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_homekit_identify_evt_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_homekit_write_request_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint16              chr_value_size;
    uint16              authorization_size;
    uint16              value_offset;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_homekit_read_request_evt_t
{
    uint8               connection;
    uint16              characteristic;
    uint16              offset;
});
PACKSTRUCT( struct gecko_msg_homekit_disconnection_required_evt_t
{
    uint8               connection;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_homekit_pairing_removed_evt_t
{
    uint8               connection;
    uint16              remaining_pairings;
    uint8array          pairing_id;
});
PACKSTRUCT( struct gecko_msg_homekit_setuppayload_display_evt_t
{
    uint8               connection;
    uint8array          setuppayload;
});
PACKSTRUCT( struct gecko_msg_mesh_node_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_start_unprov_beaconing_cmd_t
{
    uint8               bearer;
});
PACKSTRUCT( struct gecko_msg_mesh_node_start_unprov_beaconing_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_input_oob_request_rsp_cmd_t
{
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_node_input_oob_request_rsp_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_uuid_rsp_t
{
    uint16              result;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_provisioning_data_cmd_t
{
    aes_key_128         device_key;
    aes_key_128         network_key;
    uint16              netkey_index;
    uint32              iv_index;
    uint16              address;
    uint8               kr_in_progress;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_provisioning_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_init_oob_cmd_t
{
    uint8               public_key;
    uint8               auth_methods;
    uint16              output_actions;
    uint8               output_size;
    uint16              input_actions;
    uint8               input_size;
    uint16              oob_location;
});
PACKSTRUCT( struct gecko_msg_mesh_node_init_oob_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_ivrecovery_mode_cmd_t
{
    uint8               mode;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_ivrecovery_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_ivrecovery_mode_rsp_t
{
    uint16              result;
    uint8               mode;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_adv_event_filter_cmd_t
{
    uint16              mask;
    uint8array          gap_data_type;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_adv_event_filter_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_statistics_rsp_t
{
    uint16              result;
    uint8array          statistics;
});
PACKSTRUCT( struct gecko_msg_mesh_node_clear_statistics_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_net_relay_delay_cmd_t
{
    uint8               min;
    uint8               max;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_net_relay_delay_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_net_relay_delay_rsp_t
{
    uint16              result;
    uint8               min;
    uint8               max;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_ivupdate_state_rsp_t
{
    uint16              result;
    uint32              ivindex;
    uint8               state;
});
PACKSTRUCT( struct gecko_msg_mesh_node_request_ivupdate_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_seq_remaining_cmd_t
{
    uint16              elem_index;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_seq_remaining_rsp_t
{
    uint16              result;
    uint32              count;
});
PACKSTRUCT( struct gecko_msg_mesh_node_save_replay_protection_list_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_uuid_cmd_t
{
    uuid_128            uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_node_set_uuid_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_element_address_cmd_t
{
    uint16              elem_index;
});
PACKSTRUCT( struct gecko_msg_mesh_node_get_element_address_rsp_t
{
    uint16              result;
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_node_static_oob_request_rsp_cmd_t
{
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_node_static_oob_request_rsp_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_reset_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_initialized_evt_t
{
    uint8               provisioned;
    uint16              address;
    uint32              ivi;
});
PACKSTRUCT( struct gecko_msg_mesh_node_provisioned_evt_t
{
    uint32              iv_index;
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_node_config_get_evt_t
{
    uint16              id;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_node_config_set_evt_t
{
    uint16              id;
    uint16              netkey_index;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_mesh_node_display_output_oob_evt_t
{
    uint8               output_action;
    uint8               output_size;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_node_input_oob_request_evt_t
{
    uint8               input_action;
    uint8               input_size;
});
PACKSTRUCT( struct gecko_msg_mesh_node_provisioning_started_evt_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_provisioning_failed_evt_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_node_key_added_evt_t
{
    uint8               type;
    uint16              index;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_node_model_config_changed_evt_t
{
    uint8               mesh_node_config_state;
    uint16              element_address;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_node_ivrecovery_needed_evt_t
{
    uint32              node_ivindex;
    uint32              network_ivindex;
});
PACKSTRUCT( struct gecko_msg_mesh_node_changed_ivupdate_state_evt_t
{
    uint32              ivindex;
    uint8               state;
});
PACKSTRUCT( struct gecko_msg_mesh_node_key_removed_evt_t
{
    uint8               type;
    uint16              index;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_node_key_updated_evt_t
{
    uint8               type;
    uint16              index;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_scan_unprov_beacons_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_device_cmd_t
{
    uint8               network_id;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_device_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_create_network_cmd_t
{
    uint8array          key;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_create_network_rsp_t
{
    uint16              result;
    uint8               network_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_dcd_cmd_t
{
    uint16              address;
    uint8               page;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_dcd_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_config_cmd_t
{
    uint16              address;
    uint16              id;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_config_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_config_cmd_t
{
    uint16              address;
    uint16              id;
    uint16              netkey_index;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_config_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_create_appkey_cmd_t
{
    uint16              netkey_index;
    uint8array          key;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_create_appkey_rsp_t
{
    uint16              result;
    uint16              appkey_index;
    uint8array          key;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_pkey_rsp_cmd_t
{
    uint8array          pkey;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_pkey_rsp_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_auth_rsp_cmd_t
{
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_auth_rsp_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_oob_requirements_cmd_t
{
    uint8               public_key;
    uint8               auth_methods;
    uint16              output_actions;
    uint16              input_actions;
    uint8               min_size;
    uint8               max_size;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_oob_requirements_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_key_refresh_start_cmd_t
{
    uint16              netkey_index;
    uint8               num_appkeys;
    uint8array          appkey_indices;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_key_refresh_start_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_key_refresh_blacklist_cmd_t
{
    uint16              key;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_key_refresh_blacklist_rsp_t
{
    uint16              result;
    uint8               status;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_key_refresh_blacklist_cmd_t
{
    uint16              key;
    uint8               status;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_key_refresh_blacklist_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_add_cmd_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              appkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_add_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_delete_cmd_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              appkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_delete_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_app_bind_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_app_bind_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_app_unbind_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_app_unbind_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_app_get_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_app_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_add_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_add_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              pub_address;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_gatt_device_cmd_t
{
    uint8               network_id;
    uint8               connection;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_gatt_device_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_get_cmd_t
{
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_get_rsp_t
{
    uint16              result;
    aes_key_128         device_key;
    uint16              netkey_index;
    uint16              address;
    uint8               elements;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_delete_cmd_t
{
    uuid_128            uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_delete_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_add_cmd_t
{
    uuid_128            uuid;
    aes_key_128         device_key;
    uint16              netkey_index;
    uint16              address;
    uint8               elements;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_add_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_list_devices_rsp_t
{
    uint16              result;
    uint16              count;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_add_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_add_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_delete_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_delete_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_nettx_get_cmd_t
{
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_nettx_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_nettx_set_cmd_t
{
    uint16              address;
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_nettx_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_del_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_del_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_add_va_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8array          sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_add_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_del_va_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8array          sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_del_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_set_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_set_va_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8array          sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_set_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_publication_get_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_publication_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_publication_set_cmd_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              publication_address;
    uint8               count_log;
    uint8               period_log;
    uint8               ttl;
    uint16              features;
    uint16              publication_netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_publication_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_subscription_get_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_subscription_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_subscription_set_cmd_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              subscription_source;
    uint16              subscription_destination;
    uint8               period_log;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_subscription_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_relay_get_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_relay_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_relay_set_cmd_t
{
    uint16              address;
    uint16              netkey_index;
    uint8               relay;
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_relay_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_reset_node_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_reset_node_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_get_cmd_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_get_cmd_t
{
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_clear_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_clear_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_get_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_va_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8array          pub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_cred_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              pub_address;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8               credentials;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_cred_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_va_cred_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8               credentials;
    uint8array          pub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_set_va_cred_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_get_cmd_t
{
    uint16              address;
    uint16              elem_address;
    uint16              netkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_friend_timeout_get_cmd_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              lpn_address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_friend_timeout_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_default_configuration_timeout_rsp_t
{
    uint16              result;
    uint32              timeout;
    uint32              lpn_timeout;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_default_configuration_timeout_cmd_t
{
    uint32              timeout;
    uint32              lpn_timeout;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_default_configuration_timeout_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_device_with_address_cmd_t
{
    uint8               network_id;
    uint16              address;
    uint8               elements;
    uint8               attention_timer;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_device_with_address_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_gatt_device_with_address_cmd_t
{
    uint8               network_id;
    uint8               connection;
    uint16              address;
    uint8               elements;
    uint8               attention_timer;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provision_gatt_device_with_address_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_initialize_network_cmd_t
{
    uint16              address;
    uint32              ivi;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_initialize_network_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_key_refresh_appkey_blacklist_cmd_t
{
    uint16              netkey_index;
    uint16              appkey_index;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_get_key_refresh_appkey_blacklist_rsp_t
{
    uint16              result;
    uint8               status;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_key_refresh_appkey_blacklist_cmd_t
{
    uint16              netkey_index;
    uint16              appkey_index;
    uint8               status;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_set_key_refresh_appkey_blacklist_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_stop_scan_unprov_beacons_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_initialized_evt_t
{
    uint8               networks;
    uint16              address;
    uint32              ivi;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_provisioning_failed_evt_t
{
    uint8               reason;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_device_provisioned_evt_t
{
    uint16              address;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_unprov_beacon_evt_t
{
    uint16              oob_capabilities;
    uint32              uri_hash;
    uint8               bearer;
    bd_addr             address;
    uint8               address_type;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_dcd_status_evt_t
{
    uint16              result;
    uint16              address;
    uint16              cid;
    uint16              pid;
    uint16              vid;
    uint16              crpl;
    uint16              features;
    uint8               elements;
    uint8               models;
    uint8array          element_data;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_config_status_evt_t
{
    uint16              address;
    uint16              id;
    uint8               status;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_pkey_request_evt_t
{
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_auth_request_evt_t
{
    uint8               output;
    uint8               output_action;
    uint8               output_size;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_oob_display_input_evt_t
{
    uint8               input_action;
    uint8               input_size;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_ddb_list_evt_t
{
    uuid_128            uuid;
    uint16              address;
    uint8               elements;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_publication_status_evt_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              publication_address;
    uint8               count_log;
    uint8               period_log;
    uint8               ttl;
    uint16              features;
    uint16              publication_netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_heartbeat_subscription_status_evt_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              subscription_source;
    uint16              subscription_destination;
    uint8               period_log;
    uint8               count_log;
    uint8               min_hops;
    uint8               max_hops;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_relay_status_evt_t
{
    uint16              address;
    uint16              netkey_index;
    uint8               value;
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_uri_evt_t
{
    uint32              hash;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_node_reset_evt_t
{
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_list_evt_t
{
    uint16              address;
    uint16              netkey_index;
    uint16              appkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_appkey_list_end_evt_t
{
    uint16              result;
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_list_evt_t
{
    uint16              address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_network_list_end_evt_t
{
    uint16              result;
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_pub_status_evt_t
{
    uint16              result;
    uint16              elem_address;
    uint16              vendor_id;
    uint16              model_id;
    uint16              appkey_index;
    uint16              pub_address;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8               credentials;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_key_refresh_phase_update_evt_t
{
    uint16              key;
    uint8               phase;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_key_refresh_node_update_evt_t
{
    uint16              key;
    uint8               phase;
    uint8array          uuid;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_key_refresh_complete_evt_t
{
    uint16              key;
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_addr_evt_t
{
    uint16              elem_address;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_addr;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_model_sub_addr_end_evt_t
{
    uint16              result;
    uint16              elem_address;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_prov_friend_timeout_status_evt_t
{
    uint16              address;
    uint16              netkey_index;
    uint32              timeout;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_connect_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_connect_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_disconnect_cmd_t
{
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_disconnect_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_set_filter_type_cmd_t
{
    uint32              handle;
    uint8               type;
    uint16              key;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_set_filter_type_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_allow_cmd_t
{
    uint32              handle;
    uint16              address;
    uint16              key;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_allow_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_deny_cmd_t
{
    uint32              handle;
    uint16              address;
    uint16              key;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_deny_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_connected_evt_t
{
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_disconnected_evt_t
{
    uint32              handle;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_mesh_proxy_filter_status_evt_t
{
    uint32              handle;
    uint8               type;
    uint16              count;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_send_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              destination_address;
    int8                va_index;
    uint16              appkey_index;
    uint8               nonrelayed;
    uint8               opcode;
    uint8               final;
    uint8array          payload;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_send_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_set_publication_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8               opcode;
    uint8               final;
    uint8array          payload;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_set_publication_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_clear_publication_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_clear_publication_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_publish_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_publish_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_init_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8               publish;
    uint8array          opcodes;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_deinit_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_deinit_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_vendor_model_receive_evt_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              source_address;
    uint16              destination_address;
    int8                va_index;
    uint16              appkey_index;
    uint8               nonrelayed;
    uint8               opcode;
    uint8               final;
    uint8array          payload;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_get_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint16              vendor_id;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_clear_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint16              vendor_id;
    uint8               reliable;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_clear_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_test_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               test_id;
    uint16              vendor_id;
    uint8               reliable;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_test_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_get_period_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_get_period_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_set_period_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               period;
    uint8               reliable;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_set_period_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_get_attention_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_get_attention_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_set_attention_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               attention;
    uint8               reliable;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_set_attention_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_server_status_evt_t
{
    uint16              result;
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint8               current;
    uint8               test_id;
    uint16              vendor_id;
    uint8array          faults;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_server_status_period_evt_t
{
    uint16              result;
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint8               period;
});
PACKSTRUCT( struct gecko_msg_mesh_health_client_server_status_attention_evt_t
{
    uint16              result;
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint8               attention;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_set_fault_cmd_t
{
    uint16              elem_index;
    uint8               id;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_set_fault_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_clear_fault_cmd_t
{
    uint16              elem_index;
    uint8               id;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_clear_fault_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_test_response_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint16              vendor_id;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_test_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_attention_evt_t
{
    uint16              elem_index;
    uint8               timer;
});
PACKSTRUCT( struct gecko_msg_mesh_health_server_test_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               test_id;
    uint16              vendor_id;
    uint8               response_required;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_get_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               type;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_set_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               tid;
    uint32              transition;
    uint16              delay;
    uint16              flags;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_set_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_publish_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint8               tid;
    uint32              transition;
    uint16              delay;
    uint16              flags;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_publish_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_get_params_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_get_params_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_client_server_status_evt_t
{
    uint16              model_id;
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint32              remaining;
    uint16              flags;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_response_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint32              remaining;
    uint16              flags;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_update_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint32              remaining;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_update_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_publish_cmd_t
{
    uint16              model_id;
    uint16              elem_index;
    uint8               type;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_publish_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_client_request_evt_t
{
    uint16              model_id;
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint32              transition;
    uint16              delay;
    uint16              flags;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_mesh_generic_server_state_changed_evt_t
{
    uint16              model_id;
    uint16              elem_index;
    uint32              remaining;
    uint8               type;
    uint8array          parameters;
});
PACKSTRUCT( struct gecko_msg_coex_set_options_cmd_t
{
    uint32              mask;
    uint32              options;
});
PACKSTRUCT( struct gecko_msg_coex_set_options_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_coex_get_counters_cmd_t
{
    uint8               reset;
});
PACKSTRUCT( struct gecko_msg_coex_get_counters_rsp_t
{
    uint16              result;
    uint8array          counters;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_nettx_rsp_t
{
    uint16              result;
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_nettx_cmd_t
{
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_nettx_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_relay_rsp_t
{
    uint16              result;
    uint8               enabled;
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_relay_cmd_t
{
    uint8               enabled;
    uint8               count;
    uint8               interval;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_relay_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_adv_scan_params_cmd_t
{
    uint16              adv_interval_min;
    uint16              adv_interval_max;
    uint8               adv_repeat_packets;
    uint8               adv_use_random_address;
    uint8               adv_channel_map;
    uint16              scan_interval;
    uint16              scan_window;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_adv_scan_params_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_ivupdate_test_mode_cmd_t
{
    uint8               mode;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_ivupdate_test_mode_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_ivupdate_test_mode_rsp_t
{
    uint16              result;
    uint8               mode;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_segment_send_delay_cmd_t
{
    uint8               delay;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_segment_send_delay_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_ivupdate_state_cmd_t
{
    uint8               state;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_ivupdate_state_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_send_beacons_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_bind_local_model_app_cmd_t
{
    uint16              elem_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_test_bind_local_model_app_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_unbind_local_model_app_cmd_t
{
    uint16              elem_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_test_unbind_local_model_app_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_add_local_model_sub_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_test_add_local_model_sub_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_del_local_model_sub_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_test_del_local_model_sub_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_add_local_model_sub_va_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8array          sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_test_add_local_model_sub_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_del_local_model_sub_va_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8array          sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_test_del_local_model_sub_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_model_sub_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_model_sub_rsp_t
{
    uint16              result;
    uint8array          addresses;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_model_pub_cmd_t
{
    uint16              elem_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              pub_address;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8               credentials;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_model_pub_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_model_pub_va_cmd_t
{
    uint16              elem_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8               credentials;
    uint8array          pub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_model_pub_va_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_model_pub_cmd_t
{
    uint16              elem_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_model_pub_rsp_t
{
    uint16              result;
    uint16              appkey_index;
    uint16              pub_address;
    uint8               ttl;
    uint8               period;
    uint8               retrans;
    uint8               credentials;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_heartbeat_subscription_cmd_t
{
    uint16              subscription_source;
    uint16              subscription_destination;
    uint8               period_log;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_heartbeat_subscription_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_heartbeat_subscription_rsp_t
{
    uint16              result;
    uint16              count;
    uint8               hop_min;
    uint8               hop_max;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_heartbeat_publication_rsp_t
{
    uint16              result;
    uint16              publication_address;
    uint8               count;
    uint8               period_log;
    uint8               ttl;
    uint16              features;
    uint16              publication_netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_heartbeat_publication_cmd_t
{
    uint16              publication_address;
    uint8               count_log;
    uint8               period_log;
    uint8               ttl;
    uint16              features;
    uint16              publication_netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_heartbeat_publication_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_config_cmd_t
{
    uint16              id;
    uint16              netkey_index;
    uint8array          value;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_local_config_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_config_cmd_t
{
    uint16              id;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_local_config_rsp_t
{
    uint16              result;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_test_add_local_key_cmd_t
{
    uint8               key_type;
    aes_key_128         key;
    uint16              key_index;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_add_local_key_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_del_local_key_cmd_t
{
    uint8               key_type;
    uint16              key_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_del_local_key_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_update_local_key_cmd_t
{
    uint8               key_type;
    aes_key_128         key;
    uint16              key_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_update_local_key_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_sar_config_cmd_t
{
    uint32              incomplete_timer_ms;
    uint32              pending_ack_base_ms;
    uint32              pending_ack_mul_ms;
    uint32              wait_for_ack_base_ms;
    uint32              wait_for_ack_mul_ms;
    uint8               max_send_rounds;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_sar_config_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_element_seqnum_cmd_t
{
    uint16              elem_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_element_seqnum_rsp_t
{
    uint16              result;
    uint32              seqnum;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_adv_bearer_state_cmd_t
{
    uint8               state;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_adv_bearer_state_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_key_count_cmd_t
{
    uint8               type;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_key_count_rsp_t
{
    uint16              result;
    uint32              count;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_key_cmd_t
{
    uint8               type;
    uint32              index;
    uint8               current;
});
PACKSTRUCT( struct gecko_msg_mesh_test_get_key_rsp_t
{
    uint16              result;
    uint16              id;
    uint16              network;
    aes_key_128         key;
});
PACKSTRUCT( struct gecko_msg_mesh_test_prov_get_device_key_cmd_t
{
    uint16              address;
});
PACKSTRUCT( struct gecko_msg_mesh_test_prov_get_device_key_rsp_t
{
    uint16              result;
    aes_key_128         device_key;
});
PACKSTRUCT( struct gecko_msg_mesh_test_prov_prepare_key_refresh_cmd_t
{
    aes_key_128         net_key;
    uint8array          app_keys;
});
PACKSTRUCT( struct gecko_msg_mesh_test_prov_prepare_key_refresh_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_cancel_segmented_tx_cmd_t
{
    uint16              src_addr;
    uint16              dst_addr;
});
PACKSTRUCT( struct gecko_msg_mesh_test_cancel_segmented_tx_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_iv_index_cmd_t
{
    uint32              iv_index;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_iv_index_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_element_seqnum_cmd_t
{
    uint16              elem_index;
    uint32              seqnum;
});
PACKSTRUCT( struct gecko_msg_mesh_test_set_element_seqnum_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_test_local_heartbeat_subscription_complete_evt_t
{
    uint16              count;
    uint8               hop_min;
    uint8               hop_max;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_deinit_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_configure_cmd_t
{
    uint8               queue_length;
    uint32              poll_timeout;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_configure_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_establish_friendship_cmd_t
{
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_establish_friendship_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_poll_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_terminate_friendship_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_config_cmd_t
{
    uint8               setting_id;
    uint32              value;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_config_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_friendship_established_evt_t
{
    uint16              friend_address;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_friendship_failed_evt_t
{
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_mesh_lpn_friendship_terminated_evt_t
{
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_mesh_friend_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_friend_deinit_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_friend_friendship_established_evt_t
{
    uint16              lpn_address;
});
PACKSTRUCT( struct gecko_msg_mesh_friend_friendship_terminated_evt_t
{
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_cancel_request_cmd_t
{
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_cancel_request_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_request_status_cmd_t
{
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_request_status_rsp_t
{
    uint16              result;
    uint16              server_address;
    uint16              opcode;
    uint32              age;
    uint32              remaining;
    uint8               friend_acked;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_default_timeout_rsp_t
{
    uint16              result;
    uint32              timeout_ms;
    uint32              lpn_timeout_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_default_timeout_cmd_t
{
    uint32              timeout_ms;
    uint32              lpn_timeout_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_default_timeout_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_netkey_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_netkey_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_netkey_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_netkey_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_netkeys_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_netkeys_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_appkey_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              appkey_index;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_appkey_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_appkey_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              appkey_index;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_appkey_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_appkeys_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_appkeys_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_bind_model_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_bind_model_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_unbind_model_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              appkey_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_unbind_model_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_bindings_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_bindings_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_model_pub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_model_pub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_pub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              address;
    uint16              appkey_index;
    uint8               credentials;
    uint8               ttl;
    uint32              period_ms;
    uint8               retransmit_count;
    uint16              retransmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_pub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_pub_va_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uuid_128            address;
    uint16              appkey_index;
    uint8               credentials;
    uint8               ttl;
    uint32              period_ms;
    uint8               retransmit_count;
    uint16              retransmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_pub_va_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_model_sub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_model_sub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_model_sub_va_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uuid_128            sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_add_model_sub_va_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_model_sub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_model_sub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_model_sub_va_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uuid_128            sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_remove_model_sub_va_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_sub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uint16              sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_sub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_sub_va_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
    uuid_128            sub_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_model_sub_va_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_clear_model_sub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_clear_model_sub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_subs_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               element_index;
    uint16              vendor_id;
    uint16              model_id;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_list_subs_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_heartbeat_pub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_heartbeat_pub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_heartbeat_pub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              destination_address;
    uint16              netkey_index;
    uint8               count_log;
    uint8               period_log;
    uint8               ttl;
    uint16              features;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_heartbeat_pub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_heartbeat_sub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_heartbeat_sub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_heartbeat_sub_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              source_address;
    uint16              destination_address;
    uint8               period_log;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_heartbeat_sub_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_beacon_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_beacon_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_beacon_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_beacon_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_default_ttl_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_default_ttl_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_default_ttl_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_default_ttl_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_gatt_proxy_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_gatt_proxy_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_gatt_proxy_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_gatt_proxy_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_relay_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_relay_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_relay_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               value;
    uint8               retransmit_count;
    uint16              retransmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_relay_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_network_transmit_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_network_transmit_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_network_transmit_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               transmit_count;
    uint16              transmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_network_transmit_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_identity_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              netkey_index;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_identity_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_identity_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              netkey_index;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_identity_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_friend_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_friend_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_friend_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_set_friend_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_lpn_polltimeout_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint16              lpn_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_lpn_polltimeout_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_dcd_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
    uint8               page;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_get_dcd_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_reset_node_cmd_t
{
    uint16              enc_netkey_index;
    uint16              server_address;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_reset_node_rsp_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_request_modified_evt_t
{
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_netkey_status_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_netkey_list_evt_t
{
    uint32              handle;
    uint8array          netkey_indices;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_netkey_list_end_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_appkey_status_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_appkey_list_evt_t
{
    uint32              handle;
    uint8array          appkey_indices;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_appkey_list_end_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_binding_status_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_bindings_list_evt_t
{
    uint32              handle;
    uint8array          appkey_indices;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_bindings_list_end_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_model_pub_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint16              address;
    uint16              appkey_index;
    uint8               credentials;
    uint8               ttl;
    uint32              period_ms;
    uint8               retransmit_count;
    uint16              retransmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_model_sub_status_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_subs_list_evt_t
{
    uint32              handle;
    uint8array          addresses;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_subs_list_end_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_heartbeat_pub_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint16              destination_address;
    uint16              netkey_index;
    uint8               count_log;
    uint8               period_log;
    uint8               ttl;
    uint16              features;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_heartbeat_sub_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint16              source_address;
    uint16              destination_address;
    uint8               period_log;
    uint8               count_log;
    uint8               min_hops;
    uint8               max_hops;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_beacon_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_default_ttl_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_gatt_proxy_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_relay_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               relay;
    uint8               retransmit_count;
    uint16              retransmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_network_transmit_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               transmit_count;
    uint16              transmit_interval_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_identity_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_friend_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint8               value;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_lpn_polltimeout_status_evt_t
{
    uint16              result;
    uint32              handle;
    uint32              poll_timeout_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_dcd_data_evt_t
{
    uint32              handle;
    uint8               page;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_dcd_data_end_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_mesh_config_client_reset_status_evt_t
{
    uint16              result;
    uint32              handle;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_connection_request_cmd_t
{
    uint8               connection;
    uint16              le_psm;
    uint16              mtu;
    uint16              mps;
    uint16              initial_credit;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_connection_request_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_connection_response_cmd_t
{
    uint8               connection;
    uint16              cid;
    uint16              mtu;
    uint16              mps;
    uint16              initial_credit;
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_connection_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_le_flow_control_credit_cmd_t
{
    uint8               connection;
    uint16              cid;
    uint16              credits;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_le_flow_control_credit_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_disconnection_request_cmd_t
{
    uint8               connection;
    uint16              cid;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_disconnection_request_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_data_cmd_t
{
    uint8               connection;
    uint16              cid;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_send_data_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_connection_request_evt_t
{
    uint8               connection;
    uint16              le_psm;
    uint16              source_cid;
    uint16              mtu;
    uint16              mps;
    uint16              initial_credit;
    uint8               flags;
    uint8               encryption_key_size;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_connection_response_evt_t
{
    uint8               connection;
    uint16              destination_cid;
    uint16              mtu;
    uint16              mps;
    uint16              initial_credit;
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_le_flow_control_credit_evt_t
{
    uint8               connection;
    uint16              cid;
    uint16              credits;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_channel_disconnected_evt_t
{
    uint8               connection;
    uint16              cid;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_l2cap_coc_data_evt_t
{
    uint8               connection;
    uint16              cid;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_l2cap_command_rejected_evt_t
{
    uint8               connection;
    uint8               code;
    uint16              reason;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_enable_cte_response_cmd_t
{
    uint8               connection;
    uint8               cte_types;
    uint8array          switching_pattern;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_enable_cte_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_disable_cte_response_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_disable_cte_response_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_start_connectionless_cte_cmd_t
{
    uint8               adv;
    uint8               cte_length;
    uint8               cte_type;
    uint8               cte_count;
    uint8array          switching_pattern;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_start_connectionless_cte_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_stop_connectionless_cte_cmd_t
{
    uint8               adv;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_stop_connectionless_cte_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_set_dtm_parameters_cmd_t
{
    uint8               cte_length;
    uint8               cte_type;
    uint8array          switching_pattern;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_set_dtm_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_transmitter_clear_dtm_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_configure_cmd_t
{
    uint8               flags;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_configure_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_start_iq_sampling_cmd_t
{
    uint8               connection;
    uint16              interval;
    uint8               cte_length;
    uint8               cte_type;
    uint8               slot_durations;
    uint8array          switching_pattern;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_start_iq_sampling_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_stop_iq_sampling_cmd_t
{
    uint8               connection;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_stop_iq_sampling_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_start_connectionless_iq_sampling_cmd_t
{
    uint8               sync;
    uint8               slot_durations;
    uint8               cte_count;
    uint8array          switching_pattern;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_start_connectionless_iq_sampling_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_stop_connectionless_iq_sampling_cmd_t
{
    uint8               sync;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_stop_connectionless_iq_sampling_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_set_dtm_parameters_cmd_t
{
    uint8               cte_length;
    uint8               cte_type;
    uint8               slot_durations;
    uint8array          switching_pattern;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_set_dtm_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_clear_dtm_parameters_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_cte_receiver_iq_report_evt_t
{
    uint16              status;
    uint8               packet_type;
    uint8               handle;
    int8                rssi;
    uint8               channel;
    uint8array          samples;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_init_cmd_t
{
    uint16              elem_index;
    uint8array          descriptors;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_deinit_cmd_t
{
    uint16              elem_index;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_deinit_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_descriptor_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint8array          descriptors;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_descriptor_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint8array          sensor_data;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_column_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          sensor_data;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_column_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_series_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          sensor_data;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_send_series_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_get_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_get_column_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          column_ids;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_get_series_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          column_ids;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_server_publish_evt_t
{
    uint16              elem_index;
    uint32              period_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_send_cadence_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          params;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_send_cadence_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_send_settings_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          setting_ids;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_send_settings_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_send_setting_status_cmd_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint16              setting_id;
    uint8array          raw_value;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_send_setting_status_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_get_cadence_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_set_cadence_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8               period_divisor;
    uint8               trigger_type;
    uint8array          params;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_get_settings_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_get_setting_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint16              setting_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_set_setting_request_evt_t
{
    uint16              elem_index;
    uint16              client_address;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint16              setting_id;
    uint8array          raw_value;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_setup_server_publish_evt_t
{
    uint16              elem_index;
    uint32              period_ms;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_init_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_deinit_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_descriptor_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_descriptor_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_column_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          column_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_column_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_series_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          column_ids;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_series_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_cadence_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_cadence_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_set_cadence_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          params;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_set_cadence_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_settings_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_settings_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_setting_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint16              setting_id;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_get_setting_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_set_setting_cmd_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint16              setting_id;
    uint8array          raw_value;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_set_setting_rsp_t
{
    uint16              result;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_descriptor_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint8array          descriptors;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_cadence_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          params;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_settings_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          setting_ids;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_setting_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint16              setting_id;
    uint8array          raw_value;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint8array          sensor_data;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_column_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          sensor_data;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_series_status_evt_t
{
    uint16              elem_index;
    uint16              server_address;
    uint16              client_address;
    uint16              appkey_index;
    uint8               flags;
    uint16              property_id;
    uint8array          sensor_data;
});
PACKSTRUCT( struct gecko_msg_mesh_sensor_client_publish_evt_t
{
    uint16              elem_index;
    uint32              period_ms;
});
PACKSTRUCT( struct gecko_msg_user_message_to_target_cmd_t
{
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_user_message_to_target_rsp_t
{
    uint16              result;
    uint8array          data;
});
PACKSTRUCT( struct gecko_msg_user_message_to_host_evt_t
{
    uint8array          data;
});


PACKSTRUCT( struct gecko_cmd_packet
{
    uint32   header;

union{
    uint8 handle;
    struct gecko_msg_dfu_reset_cmd_t                             cmd_dfu_reset;
    struct gecko_msg_dfu_flash_set_address_cmd_t                 cmd_dfu_flash_set_address;
    struct gecko_msg_dfu_flash_set_address_rsp_t                 rsp_dfu_flash_set_address;
    struct gecko_msg_dfu_flash_upload_cmd_t                      cmd_dfu_flash_upload;
    struct gecko_msg_dfu_flash_upload_rsp_t                      rsp_dfu_flash_upload;
    struct gecko_msg_dfu_flash_upload_finish_rsp_t               rsp_dfu_flash_upload_finish;
    struct gecko_msg_dfu_boot_evt_t                              evt_dfu_boot;
    struct gecko_msg_dfu_boot_failure_evt_t                      evt_dfu_boot_failure;
    struct gecko_msg_system_hello_rsp_t                          rsp_system_hello;
    struct gecko_msg_system_reset_cmd_t                          cmd_system_reset;
    struct gecko_msg_system_get_bt_address_rsp_t                 rsp_system_get_bt_address;
    struct gecko_msg_system_set_bt_address_cmd_t                 cmd_system_set_bt_address;
    struct gecko_msg_system_set_bt_address_rsp_t                 rsp_system_set_bt_address;
    struct gecko_msg_system_set_tx_power_cmd_t                   cmd_system_set_tx_power;
    struct gecko_msg_system_set_tx_power_rsp_t                   rsp_system_set_tx_power;
    struct gecko_msg_system_get_random_data_cmd_t                cmd_system_get_random_data;
    struct gecko_msg_system_get_random_data_rsp_t                rsp_system_get_random_data;
    struct gecko_msg_system_halt_cmd_t                           cmd_system_halt;
    struct gecko_msg_system_halt_rsp_t                           rsp_system_halt;
    struct gecko_msg_system_set_device_name_cmd_t                cmd_system_set_device_name;
    struct gecko_msg_system_set_device_name_rsp_t                rsp_system_set_device_name;
    struct gecko_msg_system_linklayer_configure_cmd_t            cmd_system_linklayer_configure;
    struct gecko_msg_system_linklayer_configure_rsp_t            rsp_system_linklayer_configure;
    struct gecko_msg_system_get_counters_cmd_t                   cmd_system_get_counters;
    struct gecko_msg_system_get_counters_rsp_t                   rsp_system_get_counters;
    struct gecko_msg_system_data_buffer_write_cmd_t              cmd_system_data_buffer_write;
    struct gecko_msg_system_data_buffer_write_rsp_t              rsp_system_data_buffer_write;
    struct gecko_msg_system_set_identity_address_cmd_t           cmd_system_set_identity_address;
    struct gecko_msg_system_set_identity_address_rsp_t           rsp_system_set_identity_address;
    struct gecko_msg_system_data_buffer_clear_rsp_t              rsp_system_data_buffer_clear;
    struct gecko_msg_system_boot_evt_t                           evt_system_boot;
    struct gecko_msg_system_external_signal_evt_t                evt_system_external_signal;
    struct gecko_msg_system_hardware_error_evt_t                 evt_system_hardware_error;
    struct gecko_msg_system_error_evt_t                          evt_system_error;
    struct gecko_msg_le_gap_open_cmd_t                           cmd_le_gap_open;
    struct gecko_msg_le_gap_open_rsp_t                           rsp_le_gap_open;
    struct gecko_msg_le_gap_set_mode_cmd_t                       cmd_le_gap_set_mode;
    struct gecko_msg_le_gap_set_mode_rsp_t                       rsp_le_gap_set_mode;
    struct gecko_msg_le_gap_discover_cmd_t                       cmd_le_gap_discover;
    struct gecko_msg_le_gap_discover_rsp_t                       rsp_le_gap_discover;
    struct gecko_msg_le_gap_end_procedure_rsp_t                  rsp_le_gap_end_procedure;
    struct gecko_msg_le_gap_set_adv_parameters_cmd_t             cmd_le_gap_set_adv_parameters;
    struct gecko_msg_le_gap_set_adv_parameters_rsp_t             rsp_le_gap_set_adv_parameters;
    struct gecko_msg_le_gap_set_conn_parameters_cmd_t            cmd_le_gap_set_conn_parameters;
    struct gecko_msg_le_gap_set_conn_parameters_rsp_t            rsp_le_gap_set_conn_parameters;
    struct gecko_msg_le_gap_set_scan_parameters_cmd_t            cmd_le_gap_set_scan_parameters;
    struct gecko_msg_le_gap_set_scan_parameters_rsp_t            rsp_le_gap_set_scan_parameters;
    struct gecko_msg_le_gap_set_adv_data_cmd_t                   cmd_le_gap_set_adv_data;
    struct gecko_msg_le_gap_set_adv_data_rsp_t                   rsp_le_gap_set_adv_data;
    struct gecko_msg_le_gap_set_adv_timeout_cmd_t                cmd_le_gap_set_adv_timeout;
    struct gecko_msg_le_gap_set_adv_timeout_rsp_t                rsp_le_gap_set_adv_timeout;
    struct gecko_msg_le_gap_bt5_set_mode_cmd_t                   cmd_le_gap_bt5_set_mode;
    struct gecko_msg_le_gap_bt5_set_mode_rsp_t                   rsp_le_gap_bt5_set_mode;
    struct gecko_msg_le_gap_bt5_set_adv_parameters_cmd_t         cmd_le_gap_bt5_set_adv_parameters;
    struct gecko_msg_le_gap_bt5_set_adv_parameters_rsp_t         rsp_le_gap_bt5_set_adv_parameters;
    struct gecko_msg_le_gap_bt5_set_adv_data_cmd_t               cmd_le_gap_bt5_set_adv_data;
    struct gecko_msg_le_gap_bt5_set_adv_data_rsp_t               rsp_le_gap_bt5_set_adv_data;
    struct gecko_msg_le_gap_set_privacy_mode_cmd_t               cmd_le_gap_set_privacy_mode;
    struct gecko_msg_le_gap_set_privacy_mode_rsp_t               rsp_le_gap_set_privacy_mode;
    struct gecko_msg_le_gap_set_advertise_timing_cmd_t           cmd_le_gap_set_advertise_timing;
    struct gecko_msg_le_gap_set_advertise_timing_rsp_t           rsp_le_gap_set_advertise_timing;
    struct gecko_msg_le_gap_set_advertise_channel_map_cmd_t      cmd_le_gap_set_advertise_channel_map;
    struct gecko_msg_le_gap_set_advertise_channel_map_rsp_t      rsp_le_gap_set_advertise_channel_map;
    struct gecko_msg_le_gap_set_advertise_report_scan_request_cmd_t cmd_le_gap_set_advertise_report_scan_request;
    struct gecko_msg_le_gap_set_advertise_report_scan_request_rsp_t rsp_le_gap_set_advertise_report_scan_request;
    struct gecko_msg_le_gap_set_advertise_phy_cmd_t              cmd_le_gap_set_advertise_phy;
    struct gecko_msg_le_gap_set_advertise_phy_rsp_t              rsp_le_gap_set_advertise_phy;
    struct gecko_msg_le_gap_set_advertise_configuration_cmd_t    cmd_le_gap_set_advertise_configuration;
    struct gecko_msg_le_gap_set_advertise_configuration_rsp_t    rsp_le_gap_set_advertise_configuration;
    struct gecko_msg_le_gap_clear_advertise_configuration_cmd_t  cmd_le_gap_clear_advertise_configuration;
    struct gecko_msg_le_gap_clear_advertise_configuration_rsp_t  rsp_le_gap_clear_advertise_configuration;
    struct gecko_msg_le_gap_start_advertising_cmd_t              cmd_le_gap_start_advertising;
    struct gecko_msg_le_gap_start_advertising_rsp_t              rsp_le_gap_start_advertising;
    struct gecko_msg_le_gap_stop_advertising_cmd_t               cmd_le_gap_stop_advertising;
    struct gecko_msg_le_gap_stop_advertising_rsp_t               rsp_le_gap_stop_advertising;
    struct gecko_msg_le_gap_set_discovery_timing_cmd_t           cmd_le_gap_set_discovery_timing;
    struct gecko_msg_le_gap_set_discovery_timing_rsp_t           rsp_le_gap_set_discovery_timing;
    struct gecko_msg_le_gap_set_discovery_type_cmd_t             cmd_le_gap_set_discovery_type;
    struct gecko_msg_le_gap_set_discovery_type_rsp_t             rsp_le_gap_set_discovery_type;
    struct gecko_msg_le_gap_start_discovery_cmd_t                cmd_le_gap_start_discovery;
    struct gecko_msg_le_gap_start_discovery_rsp_t                rsp_le_gap_start_discovery;
    struct gecko_msg_le_gap_set_data_channel_classification_cmd_t cmd_le_gap_set_data_channel_classification;
    struct gecko_msg_le_gap_set_data_channel_classification_rsp_t rsp_le_gap_set_data_channel_classification;
    struct gecko_msg_le_gap_connect_cmd_t                        cmd_le_gap_connect;
    struct gecko_msg_le_gap_connect_rsp_t                        rsp_le_gap_connect;
    struct gecko_msg_le_gap_set_advertise_tx_power_cmd_t         cmd_le_gap_set_advertise_tx_power;
    struct gecko_msg_le_gap_set_advertise_tx_power_rsp_t         rsp_le_gap_set_advertise_tx_power;
    struct gecko_msg_le_gap_set_discovery_extended_scan_response_cmd_t cmd_le_gap_set_discovery_extended_scan_response;
    struct gecko_msg_le_gap_set_discovery_extended_scan_response_rsp_t rsp_le_gap_set_discovery_extended_scan_response;
    struct gecko_msg_le_gap_start_periodic_advertising_cmd_t     cmd_le_gap_start_periodic_advertising;
    struct gecko_msg_le_gap_start_periodic_advertising_rsp_t     rsp_le_gap_start_periodic_advertising;
    struct gecko_msg_le_gap_stop_periodic_advertising_cmd_t      cmd_le_gap_stop_periodic_advertising;
    struct gecko_msg_le_gap_stop_periodic_advertising_rsp_t      rsp_le_gap_stop_periodic_advertising;
    struct gecko_msg_le_gap_set_long_advertising_data_cmd_t      cmd_le_gap_set_long_advertising_data;
    struct gecko_msg_le_gap_set_long_advertising_data_rsp_t      rsp_le_gap_set_long_advertising_data;
    struct gecko_msg_le_gap_enable_whitelisting_cmd_t            cmd_le_gap_enable_whitelisting;
    struct gecko_msg_le_gap_enable_whitelisting_rsp_t            rsp_le_gap_enable_whitelisting;
    struct gecko_msg_le_gap_set_conn_timing_parameters_cmd_t     cmd_le_gap_set_conn_timing_parameters;
    struct gecko_msg_le_gap_set_conn_timing_parameters_rsp_t     rsp_le_gap_set_conn_timing_parameters;
    struct gecko_msg_le_gap_scan_response_evt_t                  evt_le_gap_scan_response;
    struct gecko_msg_le_gap_adv_timeout_evt_t                    evt_le_gap_adv_timeout;
    struct gecko_msg_le_gap_scan_request_evt_t                   evt_le_gap_scan_request;
    struct gecko_msg_le_gap_extended_scan_response_evt_t         evt_le_gap_extended_scan_response;
    struct gecko_msg_le_gap_periodic_advertising_status_evt_t    evt_le_gap_periodic_advertising_status;
    struct gecko_msg_sync_open_cmd_t                             cmd_sync_open;
    struct gecko_msg_sync_open_rsp_t                             rsp_sync_open;
    struct gecko_msg_sync_close_cmd_t                            cmd_sync_close;
    struct gecko_msg_sync_close_rsp_t                            rsp_sync_close;
    struct gecko_msg_sync_opened_evt_t                           evt_sync_opened;
    struct gecko_msg_sync_closed_evt_t                           evt_sync_closed;
    struct gecko_msg_sync_data_evt_t                             evt_sync_data;
    struct gecko_msg_le_connection_set_parameters_cmd_t          cmd_le_connection_set_parameters;
    struct gecko_msg_le_connection_set_parameters_rsp_t          rsp_le_connection_set_parameters;
    struct gecko_msg_le_connection_get_rssi_cmd_t                cmd_le_connection_get_rssi;
    struct gecko_msg_le_connection_get_rssi_rsp_t                rsp_le_connection_get_rssi;
    struct gecko_msg_le_connection_disable_slave_latency_cmd_t   cmd_le_connection_disable_slave_latency;
    struct gecko_msg_le_connection_disable_slave_latency_rsp_t   rsp_le_connection_disable_slave_latency;
    struct gecko_msg_le_connection_set_phy_cmd_t                 cmd_le_connection_set_phy;
    struct gecko_msg_le_connection_set_phy_rsp_t                 rsp_le_connection_set_phy;
    struct gecko_msg_le_connection_close_cmd_t                   cmd_le_connection_close;
    struct gecko_msg_le_connection_close_rsp_t                   rsp_le_connection_close;
    struct gecko_msg_le_connection_set_timing_parameters_cmd_t   cmd_le_connection_set_timing_parameters;
    struct gecko_msg_le_connection_set_timing_parameters_rsp_t   rsp_le_connection_set_timing_parameters;
    struct gecko_msg_le_connection_opened_evt_t                  evt_le_connection_opened;
    struct gecko_msg_le_connection_closed_evt_t                  evt_le_connection_closed;
    struct gecko_msg_le_connection_parameters_evt_t              evt_le_connection_parameters;
    struct gecko_msg_le_connection_rssi_evt_t                    evt_le_connection_rssi;
    struct gecko_msg_le_connection_phy_status_evt_t              evt_le_connection_phy_status;
    struct gecko_msg_gatt_set_max_mtu_cmd_t                      cmd_gatt_set_max_mtu;
    struct gecko_msg_gatt_set_max_mtu_rsp_t                      rsp_gatt_set_max_mtu;
    struct gecko_msg_gatt_discover_primary_services_cmd_t        cmd_gatt_discover_primary_services;
    struct gecko_msg_gatt_discover_primary_services_rsp_t        rsp_gatt_discover_primary_services;
    struct gecko_msg_gatt_discover_primary_services_by_uuid_cmd_t cmd_gatt_discover_primary_services_by_uuid;
    struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t rsp_gatt_discover_primary_services_by_uuid;
    struct gecko_msg_gatt_discover_characteristics_cmd_t         cmd_gatt_discover_characteristics;
    struct gecko_msg_gatt_discover_characteristics_rsp_t         rsp_gatt_discover_characteristics;
    struct gecko_msg_gatt_discover_characteristics_by_uuid_cmd_t cmd_gatt_discover_characteristics_by_uuid;
    struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t rsp_gatt_discover_characteristics_by_uuid;
    struct gecko_msg_gatt_set_characteristic_notification_cmd_t  cmd_gatt_set_characteristic_notification;
    struct gecko_msg_gatt_set_characteristic_notification_rsp_t  rsp_gatt_set_characteristic_notification;
    struct gecko_msg_gatt_discover_descriptors_cmd_t             cmd_gatt_discover_descriptors;
    struct gecko_msg_gatt_discover_descriptors_rsp_t             rsp_gatt_discover_descriptors;
    struct gecko_msg_gatt_read_characteristic_value_cmd_t        cmd_gatt_read_characteristic_value;
    struct gecko_msg_gatt_read_characteristic_value_rsp_t        rsp_gatt_read_characteristic_value;
    struct gecko_msg_gatt_read_characteristic_value_by_uuid_cmd_t cmd_gatt_read_characteristic_value_by_uuid;
    struct gecko_msg_gatt_read_characteristic_value_by_uuid_rsp_t rsp_gatt_read_characteristic_value_by_uuid;
    struct gecko_msg_gatt_write_characteristic_value_cmd_t       cmd_gatt_write_characteristic_value;
    struct gecko_msg_gatt_write_characteristic_value_rsp_t       rsp_gatt_write_characteristic_value;
    struct gecko_msg_gatt_write_characteristic_value_without_response_cmd_t cmd_gatt_write_characteristic_value_without_response;
    struct gecko_msg_gatt_write_characteristic_value_without_response_rsp_t rsp_gatt_write_characteristic_value_without_response;
    struct gecko_msg_gatt_prepare_characteristic_value_write_cmd_t cmd_gatt_prepare_characteristic_value_write;
    struct gecko_msg_gatt_prepare_characteristic_value_write_rsp_t rsp_gatt_prepare_characteristic_value_write;
    struct gecko_msg_gatt_execute_characteristic_value_write_cmd_t cmd_gatt_execute_characteristic_value_write;
    struct gecko_msg_gatt_execute_characteristic_value_write_rsp_t rsp_gatt_execute_characteristic_value_write;
    struct gecko_msg_gatt_send_characteristic_confirmation_cmd_t cmd_gatt_send_characteristic_confirmation;
    struct gecko_msg_gatt_send_characteristic_confirmation_rsp_t rsp_gatt_send_characteristic_confirmation;
    struct gecko_msg_gatt_read_descriptor_value_cmd_t            cmd_gatt_read_descriptor_value;
    struct gecko_msg_gatt_read_descriptor_value_rsp_t            rsp_gatt_read_descriptor_value;
    struct gecko_msg_gatt_write_descriptor_value_cmd_t           cmd_gatt_write_descriptor_value;
    struct gecko_msg_gatt_write_descriptor_value_rsp_t           rsp_gatt_write_descriptor_value;
    struct gecko_msg_gatt_find_included_services_cmd_t           cmd_gatt_find_included_services;
    struct gecko_msg_gatt_find_included_services_rsp_t           rsp_gatt_find_included_services;
    struct gecko_msg_gatt_read_multiple_characteristic_values_cmd_t cmd_gatt_read_multiple_characteristic_values;
    struct gecko_msg_gatt_read_multiple_characteristic_values_rsp_t rsp_gatt_read_multiple_characteristic_values;
    struct gecko_msg_gatt_read_characteristic_value_from_offset_cmd_t cmd_gatt_read_characteristic_value_from_offset;
    struct gecko_msg_gatt_read_characteristic_value_from_offset_rsp_t rsp_gatt_read_characteristic_value_from_offset;
    struct gecko_msg_gatt_prepare_characteristic_value_reliable_write_cmd_t cmd_gatt_prepare_characteristic_value_reliable_write;
    struct gecko_msg_gatt_prepare_characteristic_value_reliable_write_rsp_t rsp_gatt_prepare_characteristic_value_reliable_write;
    struct gecko_msg_gatt_mtu_exchanged_evt_t                    evt_gatt_mtu_exchanged;
    struct gecko_msg_gatt_service_evt_t                          evt_gatt_service;
    struct gecko_msg_gatt_characteristic_evt_t                   evt_gatt_characteristic;
    struct gecko_msg_gatt_descriptor_evt_t                       evt_gatt_descriptor;
    struct gecko_msg_gatt_characteristic_value_evt_t             evt_gatt_characteristic_value;
    struct gecko_msg_gatt_descriptor_value_evt_t                 evt_gatt_descriptor_value;
    struct gecko_msg_gatt_procedure_completed_evt_t              evt_gatt_procedure_completed;
    struct gecko_msg_gatt_server_read_attribute_value_cmd_t      cmd_gatt_server_read_attribute_value;
    struct gecko_msg_gatt_server_read_attribute_value_rsp_t      rsp_gatt_server_read_attribute_value;
    struct gecko_msg_gatt_server_read_attribute_type_cmd_t       cmd_gatt_server_read_attribute_type;
    struct gecko_msg_gatt_server_read_attribute_type_rsp_t       rsp_gatt_server_read_attribute_type;
    struct gecko_msg_gatt_server_write_attribute_value_cmd_t     cmd_gatt_server_write_attribute_value;
    struct gecko_msg_gatt_server_write_attribute_value_rsp_t     rsp_gatt_server_write_attribute_value;
    struct gecko_msg_gatt_server_send_user_read_response_cmd_t   cmd_gatt_server_send_user_read_response;
    struct gecko_msg_gatt_server_send_user_read_response_rsp_t   rsp_gatt_server_send_user_read_response;
    struct gecko_msg_gatt_server_send_user_write_response_cmd_t  cmd_gatt_server_send_user_write_response;
    struct gecko_msg_gatt_server_send_user_write_response_rsp_t  rsp_gatt_server_send_user_write_response;
    struct gecko_msg_gatt_server_send_characteristic_notification_cmd_t cmd_gatt_server_send_characteristic_notification;
    struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t rsp_gatt_server_send_characteristic_notification;
    struct gecko_msg_gatt_server_find_attribute_cmd_t            cmd_gatt_server_find_attribute;
    struct gecko_msg_gatt_server_find_attribute_rsp_t            rsp_gatt_server_find_attribute;
    struct gecko_msg_gatt_server_set_capabilities_cmd_t          cmd_gatt_server_set_capabilities;
    struct gecko_msg_gatt_server_set_capabilities_rsp_t          rsp_gatt_server_set_capabilities;
    struct gecko_msg_gatt_server_set_max_mtu_cmd_t               cmd_gatt_server_set_max_mtu;
    struct gecko_msg_gatt_server_set_max_mtu_rsp_t               rsp_gatt_server_set_max_mtu;
    struct gecko_msg_gatt_server_attribute_value_evt_t           evt_gatt_server_attribute_value;
    struct gecko_msg_gatt_server_user_read_request_evt_t         evt_gatt_server_user_read_request;
    struct gecko_msg_gatt_server_user_write_request_evt_t        evt_gatt_server_user_write_request;
    struct gecko_msg_gatt_server_characteristic_status_evt_t     evt_gatt_server_characteristic_status;
    struct gecko_msg_gatt_server_execute_write_completed_evt_t   evt_gatt_server_execute_write_completed;
    struct gecko_msg_hardware_set_soft_timer_cmd_t               cmd_hardware_set_soft_timer;
    struct gecko_msg_hardware_set_soft_timer_rsp_t               rsp_hardware_set_soft_timer;
    struct gecko_msg_hardware_get_time_rsp_t                     rsp_hardware_get_time;
    struct gecko_msg_hardware_set_lazy_soft_timer_cmd_t          cmd_hardware_set_lazy_soft_timer;
    struct gecko_msg_hardware_set_lazy_soft_timer_rsp_t          rsp_hardware_set_lazy_soft_timer;
    struct gecko_msg_hardware_soft_timer_evt_t                   evt_hardware_soft_timer;
    struct gecko_msg_flash_ps_erase_all_rsp_t                    rsp_flash_ps_erase_all;
    struct gecko_msg_flash_ps_save_cmd_t                         cmd_flash_ps_save;
    struct gecko_msg_flash_ps_save_rsp_t                         rsp_flash_ps_save;
    struct gecko_msg_flash_ps_load_cmd_t                         cmd_flash_ps_load;
    struct gecko_msg_flash_ps_load_rsp_t                         rsp_flash_ps_load;
    struct gecko_msg_flash_ps_erase_cmd_t                        cmd_flash_ps_erase;
    struct gecko_msg_flash_ps_erase_rsp_t                        rsp_flash_ps_erase;
    struct gecko_msg_test_dtm_tx_cmd_t                           cmd_test_dtm_tx;
    struct gecko_msg_test_dtm_tx_rsp_t                           rsp_test_dtm_tx;
    struct gecko_msg_test_dtm_rx_cmd_t                           cmd_test_dtm_rx;
    struct gecko_msg_test_dtm_rx_rsp_t                           rsp_test_dtm_rx;
    struct gecko_msg_test_dtm_end_rsp_t                          rsp_test_dtm_end;
    struct gecko_msg_test_dtm_completed_evt_t                    evt_test_dtm_completed;
    struct gecko_msg_sm_set_bondable_mode_cmd_t                  cmd_sm_set_bondable_mode;
    struct gecko_msg_sm_set_bondable_mode_rsp_t                  rsp_sm_set_bondable_mode;
    struct gecko_msg_sm_configure_cmd_t                          cmd_sm_configure;
    struct gecko_msg_sm_configure_rsp_t                          rsp_sm_configure;
    struct gecko_msg_sm_store_bonding_configuration_cmd_t        cmd_sm_store_bonding_configuration;
    struct gecko_msg_sm_store_bonding_configuration_rsp_t        rsp_sm_store_bonding_configuration;
    struct gecko_msg_sm_increase_security_cmd_t                  cmd_sm_increase_security;
    struct gecko_msg_sm_increase_security_rsp_t                  rsp_sm_increase_security;
    struct gecko_msg_sm_delete_bonding_cmd_t                     cmd_sm_delete_bonding;
    struct gecko_msg_sm_delete_bonding_rsp_t                     rsp_sm_delete_bonding;
    struct gecko_msg_sm_delete_bondings_rsp_t                    rsp_sm_delete_bondings;
    struct gecko_msg_sm_enter_passkey_cmd_t                      cmd_sm_enter_passkey;
    struct gecko_msg_sm_enter_passkey_rsp_t                      rsp_sm_enter_passkey;
    struct gecko_msg_sm_passkey_confirm_cmd_t                    cmd_sm_passkey_confirm;
    struct gecko_msg_sm_passkey_confirm_rsp_t                    rsp_sm_passkey_confirm;
    struct gecko_msg_sm_set_oob_data_cmd_t                       cmd_sm_set_oob_data;
    struct gecko_msg_sm_set_oob_data_rsp_t                       rsp_sm_set_oob_data;
    struct gecko_msg_sm_list_all_bondings_rsp_t                  rsp_sm_list_all_bondings;
    struct gecko_msg_sm_bonding_confirm_cmd_t                    cmd_sm_bonding_confirm;
    struct gecko_msg_sm_bonding_confirm_rsp_t                    rsp_sm_bonding_confirm;
    struct gecko_msg_sm_set_debug_mode_rsp_t                     rsp_sm_set_debug_mode;
    struct gecko_msg_sm_set_passkey_cmd_t                        cmd_sm_set_passkey;
    struct gecko_msg_sm_set_passkey_rsp_t                        rsp_sm_set_passkey;
    struct gecko_msg_sm_use_sc_oob_cmd_t                         cmd_sm_use_sc_oob;
    struct gecko_msg_sm_use_sc_oob_rsp_t                         rsp_sm_use_sc_oob;
    struct gecko_msg_sm_set_sc_remote_oob_data_cmd_t             cmd_sm_set_sc_remote_oob_data;
    struct gecko_msg_sm_set_sc_remote_oob_data_rsp_t             rsp_sm_set_sc_remote_oob_data;
    struct gecko_msg_sm_add_to_whitelist_cmd_t                   cmd_sm_add_to_whitelist;
    struct gecko_msg_sm_add_to_whitelist_rsp_t                   rsp_sm_add_to_whitelist;
    struct gecko_msg_sm_set_minimum_key_size_cmd_t               cmd_sm_set_minimum_key_size;
    struct gecko_msg_sm_set_minimum_key_size_rsp_t               rsp_sm_set_minimum_key_size;
    struct gecko_msg_sm_passkey_display_evt_t                    evt_sm_passkey_display;
    struct gecko_msg_sm_passkey_request_evt_t                    evt_sm_passkey_request;
    struct gecko_msg_sm_confirm_passkey_evt_t                    evt_sm_confirm_passkey;
    struct gecko_msg_sm_bonded_evt_t                             evt_sm_bonded;
    struct gecko_msg_sm_bonding_failed_evt_t                     evt_sm_bonding_failed;
    struct gecko_msg_sm_list_bonding_entry_evt_t                 evt_sm_list_bonding_entry;
    struct gecko_msg_sm_confirm_bonding_evt_t                    evt_sm_confirm_bonding;
    struct gecko_msg_homekit_configure_cmd_t                     cmd_homekit_configure;
    struct gecko_msg_homekit_configure_rsp_t                     rsp_homekit_configure;
    struct gecko_msg_homekit_advertise_cmd_t                     cmd_homekit_advertise;
    struct gecko_msg_homekit_advertise_rsp_t                     rsp_homekit_advertise;
    struct gecko_msg_homekit_delete_pairings_rsp_t               rsp_homekit_delete_pairings;
    struct gecko_msg_homekit_check_authcp_rsp_t                  rsp_homekit_check_authcp;
    struct gecko_msg_homekit_get_pairing_id_cmd_t                cmd_homekit_get_pairing_id;
    struct gecko_msg_homekit_get_pairing_id_rsp_t                rsp_homekit_get_pairing_id;
    struct gecko_msg_homekit_send_write_response_cmd_t           cmd_homekit_send_write_response;
    struct gecko_msg_homekit_send_write_response_rsp_t           rsp_homekit_send_write_response;
    struct gecko_msg_homekit_send_read_response_cmd_t            cmd_homekit_send_read_response;
    struct gecko_msg_homekit_send_read_response_rsp_t            rsp_homekit_send_read_response;
    struct gecko_msg_homekit_gsn_action_cmd_t                    cmd_homekit_gsn_action;
    struct gecko_msg_homekit_gsn_action_rsp_t                    rsp_homekit_gsn_action;
    struct gecko_msg_homekit_event_notification_cmd_t            cmd_homekit_event_notification;
    struct gecko_msg_homekit_event_notification_rsp_t            rsp_homekit_event_notification;
    struct gecko_msg_homekit_broadcast_action_cmd_t              cmd_homekit_broadcast_action;
    struct gecko_msg_homekit_broadcast_action_rsp_t              rsp_homekit_broadcast_action;
    struct gecko_msg_homekit_setupcode_display_evt_t             evt_homekit_setupcode_display;
    struct gecko_msg_homekit_paired_evt_t                        evt_homekit_paired;
    struct gecko_msg_homekit_pair_verified_evt_t                 evt_homekit_pair_verified;
    struct gecko_msg_homekit_connection_opened_evt_t             evt_homekit_connection_opened;
    struct gecko_msg_homekit_connection_closed_evt_t             evt_homekit_connection_closed;
    struct gecko_msg_homekit_identify_evt_t                      evt_homekit_identify;
    struct gecko_msg_homekit_write_request_evt_t                 evt_homekit_write_request;
    struct gecko_msg_homekit_read_request_evt_t                  evt_homekit_read_request;
    struct gecko_msg_homekit_disconnection_required_evt_t        evt_homekit_disconnection_required;
    struct gecko_msg_homekit_pairing_removed_evt_t               evt_homekit_pairing_removed;
    struct gecko_msg_homekit_setuppayload_display_evt_t          evt_homekit_setuppayload_display;
    struct gecko_msg_mesh_node_init_rsp_t                        rsp_mesh_node_init;
    struct gecko_msg_mesh_node_start_unprov_beaconing_cmd_t      cmd_mesh_node_start_unprov_beaconing;
    struct gecko_msg_mesh_node_start_unprov_beaconing_rsp_t      rsp_mesh_node_start_unprov_beaconing;
    struct gecko_msg_mesh_node_input_oob_request_rsp_cmd_t       cmd_mesh_node_input_oob_request_rsp;
    struct gecko_msg_mesh_node_input_oob_request_rsp_rsp_t       rsp_mesh_node_input_oob_request_rsp;
    struct gecko_msg_mesh_node_get_uuid_rsp_t                    rsp_mesh_node_get_uuid;
    struct gecko_msg_mesh_node_set_provisioning_data_cmd_t       cmd_mesh_node_set_provisioning_data;
    struct gecko_msg_mesh_node_set_provisioning_data_rsp_t       rsp_mesh_node_set_provisioning_data;
    struct gecko_msg_mesh_node_init_oob_cmd_t                    cmd_mesh_node_init_oob;
    struct gecko_msg_mesh_node_init_oob_rsp_t                    rsp_mesh_node_init_oob;
    struct gecko_msg_mesh_node_set_ivrecovery_mode_cmd_t         cmd_mesh_node_set_ivrecovery_mode;
    struct gecko_msg_mesh_node_set_ivrecovery_mode_rsp_t         rsp_mesh_node_set_ivrecovery_mode;
    struct gecko_msg_mesh_node_get_ivrecovery_mode_rsp_t         rsp_mesh_node_get_ivrecovery_mode;
    struct gecko_msg_mesh_node_set_adv_event_filter_cmd_t        cmd_mesh_node_set_adv_event_filter;
    struct gecko_msg_mesh_node_set_adv_event_filter_rsp_t        rsp_mesh_node_set_adv_event_filter;
    struct gecko_msg_mesh_node_get_statistics_rsp_t              rsp_mesh_node_get_statistics;
    struct gecko_msg_mesh_node_clear_statistics_rsp_t            rsp_mesh_node_clear_statistics;
    struct gecko_msg_mesh_node_set_net_relay_delay_cmd_t         cmd_mesh_node_set_net_relay_delay;
    struct gecko_msg_mesh_node_set_net_relay_delay_rsp_t         rsp_mesh_node_set_net_relay_delay;
    struct gecko_msg_mesh_node_get_net_relay_delay_rsp_t         rsp_mesh_node_get_net_relay_delay;
    struct gecko_msg_mesh_node_get_ivupdate_state_rsp_t          rsp_mesh_node_get_ivupdate_state;
    struct gecko_msg_mesh_node_request_ivupdate_rsp_t            rsp_mesh_node_request_ivupdate;
    struct gecko_msg_mesh_node_get_seq_remaining_cmd_t           cmd_mesh_node_get_seq_remaining;
    struct gecko_msg_mesh_node_get_seq_remaining_rsp_t           rsp_mesh_node_get_seq_remaining;
    struct gecko_msg_mesh_node_save_replay_protection_list_rsp_t rsp_mesh_node_save_replay_protection_list;
    struct gecko_msg_mesh_node_set_uuid_cmd_t                    cmd_mesh_node_set_uuid;
    struct gecko_msg_mesh_node_set_uuid_rsp_t                    rsp_mesh_node_set_uuid;
    struct gecko_msg_mesh_node_get_element_address_cmd_t         cmd_mesh_node_get_element_address;
    struct gecko_msg_mesh_node_get_element_address_rsp_t         rsp_mesh_node_get_element_address;
    struct gecko_msg_mesh_node_static_oob_request_rsp_cmd_t      cmd_mesh_node_static_oob_request_rsp;
    struct gecko_msg_mesh_node_static_oob_request_rsp_rsp_t      rsp_mesh_node_static_oob_request_rsp;
    struct gecko_msg_mesh_node_reset_rsp_t                       rsp_mesh_node_reset;
    struct gecko_msg_mesh_node_initialized_evt_t                 evt_mesh_node_initialized;
    struct gecko_msg_mesh_node_provisioned_evt_t                 evt_mesh_node_provisioned;
    struct gecko_msg_mesh_node_config_get_evt_t                  evt_mesh_node_config_get;
    struct gecko_msg_mesh_node_config_set_evt_t                  evt_mesh_node_config_set;
    struct gecko_msg_mesh_node_display_output_oob_evt_t          evt_mesh_node_display_output_oob;
    struct gecko_msg_mesh_node_input_oob_request_evt_t           evt_mesh_node_input_oob_request;
    struct gecko_msg_mesh_node_provisioning_started_evt_t        evt_mesh_node_provisioning_started;
    struct gecko_msg_mesh_node_provisioning_failed_evt_t         evt_mesh_node_provisioning_failed;
    struct gecko_msg_mesh_node_key_added_evt_t                   evt_mesh_node_key_added;
    struct gecko_msg_mesh_node_model_config_changed_evt_t        evt_mesh_node_model_config_changed;
    struct gecko_msg_mesh_node_ivrecovery_needed_evt_t           evt_mesh_node_ivrecovery_needed;
    struct gecko_msg_mesh_node_changed_ivupdate_state_evt_t      evt_mesh_node_changed_ivupdate_state;
    struct gecko_msg_mesh_node_key_removed_evt_t                 evt_mesh_node_key_removed;
    struct gecko_msg_mesh_node_key_updated_evt_t                 evt_mesh_node_key_updated;
    struct gecko_msg_mesh_prov_init_rsp_t                        rsp_mesh_prov_init;
    struct gecko_msg_mesh_prov_scan_unprov_beacons_rsp_t         rsp_mesh_prov_scan_unprov_beacons;
    struct gecko_msg_mesh_prov_provision_device_cmd_t            cmd_mesh_prov_provision_device;
    struct gecko_msg_mesh_prov_provision_device_rsp_t            rsp_mesh_prov_provision_device;
    struct gecko_msg_mesh_prov_create_network_cmd_t              cmd_mesh_prov_create_network;
    struct gecko_msg_mesh_prov_create_network_rsp_t              rsp_mesh_prov_create_network;
    struct gecko_msg_mesh_prov_get_dcd_cmd_t                     cmd_mesh_prov_get_dcd;
    struct gecko_msg_mesh_prov_get_dcd_rsp_t                     rsp_mesh_prov_get_dcd;
    struct gecko_msg_mesh_prov_get_config_cmd_t                  cmd_mesh_prov_get_config;
    struct gecko_msg_mesh_prov_get_config_rsp_t                  rsp_mesh_prov_get_config;
    struct gecko_msg_mesh_prov_set_config_cmd_t                  cmd_mesh_prov_set_config;
    struct gecko_msg_mesh_prov_set_config_rsp_t                  rsp_mesh_prov_set_config;
    struct gecko_msg_mesh_prov_create_appkey_cmd_t               cmd_mesh_prov_create_appkey;
    struct gecko_msg_mesh_prov_create_appkey_rsp_t               rsp_mesh_prov_create_appkey;
    struct gecko_msg_mesh_prov_oob_pkey_rsp_cmd_t                cmd_mesh_prov_oob_pkey_rsp;
    struct gecko_msg_mesh_prov_oob_pkey_rsp_rsp_t                rsp_mesh_prov_oob_pkey_rsp;
    struct gecko_msg_mesh_prov_oob_auth_rsp_cmd_t                cmd_mesh_prov_oob_auth_rsp;
    struct gecko_msg_mesh_prov_oob_auth_rsp_rsp_t                rsp_mesh_prov_oob_auth_rsp;
    struct gecko_msg_mesh_prov_set_oob_requirements_cmd_t        cmd_mesh_prov_set_oob_requirements;
    struct gecko_msg_mesh_prov_set_oob_requirements_rsp_t        rsp_mesh_prov_set_oob_requirements;
    struct gecko_msg_mesh_prov_key_refresh_start_cmd_t           cmd_mesh_prov_key_refresh_start;
    struct gecko_msg_mesh_prov_key_refresh_start_rsp_t           rsp_mesh_prov_key_refresh_start;
    struct gecko_msg_mesh_prov_get_key_refresh_blacklist_cmd_t   cmd_mesh_prov_get_key_refresh_blacklist;
    struct gecko_msg_mesh_prov_get_key_refresh_blacklist_rsp_t   rsp_mesh_prov_get_key_refresh_blacklist;
    struct gecko_msg_mesh_prov_set_key_refresh_blacklist_cmd_t   cmd_mesh_prov_set_key_refresh_blacklist;
    struct gecko_msg_mesh_prov_set_key_refresh_blacklist_rsp_t   rsp_mesh_prov_set_key_refresh_blacklist;
    struct gecko_msg_mesh_prov_appkey_add_cmd_t                  cmd_mesh_prov_appkey_add;
    struct gecko_msg_mesh_prov_appkey_add_rsp_t                  rsp_mesh_prov_appkey_add;
    struct gecko_msg_mesh_prov_appkey_delete_cmd_t               cmd_mesh_prov_appkey_delete;
    struct gecko_msg_mesh_prov_appkey_delete_rsp_t               rsp_mesh_prov_appkey_delete;
    struct gecko_msg_mesh_prov_model_app_bind_cmd_t              cmd_mesh_prov_model_app_bind;
    struct gecko_msg_mesh_prov_model_app_bind_rsp_t              rsp_mesh_prov_model_app_bind;
    struct gecko_msg_mesh_prov_model_app_unbind_cmd_t            cmd_mesh_prov_model_app_unbind;
    struct gecko_msg_mesh_prov_model_app_unbind_rsp_t            rsp_mesh_prov_model_app_unbind;
    struct gecko_msg_mesh_prov_model_app_get_cmd_t               cmd_mesh_prov_model_app_get;
    struct gecko_msg_mesh_prov_model_app_get_rsp_t               rsp_mesh_prov_model_app_get;
    struct gecko_msg_mesh_prov_model_sub_add_cmd_t               cmd_mesh_prov_model_sub_add;
    struct gecko_msg_mesh_prov_model_sub_add_rsp_t               rsp_mesh_prov_model_sub_add;
    struct gecko_msg_mesh_prov_model_pub_set_cmd_t               cmd_mesh_prov_model_pub_set;
    struct gecko_msg_mesh_prov_model_pub_set_rsp_t               rsp_mesh_prov_model_pub_set;
    struct gecko_msg_mesh_prov_provision_gatt_device_cmd_t       cmd_mesh_prov_provision_gatt_device;
    struct gecko_msg_mesh_prov_provision_gatt_device_rsp_t       rsp_mesh_prov_provision_gatt_device;
    struct gecko_msg_mesh_prov_ddb_get_cmd_t                     cmd_mesh_prov_ddb_get;
    struct gecko_msg_mesh_prov_ddb_get_rsp_t                     rsp_mesh_prov_ddb_get;
    struct gecko_msg_mesh_prov_ddb_delete_cmd_t                  cmd_mesh_prov_ddb_delete;
    struct gecko_msg_mesh_prov_ddb_delete_rsp_t                  rsp_mesh_prov_ddb_delete;
    struct gecko_msg_mesh_prov_ddb_add_cmd_t                     cmd_mesh_prov_ddb_add;
    struct gecko_msg_mesh_prov_ddb_add_rsp_t                     rsp_mesh_prov_ddb_add;
    struct gecko_msg_mesh_prov_ddb_list_devices_rsp_t            rsp_mesh_prov_ddb_list_devices;
    struct gecko_msg_mesh_prov_network_add_cmd_t                 cmd_mesh_prov_network_add;
    struct gecko_msg_mesh_prov_network_add_rsp_t                 rsp_mesh_prov_network_add;
    struct gecko_msg_mesh_prov_network_delete_cmd_t              cmd_mesh_prov_network_delete;
    struct gecko_msg_mesh_prov_network_delete_rsp_t              rsp_mesh_prov_network_delete;
    struct gecko_msg_mesh_prov_nettx_get_cmd_t                   cmd_mesh_prov_nettx_get;
    struct gecko_msg_mesh_prov_nettx_get_rsp_t                   rsp_mesh_prov_nettx_get;
    struct gecko_msg_mesh_prov_nettx_set_cmd_t                   cmd_mesh_prov_nettx_set;
    struct gecko_msg_mesh_prov_nettx_set_rsp_t                   rsp_mesh_prov_nettx_set;
    struct gecko_msg_mesh_prov_model_sub_del_cmd_t               cmd_mesh_prov_model_sub_del;
    struct gecko_msg_mesh_prov_model_sub_del_rsp_t               rsp_mesh_prov_model_sub_del;
    struct gecko_msg_mesh_prov_model_sub_add_va_cmd_t            cmd_mesh_prov_model_sub_add_va;
    struct gecko_msg_mesh_prov_model_sub_add_va_rsp_t            rsp_mesh_prov_model_sub_add_va;
    struct gecko_msg_mesh_prov_model_sub_del_va_cmd_t            cmd_mesh_prov_model_sub_del_va;
    struct gecko_msg_mesh_prov_model_sub_del_va_rsp_t            rsp_mesh_prov_model_sub_del_va;
    struct gecko_msg_mesh_prov_model_sub_set_cmd_t               cmd_mesh_prov_model_sub_set;
    struct gecko_msg_mesh_prov_model_sub_set_rsp_t               rsp_mesh_prov_model_sub_set;
    struct gecko_msg_mesh_prov_model_sub_set_va_cmd_t            cmd_mesh_prov_model_sub_set_va;
    struct gecko_msg_mesh_prov_model_sub_set_va_rsp_t            rsp_mesh_prov_model_sub_set_va;
    struct gecko_msg_mesh_prov_heartbeat_publication_get_cmd_t   cmd_mesh_prov_heartbeat_publication_get;
    struct gecko_msg_mesh_prov_heartbeat_publication_get_rsp_t   rsp_mesh_prov_heartbeat_publication_get;
    struct gecko_msg_mesh_prov_heartbeat_publication_set_cmd_t   cmd_mesh_prov_heartbeat_publication_set;
    struct gecko_msg_mesh_prov_heartbeat_publication_set_rsp_t   rsp_mesh_prov_heartbeat_publication_set;
    struct gecko_msg_mesh_prov_heartbeat_subscription_get_cmd_t  cmd_mesh_prov_heartbeat_subscription_get;
    struct gecko_msg_mesh_prov_heartbeat_subscription_get_rsp_t  rsp_mesh_prov_heartbeat_subscription_get;
    struct gecko_msg_mesh_prov_heartbeat_subscription_set_cmd_t  cmd_mesh_prov_heartbeat_subscription_set;
    struct gecko_msg_mesh_prov_heartbeat_subscription_set_rsp_t  rsp_mesh_prov_heartbeat_subscription_set;
    struct gecko_msg_mesh_prov_relay_get_cmd_t                   cmd_mesh_prov_relay_get;
    struct gecko_msg_mesh_prov_relay_get_rsp_t                   rsp_mesh_prov_relay_get;
    struct gecko_msg_mesh_prov_relay_set_cmd_t                   cmd_mesh_prov_relay_set;
    struct gecko_msg_mesh_prov_relay_set_rsp_t                   rsp_mesh_prov_relay_set;
    struct gecko_msg_mesh_prov_reset_node_cmd_t                  cmd_mesh_prov_reset_node;
    struct gecko_msg_mesh_prov_reset_node_rsp_t                  rsp_mesh_prov_reset_node;
    struct gecko_msg_mesh_prov_appkey_get_cmd_t                  cmd_mesh_prov_appkey_get;
    struct gecko_msg_mesh_prov_appkey_get_rsp_t                  rsp_mesh_prov_appkey_get;
    struct gecko_msg_mesh_prov_network_get_cmd_t                 cmd_mesh_prov_network_get;
    struct gecko_msg_mesh_prov_network_get_rsp_t                 rsp_mesh_prov_network_get;
    struct gecko_msg_mesh_prov_model_sub_clear_cmd_t             cmd_mesh_prov_model_sub_clear;
    struct gecko_msg_mesh_prov_model_sub_clear_rsp_t             rsp_mesh_prov_model_sub_clear;
    struct gecko_msg_mesh_prov_model_pub_get_cmd_t               cmd_mesh_prov_model_pub_get;
    struct gecko_msg_mesh_prov_model_pub_get_rsp_t               rsp_mesh_prov_model_pub_get;
    struct gecko_msg_mesh_prov_model_pub_set_va_cmd_t            cmd_mesh_prov_model_pub_set_va;
    struct gecko_msg_mesh_prov_model_pub_set_va_rsp_t            rsp_mesh_prov_model_pub_set_va;
    struct gecko_msg_mesh_prov_model_pub_set_cred_cmd_t          cmd_mesh_prov_model_pub_set_cred;
    struct gecko_msg_mesh_prov_model_pub_set_cred_rsp_t          rsp_mesh_prov_model_pub_set_cred;
    struct gecko_msg_mesh_prov_model_pub_set_va_cred_cmd_t       cmd_mesh_prov_model_pub_set_va_cred;
    struct gecko_msg_mesh_prov_model_pub_set_va_cred_rsp_t       rsp_mesh_prov_model_pub_set_va_cred;
    struct gecko_msg_mesh_prov_model_sub_get_cmd_t               cmd_mesh_prov_model_sub_get;
    struct gecko_msg_mesh_prov_model_sub_get_rsp_t               rsp_mesh_prov_model_sub_get;
    struct gecko_msg_mesh_prov_friend_timeout_get_cmd_t          cmd_mesh_prov_friend_timeout_get;
    struct gecko_msg_mesh_prov_friend_timeout_get_rsp_t          rsp_mesh_prov_friend_timeout_get;
    struct gecko_msg_mesh_prov_get_default_configuration_timeout_rsp_t rsp_mesh_prov_get_default_configuration_timeout;
    struct gecko_msg_mesh_prov_set_default_configuration_timeout_cmd_t cmd_mesh_prov_set_default_configuration_timeout;
    struct gecko_msg_mesh_prov_set_default_configuration_timeout_rsp_t rsp_mesh_prov_set_default_configuration_timeout;
    struct gecko_msg_mesh_prov_provision_device_with_address_cmd_t cmd_mesh_prov_provision_device_with_address;
    struct gecko_msg_mesh_prov_provision_device_with_address_rsp_t rsp_mesh_prov_provision_device_with_address;
    struct gecko_msg_mesh_prov_provision_gatt_device_with_address_cmd_t cmd_mesh_prov_provision_gatt_device_with_address;
    struct gecko_msg_mesh_prov_provision_gatt_device_with_address_rsp_t rsp_mesh_prov_provision_gatt_device_with_address;
    struct gecko_msg_mesh_prov_initialize_network_cmd_t          cmd_mesh_prov_initialize_network;
    struct gecko_msg_mesh_prov_initialize_network_rsp_t          rsp_mesh_prov_initialize_network;
    struct gecko_msg_mesh_prov_get_key_refresh_appkey_blacklist_cmd_t cmd_mesh_prov_get_key_refresh_appkey_blacklist;
    struct gecko_msg_mesh_prov_get_key_refresh_appkey_blacklist_rsp_t rsp_mesh_prov_get_key_refresh_appkey_blacklist;
    struct gecko_msg_mesh_prov_set_key_refresh_appkey_blacklist_cmd_t cmd_mesh_prov_set_key_refresh_appkey_blacklist;
    struct gecko_msg_mesh_prov_set_key_refresh_appkey_blacklist_rsp_t rsp_mesh_prov_set_key_refresh_appkey_blacklist;
    struct gecko_msg_mesh_prov_stop_scan_unprov_beacons_rsp_t    rsp_mesh_prov_stop_scan_unprov_beacons;
    struct gecko_msg_mesh_prov_initialized_evt_t                 evt_mesh_prov_initialized;
    struct gecko_msg_mesh_prov_provisioning_failed_evt_t         evt_mesh_prov_provisioning_failed;
    struct gecko_msg_mesh_prov_device_provisioned_evt_t          evt_mesh_prov_device_provisioned;
    struct gecko_msg_mesh_prov_unprov_beacon_evt_t               evt_mesh_prov_unprov_beacon;
    struct gecko_msg_mesh_prov_dcd_status_evt_t                  evt_mesh_prov_dcd_status;
    struct gecko_msg_mesh_prov_config_status_evt_t               evt_mesh_prov_config_status;
    struct gecko_msg_mesh_prov_oob_pkey_request_evt_t            evt_mesh_prov_oob_pkey_request;
    struct gecko_msg_mesh_prov_oob_auth_request_evt_t            evt_mesh_prov_oob_auth_request;
    struct gecko_msg_mesh_prov_oob_display_input_evt_t           evt_mesh_prov_oob_display_input;
    struct gecko_msg_mesh_prov_ddb_list_evt_t                    evt_mesh_prov_ddb_list;
    struct gecko_msg_mesh_prov_heartbeat_publication_status_evt_t evt_mesh_prov_heartbeat_publication_status;
    struct gecko_msg_mesh_prov_heartbeat_subscription_status_evt_t evt_mesh_prov_heartbeat_subscription_status;
    struct gecko_msg_mesh_prov_relay_status_evt_t                evt_mesh_prov_relay_status;
    struct gecko_msg_mesh_prov_uri_evt_t                         evt_mesh_prov_uri;
    struct gecko_msg_mesh_prov_node_reset_evt_t                  evt_mesh_prov_node_reset;
    struct gecko_msg_mesh_prov_appkey_list_evt_t                 evt_mesh_prov_appkey_list;
    struct gecko_msg_mesh_prov_appkey_list_end_evt_t             evt_mesh_prov_appkey_list_end;
    struct gecko_msg_mesh_prov_network_list_evt_t                evt_mesh_prov_network_list;
    struct gecko_msg_mesh_prov_network_list_end_evt_t            evt_mesh_prov_network_list_end;
    struct gecko_msg_mesh_prov_model_pub_status_evt_t            evt_mesh_prov_model_pub_status;
    struct gecko_msg_mesh_prov_key_refresh_phase_update_evt_t    evt_mesh_prov_key_refresh_phase_update;
    struct gecko_msg_mesh_prov_key_refresh_node_update_evt_t     evt_mesh_prov_key_refresh_node_update;
    struct gecko_msg_mesh_prov_key_refresh_complete_evt_t        evt_mesh_prov_key_refresh_complete;
    struct gecko_msg_mesh_prov_model_sub_addr_evt_t              evt_mesh_prov_model_sub_addr;
    struct gecko_msg_mesh_prov_model_sub_addr_end_evt_t          evt_mesh_prov_model_sub_addr_end;
    struct gecko_msg_mesh_prov_friend_timeout_status_evt_t       evt_mesh_prov_friend_timeout_status;
    struct gecko_msg_mesh_proxy_connect_cmd_t                    cmd_mesh_proxy_connect;
    struct gecko_msg_mesh_proxy_connect_rsp_t                    rsp_mesh_proxy_connect;
    struct gecko_msg_mesh_proxy_disconnect_cmd_t                 cmd_mesh_proxy_disconnect;
    struct gecko_msg_mesh_proxy_disconnect_rsp_t                 rsp_mesh_proxy_disconnect;
    struct gecko_msg_mesh_proxy_set_filter_type_cmd_t            cmd_mesh_proxy_set_filter_type;
    struct gecko_msg_mesh_proxy_set_filter_type_rsp_t            rsp_mesh_proxy_set_filter_type;
    struct gecko_msg_mesh_proxy_allow_cmd_t                      cmd_mesh_proxy_allow;
    struct gecko_msg_mesh_proxy_allow_rsp_t                      rsp_mesh_proxy_allow;
    struct gecko_msg_mesh_proxy_deny_cmd_t                       cmd_mesh_proxy_deny;
    struct gecko_msg_mesh_proxy_deny_rsp_t                       rsp_mesh_proxy_deny;
    struct gecko_msg_mesh_proxy_connected_evt_t                  evt_mesh_proxy_connected;
    struct gecko_msg_mesh_proxy_disconnected_evt_t               evt_mesh_proxy_disconnected;
    struct gecko_msg_mesh_proxy_filter_status_evt_t              evt_mesh_proxy_filter_status;
    struct gecko_msg_mesh_vendor_model_send_cmd_t                cmd_mesh_vendor_model_send;
    struct gecko_msg_mesh_vendor_model_send_rsp_t                rsp_mesh_vendor_model_send;
    struct gecko_msg_mesh_vendor_model_set_publication_cmd_t     cmd_mesh_vendor_model_set_publication;
    struct gecko_msg_mesh_vendor_model_set_publication_rsp_t     rsp_mesh_vendor_model_set_publication;
    struct gecko_msg_mesh_vendor_model_clear_publication_cmd_t   cmd_mesh_vendor_model_clear_publication;
    struct gecko_msg_mesh_vendor_model_clear_publication_rsp_t   rsp_mesh_vendor_model_clear_publication;
    struct gecko_msg_mesh_vendor_model_publish_cmd_t             cmd_mesh_vendor_model_publish;
    struct gecko_msg_mesh_vendor_model_publish_rsp_t             rsp_mesh_vendor_model_publish;
    struct gecko_msg_mesh_vendor_model_init_cmd_t                cmd_mesh_vendor_model_init;
    struct gecko_msg_mesh_vendor_model_init_rsp_t                rsp_mesh_vendor_model_init;
    struct gecko_msg_mesh_vendor_model_deinit_cmd_t              cmd_mesh_vendor_model_deinit;
    struct gecko_msg_mesh_vendor_model_deinit_rsp_t              rsp_mesh_vendor_model_deinit;
    struct gecko_msg_mesh_vendor_model_receive_evt_t             evt_mesh_vendor_model_receive;
    struct gecko_msg_mesh_health_client_get_cmd_t                cmd_mesh_health_client_get;
    struct gecko_msg_mesh_health_client_get_rsp_t                rsp_mesh_health_client_get;
    struct gecko_msg_mesh_health_client_clear_cmd_t              cmd_mesh_health_client_clear;
    struct gecko_msg_mesh_health_client_clear_rsp_t              rsp_mesh_health_client_clear;
    struct gecko_msg_mesh_health_client_test_cmd_t               cmd_mesh_health_client_test;
    struct gecko_msg_mesh_health_client_test_rsp_t               rsp_mesh_health_client_test;
    struct gecko_msg_mesh_health_client_get_period_cmd_t         cmd_mesh_health_client_get_period;
    struct gecko_msg_mesh_health_client_get_period_rsp_t         rsp_mesh_health_client_get_period;
    struct gecko_msg_mesh_health_client_set_period_cmd_t         cmd_mesh_health_client_set_period;
    struct gecko_msg_mesh_health_client_set_period_rsp_t         rsp_mesh_health_client_set_period;
    struct gecko_msg_mesh_health_client_get_attention_cmd_t      cmd_mesh_health_client_get_attention;
    struct gecko_msg_mesh_health_client_get_attention_rsp_t      rsp_mesh_health_client_get_attention;
    struct gecko_msg_mesh_health_client_set_attention_cmd_t      cmd_mesh_health_client_set_attention;
    struct gecko_msg_mesh_health_client_set_attention_rsp_t      rsp_mesh_health_client_set_attention;
    struct gecko_msg_mesh_health_client_server_status_evt_t      evt_mesh_health_client_server_status;
    struct gecko_msg_mesh_health_client_server_status_period_evt_t evt_mesh_health_client_server_status_period;
    struct gecko_msg_mesh_health_client_server_status_attention_evt_t evt_mesh_health_client_server_status_attention;
    struct gecko_msg_mesh_health_server_set_fault_cmd_t          cmd_mesh_health_server_set_fault;
    struct gecko_msg_mesh_health_server_set_fault_rsp_t          rsp_mesh_health_server_set_fault;
    struct gecko_msg_mesh_health_server_clear_fault_cmd_t        cmd_mesh_health_server_clear_fault;
    struct gecko_msg_mesh_health_server_clear_fault_rsp_t        rsp_mesh_health_server_clear_fault;
    struct gecko_msg_mesh_health_server_test_response_cmd_t      cmd_mesh_health_server_test_response;
    struct gecko_msg_mesh_health_server_test_response_rsp_t      rsp_mesh_health_server_test_response;
    struct gecko_msg_mesh_health_server_attention_evt_t          evt_mesh_health_server_attention;
    struct gecko_msg_mesh_health_server_test_request_evt_t       evt_mesh_health_server_test_request;
    struct gecko_msg_mesh_generic_client_get_cmd_t               cmd_mesh_generic_client_get;
    struct gecko_msg_mesh_generic_client_get_rsp_t               rsp_mesh_generic_client_get;
    struct gecko_msg_mesh_generic_client_set_cmd_t               cmd_mesh_generic_client_set;
    struct gecko_msg_mesh_generic_client_set_rsp_t               rsp_mesh_generic_client_set;
    struct gecko_msg_mesh_generic_client_publish_cmd_t           cmd_mesh_generic_client_publish;
    struct gecko_msg_mesh_generic_client_publish_rsp_t           rsp_mesh_generic_client_publish;
    struct gecko_msg_mesh_generic_client_get_params_cmd_t        cmd_mesh_generic_client_get_params;
    struct gecko_msg_mesh_generic_client_get_params_rsp_t        rsp_mesh_generic_client_get_params;
    struct gecko_msg_mesh_generic_client_init_rsp_t              rsp_mesh_generic_client_init;
    struct gecko_msg_mesh_generic_client_server_status_evt_t     evt_mesh_generic_client_server_status;
    struct gecko_msg_mesh_generic_server_response_cmd_t          cmd_mesh_generic_server_response;
    struct gecko_msg_mesh_generic_server_response_rsp_t          rsp_mesh_generic_server_response;
    struct gecko_msg_mesh_generic_server_update_cmd_t            cmd_mesh_generic_server_update;
    struct gecko_msg_mesh_generic_server_update_rsp_t            rsp_mesh_generic_server_update;
    struct gecko_msg_mesh_generic_server_publish_cmd_t           cmd_mesh_generic_server_publish;
    struct gecko_msg_mesh_generic_server_publish_rsp_t           rsp_mesh_generic_server_publish;
    struct gecko_msg_mesh_generic_server_init_rsp_t              rsp_mesh_generic_server_init;
    struct gecko_msg_mesh_generic_server_client_request_evt_t    evt_mesh_generic_server_client_request;
    struct gecko_msg_mesh_generic_server_state_changed_evt_t     evt_mesh_generic_server_state_changed;
    struct gecko_msg_coex_set_options_cmd_t                      cmd_coex_set_options;
    struct gecko_msg_coex_set_options_rsp_t                      rsp_coex_set_options;
    struct gecko_msg_coex_get_counters_cmd_t                     cmd_coex_get_counters;
    struct gecko_msg_coex_get_counters_rsp_t                     rsp_coex_get_counters;
    struct gecko_msg_mesh_test_get_nettx_rsp_t                   rsp_mesh_test_get_nettx;
    struct gecko_msg_mesh_test_set_nettx_cmd_t                   cmd_mesh_test_set_nettx;
    struct gecko_msg_mesh_test_set_nettx_rsp_t                   rsp_mesh_test_set_nettx;
    struct gecko_msg_mesh_test_get_relay_rsp_t                   rsp_mesh_test_get_relay;
    struct gecko_msg_mesh_test_set_relay_cmd_t                   cmd_mesh_test_set_relay;
    struct gecko_msg_mesh_test_set_relay_rsp_t                   rsp_mesh_test_set_relay;
    struct gecko_msg_mesh_test_set_adv_scan_params_cmd_t         cmd_mesh_test_set_adv_scan_params;
    struct gecko_msg_mesh_test_set_adv_scan_params_rsp_t         rsp_mesh_test_set_adv_scan_params;
    struct gecko_msg_mesh_test_set_ivupdate_test_mode_cmd_t      cmd_mesh_test_set_ivupdate_test_mode;
    struct gecko_msg_mesh_test_set_ivupdate_test_mode_rsp_t      rsp_mesh_test_set_ivupdate_test_mode;
    struct gecko_msg_mesh_test_get_ivupdate_test_mode_rsp_t      rsp_mesh_test_get_ivupdate_test_mode;
    struct gecko_msg_mesh_test_set_segment_send_delay_cmd_t      cmd_mesh_test_set_segment_send_delay;
    struct gecko_msg_mesh_test_set_segment_send_delay_rsp_t      rsp_mesh_test_set_segment_send_delay;
    struct gecko_msg_mesh_test_set_ivupdate_state_cmd_t          cmd_mesh_test_set_ivupdate_state;
    struct gecko_msg_mesh_test_set_ivupdate_state_rsp_t          rsp_mesh_test_set_ivupdate_state;
    struct gecko_msg_mesh_test_send_beacons_rsp_t                rsp_mesh_test_send_beacons;
    struct gecko_msg_mesh_test_bind_local_model_app_cmd_t        cmd_mesh_test_bind_local_model_app;
    struct gecko_msg_mesh_test_bind_local_model_app_rsp_t        rsp_mesh_test_bind_local_model_app;
    struct gecko_msg_mesh_test_unbind_local_model_app_cmd_t      cmd_mesh_test_unbind_local_model_app;
    struct gecko_msg_mesh_test_unbind_local_model_app_rsp_t      rsp_mesh_test_unbind_local_model_app;
    struct gecko_msg_mesh_test_add_local_model_sub_cmd_t         cmd_mesh_test_add_local_model_sub;
    struct gecko_msg_mesh_test_add_local_model_sub_rsp_t         rsp_mesh_test_add_local_model_sub;
    struct gecko_msg_mesh_test_del_local_model_sub_cmd_t         cmd_mesh_test_del_local_model_sub;
    struct gecko_msg_mesh_test_del_local_model_sub_rsp_t         rsp_mesh_test_del_local_model_sub;
    struct gecko_msg_mesh_test_add_local_model_sub_va_cmd_t      cmd_mesh_test_add_local_model_sub_va;
    struct gecko_msg_mesh_test_add_local_model_sub_va_rsp_t      rsp_mesh_test_add_local_model_sub_va;
    struct gecko_msg_mesh_test_del_local_model_sub_va_cmd_t      cmd_mesh_test_del_local_model_sub_va;
    struct gecko_msg_mesh_test_del_local_model_sub_va_rsp_t      rsp_mesh_test_del_local_model_sub_va;
    struct gecko_msg_mesh_test_get_local_model_sub_cmd_t         cmd_mesh_test_get_local_model_sub;
    struct gecko_msg_mesh_test_get_local_model_sub_rsp_t         rsp_mesh_test_get_local_model_sub;
    struct gecko_msg_mesh_test_set_local_model_pub_cmd_t         cmd_mesh_test_set_local_model_pub;
    struct gecko_msg_mesh_test_set_local_model_pub_rsp_t         rsp_mesh_test_set_local_model_pub;
    struct gecko_msg_mesh_test_set_local_model_pub_va_cmd_t      cmd_mesh_test_set_local_model_pub_va;
    struct gecko_msg_mesh_test_set_local_model_pub_va_rsp_t      rsp_mesh_test_set_local_model_pub_va;
    struct gecko_msg_mesh_test_get_local_model_pub_cmd_t         cmd_mesh_test_get_local_model_pub;
    struct gecko_msg_mesh_test_get_local_model_pub_rsp_t         rsp_mesh_test_get_local_model_pub;
    struct gecko_msg_mesh_test_set_local_heartbeat_subscription_cmd_t cmd_mesh_test_set_local_heartbeat_subscription;
    struct gecko_msg_mesh_test_set_local_heartbeat_subscription_rsp_t rsp_mesh_test_set_local_heartbeat_subscription;
    struct gecko_msg_mesh_test_get_local_heartbeat_subscription_rsp_t rsp_mesh_test_get_local_heartbeat_subscription;
    struct gecko_msg_mesh_test_get_local_heartbeat_publication_rsp_t rsp_mesh_test_get_local_heartbeat_publication;
    struct gecko_msg_mesh_test_set_local_heartbeat_publication_cmd_t cmd_mesh_test_set_local_heartbeat_publication;
    struct gecko_msg_mesh_test_set_local_heartbeat_publication_rsp_t rsp_mesh_test_set_local_heartbeat_publication;
    struct gecko_msg_mesh_test_set_local_config_cmd_t            cmd_mesh_test_set_local_config;
    struct gecko_msg_mesh_test_set_local_config_rsp_t            rsp_mesh_test_set_local_config;
    struct gecko_msg_mesh_test_get_local_config_cmd_t            cmd_mesh_test_get_local_config;
    struct gecko_msg_mesh_test_get_local_config_rsp_t            rsp_mesh_test_get_local_config;
    struct gecko_msg_mesh_test_add_local_key_cmd_t               cmd_mesh_test_add_local_key;
    struct gecko_msg_mesh_test_add_local_key_rsp_t               rsp_mesh_test_add_local_key;
    struct gecko_msg_mesh_test_del_local_key_cmd_t               cmd_mesh_test_del_local_key;
    struct gecko_msg_mesh_test_del_local_key_rsp_t               rsp_mesh_test_del_local_key;
    struct gecko_msg_mesh_test_update_local_key_cmd_t            cmd_mesh_test_update_local_key;
    struct gecko_msg_mesh_test_update_local_key_rsp_t            rsp_mesh_test_update_local_key;
    struct gecko_msg_mesh_test_set_sar_config_cmd_t              cmd_mesh_test_set_sar_config;
    struct gecko_msg_mesh_test_set_sar_config_rsp_t              rsp_mesh_test_set_sar_config;
    struct gecko_msg_mesh_test_get_element_seqnum_cmd_t          cmd_mesh_test_get_element_seqnum;
    struct gecko_msg_mesh_test_get_element_seqnum_rsp_t          rsp_mesh_test_get_element_seqnum;
    struct gecko_msg_mesh_test_set_adv_bearer_state_cmd_t        cmd_mesh_test_set_adv_bearer_state;
    struct gecko_msg_mesh_test_set_adv_bearer_state_rsp_t        rsp_mesh_test_set_adv_bearer_state;
    struct gecko_msg_mesh_test_get_key_count_cmd_t               cmd_mesh_test_get_key_count;
    struct gecko_msg_mesh_test_get_key_count_rsp_t               rsp_mesh_test_get_key_count;
    struct gecko_msg_mesh_test_get_key_cmd_t                     cmd_mesh_test_get_key;
    struct gecko_msg_mesh_test_get_key_rsp_t                     rsp_mesh_test_get_key;
    struct gecko_msg_mesh_test_prov_get_device_key_cmd_t         cmd_mesh_test_prov_get_device_key;
    struct gecko_msg_mesh_test_prov_get_device_key_rsp_t         rsp_mesh_test_prov_get_device_key;
    struct gecko_msg_mesh_test_prov_prepare_key_refresh_cmd_t    cmd_mesh_test_prov_prepare_key_refresh;
    struct gecko_msg_mesh_test_prov_prepare_key_refresh_rsp_t    rsp_mesh_test_prov_prepare_key_refresh;
    struct gecko_msg_mesh_test_cancel_segmented_tx_cmd_t         cmd_mesh_test_cancel_segmented_tx;
    struct gecko_msg_mesh_test_cancel_segmented_tx_rsp_t         rsp_mesh_test_cancel_segmented_tx;
    struct gecko_msg_mesh_test_set_iv_index_cmd_t                cmd_mesh_test_set_iv_index;
    struct gecko_msg_mesh_test_set_iv_index_rsp_t                rsp_mesh_test_set_iv_index;
    struct gecko_msg_mesh_test_set_element_seqnum_cmd_t          cmd_mesh_test_set_element_seqnum;
    struct gecko_msg_mesh_test_set_element_seqnum_rsp_t          rsp_mesh_test_set_element_seqnum;
    struct gecko_msg_mesh_test_local_heartbeat_subscription_complete_evt_t evt_mesh_test_local_heartbeat_subscription_complete;
    struct gecko_msg_mesh_lpn_init_rsp_t                         rsp_mesh_lpn_init;
    struct gecko_msg_mesh_lpn_deinit_rsp_t                       rsp_mesh_lpn_deinit;
    struct gecko_msg_mesh_lpn_configure_cmd_t                    cmd_mesh_lpn_configure;
    struct gecko_msg_mesh_lpn_configure_rsp_t                    rsp_mesh_lpn_configure;
    struct gecko_msg_mesh_lpn_establish_friendship_cmd_t         cmd_mesh_lpn_establish_friendship;
    struct gecko_msg_mesh_lpn_establish_friendship_rsp_t         rsp_mesh_lpn_establish_friendship;
    struct gecko_msg_mesh_lpn_poll_rsp_t                         rsp_mesh_lpn_poll;
    struct gecko_msg_mesh_lpn_terminate_friendship_rsp_t         rsp_mesh_lpn_terminate_friendship;
    struct gecko_msg_mesh_lpn_config_cmd_t                       cmd_mesh_lpn_config;
    struct gecko_msg_mesh_lpn_config_rsp_t                       rsp_mesh_lpn_config;
    struct gecko_msg_mesh_lpn_friendship_established_evt_t       evt_mesh_lpn_friendship_established;
    struct gecko_msg_mesh_lpn_friendship_failed_evt_t            evt_mesh_lpn_friendship_failed;
    struct gecko_msg_mesh_lpn_friendship_terminated_evt_t        evt_mesh_lpn_friendship_terminated;
    struct gecko_msg_mesh_friend_init_rsp_t                      rsp_mesh_friend_init;
    struct gecko_msg_mesh_friend_deinit_rsp_t                    rsp_mesh_friend_deinit;
    struct gecko_msg_mesh_friend_friendship_established_evt_t    evt_mesh_friend_friendship_established;
    struct gecko_msg_mesh_friend_friendship_terminated_evt_t     evt_mesh_friend_friendship_terminated;
    struct gecko_msg_mesh_config_client_cancel_request_cmd_t     cmd_mesh_config_client_cancel_request;
    struct gecko_msg_mesh_config_client_cancel_request_rsp_t     rsp_mesh_config_client_cancel_request;
    struct gecko_msg_mesh_config_client_get_request_status_cmd_t cmd_mesh_config_client_get_request_status;
    struct gecko_msg_mesh_config_client_get_request_status_rsp_t rsp_mesh_config_client_get_request_status;
    struct gecko_msg_mesh_config_client_get_default_timeout_rsp_t rsp_mesh_config_client_get_default_timeout;
    struct gecko_msg_mesh_config_client_set_default_timeout_cmd_t cmd_mesh_config_client_set_default_timeout;
    struct gecko_msg_mesh_config_client_set_default_timeout_rsp_t rsp_mesh_config_client_set_default_timeout;
    struct gecko_msg_mesh_config_client_add_netkey_cmd_t         cmd_mesh_config_client_add_netkey;
    struct gecko_msg_mesh_config_client_add_netkey_rsp_t         rsp_mesh_config_client_add_netkey;
    struct gecko_msg_mesh_config_client_remove_netkey_cmd_t      cmd_mesh_config_client_remove_netkey;
    struct gecko_msg_mesh_config_client_remove_netkey_rsp_t      rsp_mesh_config_client_remove_netkey;
    struct gecko_msg_mesh_config_client_list_netkeys_cmd_t       cmd_mesh_config_client_list_netkeys;
    struct gecko_msg_mesh_config_client_list_netkeys_rsp_t       rsp_mesh_config_client_list_netkeys;
    struct gecko_msg_mesh_config_client_add_appkey_cmd_t         cmd_mesh_config_client_add_appkey;
    struct gecko_msg_mesh_config_client_add_appkey_rsp_t         rsp_mesh_config_client_add_appkey;
    struct gecko_msg_mesh_config_client_remove_appkey_cmd_t      cmd_mesh_config_client_remove_appkey;
    struct gecko_msg_mesh_config_client_remove_appkey_rsp_t      rsp_mesh_config_client_remove_appkey;
    struct gecko_msg_mesh_config_client_list_appkeys_cmd_t       cmd_mesh_config_client_list_appkeys;
    struct gecko_msg_mesh_config_client_list_appkeys_rsp_t       rsp_mesh_config_client_list_appkeys;
    struct gecko_msg_mesh_config_client_bind_model_cmd_t         cmd_mesh_config_client_bind_model;
    struct gecko_msg_mesh_config_client_bind_model_rsp_t         rsp_mesh_config_client_bind_model;
    struct gecko_msg_mesh_config_client_unbind_model_cmd_t       cmd_mesh_config_client_unbind_model;
    struct gecko_msg_mesh_config_client_unbind_model_rsp_t       rsp_mesh_config_client_unbind_model;
    struct gecko_msg_mesh_config_client_list_bindings_cmd_t      cmd_mesh_config_client_list_bindings;
    struct gecko_msg_mesh_config_client_list_bindings_rsp_t      rsp_mesh_config_client_list_bindings;
    struct gecko_msg_mesh_config_client_get_model_pub_cmd_t      cmd_mesh_config_client_get_model_pub;
    struct gecko_msg_mesh_config_client_get_model_pub_rsp_t      rsp_mesh_config_client_get_model_pub;
    struct gecko_msg_mesh_config_client_set_model_pub_cmd_t      cmd_mesh_config_client_set_model_pub;
    struct gecko_msg_mesh_config_client_set_model_pub_rsp_t      rsp_mesh_config_client_set_model_pub;
    struct gecko_msg_mesh_config_client_set_model_pub_va_cmd_t   cmd_mesh_config_client_set_model_pub_va;
    struct gecko_msg_mesh_config_client_set_model_pub_va_rsp_t   rsp_mesh_config_client_set_model_pub_va;
    struct gecko_msg_mesh_config_client_add_model_sub_cmd_t      cmd_mesh_config_client_add_model_sub;
    struct gecko_msg_mesh_config_client_add_model_sub_rsp_t      rsp_mesh_config_client_add_model_sub;
    struct gecko_msg_mesh_config_client_add_model_sub_va_cmd_t   cmd_mesh_config_client_add_model_sub_va;
    struct gecko_msg_mesh_config_client_add_model_sub_va_rsp_t   rsp_mesh_config_client_add_model_sub_va;
    struct gecko_msg_mesh_config_client_remove_model_sub_cmd_t   cmd_mesh_config_client_remove_model_sub;
    struct gecko_msg_mesh_config_client_remove_model_sub_rsp_t   rsp_mesh_config_client_remove_model_sub;
    struct gecko_msg_mesh_config_client_remove_model_sub_va_cmd_t cmd_mesh_config_client_remove_model_sub_va;
    struct gecko_msg_mesh_config_client_remove_model_sub_va_rsp_t rsp_mesh_config_client_remove_model_sub_va;
    struct gecko_msg_mesh_config_client_set_model_sub_cmd_t      cmd_mesh_config_client_set_model_sub;
    struct gecko_msg_mesh_config_client_set_model_sub_rsp_t      rsp_mesh_config_client_set_model_sub;
    struct gecko_msg_mesh_config_client_set_model_sub_va_cmd_t   cmd_mesh_config_client_set_model_sub_va;
    struct gecko_msg_mesh_config_client_set_model_sub_va_rsp_t   rsp_mesh_config_client_set_model_sub_va;
    struct gecko_msg_mesh_config_client_clear_model_sub_cmd_t    cmd_mesh_config_client_clear_model_sub;
    struct gecko_msg_mesh_config_client_clear_model_sub_rsp_t    rsp_mesh_config_client_clear_model_sub;
    struct gecko_msg_mesh_config_client_list_subs_cmd_t          cmd_mesh_config_client_list_subs;
    struct gecko_msg_mesh_config_client_list_subs_rsp_t          rsp_mesh_config_client_list_subs;
    struct gecko_msg_mesh_config_client_get_heartbeat_pub_cmd_t  cmd_mesh_config_client_get_heartbeat_pub;
    struct gecko_msg_mesh_config_client_get_heartbeat_pub_rsp_t  rsp_mesh_config_client_get_heartbeat_pub;
    struct gecko_msg_mesh_config_client_set_heartbeat_pub_cmd_t  cmd_mesh_config_client_set_heartbeat_pub;
    struct gecko_msg_mesh_config_client_set_heartbeat_pub_rsp_t  rsp_mesh_config_client_set_heartbeat_pub;
    struct gecko_msg_mesh_config_client_get_heartbeat_sub_cmd_t  cmd_mesh_config_client_get_heartbeat_sub;
    struct gecko_msg_mesh_config_client_get_heartbeat_sub_rsp_t  rsp_mesh_config_client_get_heartbeat_sub;
    struct gecko_msg_mesh_config_client_set_heartbeat_sub_cmd_t  cmd_mesh_config_client_set_heartbeat_sub;
    struct gecko_msg_mesh_config_client_set_heartbeat_sub_rsp_t  rsp_mesh_config_client_set_heartbeat_sub;
    struct gecko_msg_mesh_config_client_get_beacon_cmd_t         cmd_mesh_config_client_get_beacon;
    struct gecko_msg_mesh_config_client_get_beacon_rsp_t         rsp_mesh_config_client_get_beacon;
    struct gecko_msg_mesh_config_client_set_beacon_cmd_t         cmd_mesh_config_client_set_beacon;
    struct gecko_msg_mesh_config_client_set_beacon_rsp_t         rsp_mesh_config_client_set_beacon;
    struct gecko_msg_mesh_config_client_get_default_ttl_cmd_t    cmd_mesh_config_client_get_default_ttl;
    struct gecko_msg_mesh_config_client_get_default_ttl_rsp_t    rsp_mesh_config_client_get_default_ttl;
    struct gecko_msg_mesh_config_client_set_default_ttl_cmd_t    cmd_mesh_config_client_set_default_ttl;
    struct gecko_msg_mesh_config_client_set_default_ttl_rsp_t    rsp_mesh_config_client_set_default_ttl;
    struct gecko_msg_mesh_config_client_get_gatt_proxy_cmd_t     cmd_mesh_config_client_get_gatt_proxy;
    struct gecko_msg_mesh_config_client_get_gatt_proxy_rsp_t     rsp_mesh_config_client_get_gatt_proxy;
    struct gecko_msg_mesh_config_client_set_gatt_proxy_cmd_t     cmd_mesh_config_client_set_gatt_proxy;
    struct gecko_msg_mesh_config_client_set_gatt_proxy_rsp_t     rsp_mesh_config_client_set_gatt_proxy;
    struct gecko_msg_mesh_config_client_get_relay_cmd_t          cmd_mesh_config_client_get_relay;
    struct gecko_msg_mesh_config_client_get_relay_rsp_t          rsp_mesh_config_client_get_relay;
    struct gecko_msg_mesh_config_client_set_relay_cmd_t          cmd_mesh_config_client_set_relay;
    struct gecko_msg_mesh_config_client_set_relay_rsp_t          rsp_mesh_config_client_set_relay;
    struct gecko_msg_mesh_config_client_get_network_transmit_cmd_t cmd_mesh_config_client_get_network_transmit;
    struct gecko_msg_mesh_config_client_get_network_transmit_rsp_t rsp_mesh_config_client_get_network_transmit;
    struct gecko_msg_mesh_config_client_set_network_transmit_cmd_t cmd_mesh_config_client_set_network_transmit;
    struct gecko_msg_mesh_config_client_set_network_transmit_rsp_t rsp_mesh_config_client_set_network_transmit;
    struct gecko_msg_mesh_config_client_get_identity_cmd_t       cmd_mesh_config_client_get_identity;
    struct gecko_msg_mesh_config_client_get_identity_rsp_t       rsp_mesh_config_client_get_identity;
    struct gecko_msg_mesh_config_client_set_identity_cmd_t       cmd_mesh_config_client_set_identity;
    struct gecko_msg_mesh_config_client_set_identity_rsp_t       rsp_mesh_config_client_set_identity;
    struct gecko_msg_mesh_config_client_get_friend_cmd_t         cmd_mesh_config_client_get_friend;
    struct gecko_msg_mesh_config_client_get_friend_rsp_t         rsp_mesh_config_client_get_friend;
    struct gecko_msg_mesh_config_client_set_friend_cmd_t         cmd_mesh_config_client_set_friend;
    struct gecko_msg_mesh_config_client_set_friend_rsp_t         rsp_mesh_config_client_set_friend;
    struct gecko_msg_mesh_config_client_get_lpn_polltimeout_cmd_t cmd_mesh_config_client_get_lpn_polltimeout;
    struct gecko_msg_mesh_config_client_get_lpn_polltimeout_rsp_t rsp_mesh_config_client_get_lpn_polltimeout;
    struct gecko_msg_mesh_config_client_get_dcd_cmd_t            cmd_mesh_config_client_get_dcd;
    struct gecko_msg_mesh_config_client_get_dcd_rsp_t            rsp_mesh_config_client_get_dcd;
    struct gecko_msg_mesh_config_client_reset_node_cmd_t         cmd_mesh_config_client_reset_node;
    struct gecko_msg_mesh_config_client_reset_node_rsp_t         rsp_mesh_config_client_reset_node;
    struct gecko_msg_mesh_config_client_request_modified_evt_t   evt_mesh_config_client_request_modified;
    struct gecko_msg_mesh_config_client_netkey_status_evt_t      evt_mesh_config_client_netkey_status;
    struct gecko_msg_mesh_config_client_netkey_list_evt_t        evt_mesh_config_client_netkey_list;
    struct gecko_msg_mesh_config_client_netkey_list_end_evt_t    evt_mesh_config_client_netkey_list_end;
    struct gecko_msg_mesh_config_client_appkey_status_evt_t      evt_mesh_config_client_appkey_status;
    struct gecko_msg_mesh_config_client_appkey_list_evt_t        evt_mesh_config_client_appkey_list;
    struct gecko_msg_mesh_config_client_appkey_list_end_evt_t    evt_mesh_config_client_appkey_list_end;
    struct gecko_msg_mesh_config_client_binding_status_evt_t     evt_mesh_config_client_binding_status;
    struct gecko_msg_mesh_config_client_bindings_list_evt_t      evt_mesh_config_client_bindings_list;
    struct gecko_msg_mesh_config_client_bindings_list_end_evt_t  evt_mesh_config_client_bindings_list_end;
    struct gecko_msg_mesh_config_client_model_pub_status_evt_t   evt_mesh_config_client_model_pub_status;
    struct gecko_msg_mesh_config_client_model_sub_status_evt_t   evt_mesh_config_client_model_sub_status;
    struct gecko_msg_mesh_config_client_subs_list_evt_t          evt_mesh_config_client_subs_list;
    struct gecko_msg_mesh_config_client_subs_list_end_evt_t      evt_mesh_config_client_subs_list_end;
    struct gecko_msg_mesh_config_client_heartbeat_pub_status_evt_t evt_mesh_config_client_heartbeat_pub_status;
    struct gecko_msg_mesh_config_client_heartbeat_sub_status_evt_t evt_mesh_config_client_heartbeat_sub_status;
    struct gecko_msg_mesh_config_client_beacon_status_evt_t      evt_mesh_config_client_beacon_status;
    struct gecko_msg_mesh_config_client_default_ttl_status_evt_t evt_mesh_config_client_default_ttl_status;
    struct gecko_msg_mesh_config_client_gatt_proxy_status_evt_t  evt_mesh_config_client_gatt_proxy_status;
    struct gecko_msg_mesh_config_client_relay_status_evt_t       evt_mesh_config_client_relay_status;
    struct gecko_msg_mesh_config_client_network_transmit_status_evt_t evt_mesh_config_client_network_transmit_status;
    struct gecko_msg_mesh_config_client_identity_status_evt_t    evt_mesh_config_client_identity_status;
    struct gecko_msg_mesh_config_client_friend_status_evt_t      evt_mesh_config_client_friend_status;
    struct gecko_msg_mesh_config_client_lpn_polltimeout_status_evt_t evt_mesh_config_client_lpn_polltimeout_status;
    struct gecko_msg_mesh_config_client_dcd_data_evt_t           evt_mesh_config_client_dcd_data;
    struct gecko_msg_mesh_config_client_dcd_data_end_evt_t       evt_mesh_config_client_dcd_data_end;
    struct gecko_msg_mesh_config_client_reset_status_evt_t       evt_mesh_config_client_reset_status;
    struct gecko_msg_l2cap_coc_send_connection_request_cmd_t     cmd_l2cap_coc_send_connection_request;
    struct gecko_msg_l2cap_coc_send_connection_request_rsp_t     rsp_l2cap_coc_send_connection_request;
    struct gecko_msg_l2cap_coc_send_connection_response_cmd_t    cmd_l2cap_coc_send_connection_response;
    struct gecko_msg_l2cap_coc_send_connection_response_rsp_t    rsp_l2cap_coc_send_connection_response;
    struct gecko_msg_l2cap_coc_send_le_flow_control_credit_cmd_t cmd_l2cap_coc_send_le_flow_control_credit;
    struct gecko_msg_l2cap_coc_send_le_flow_control_credit_rsp_t rsp_l2cap_coc_send_le_flow_control_credit;
    struct gecko_msg_l2cap_coc_send_disconnection_request_cmd_t  cmd_l2cap_coc_send_disconnection_request;
    struct gecko_msg_l2cap_coc_send_disconnection_request_rsp_t  rsp_l2cap_coc_send_disconnection_request;
    struct gecko_msg_l2cap_coc_send_data_cmd_t                   cmd_l2cap_coc_send_data;
    struct gecko_msg_l2cap_coc_send_data_rsp_t                   rsp_l2cap_coc_send_data;
    struct gecko_msg_l2cap_coc_connection_request_evt_t          evt_l2cap_coc_connection_request;
    struct gecko_msg_l2cap_coc_connection_response_evt_t         evt_l2cap_coc_connection_response;
    struct gecko_msg_l2cap_coc_le_flow_control_credit_evt_t      evt_l2cap_coc_le_flow_control_credit;
    struct gecko_msg_l2cap_coc_channel_disconnected_evt_t        evt_l2cap_coc_channel_disconnected;
    struct gecko_msg_l2cap_coc_data_evt_t                        evt_l2cap_coc_data;
    struct gecko_msg_l2cap_command_rejected_evt_t                evt_l2cap_command_rejected;
    struct gecko_msg_cte_transmitter_enable_cte_response_cmd_t   cmd_cte_transmitter_enable_cte_response;
    struct gecko_msg_cte_transmitter_enable_cte_response_rsp_t   rsp_cte_transmitter_enable_cte_response;
    struct gecko_msg_cte_transmitter_disable_cte_response_cmd_t  cmd_cte_transmitter_disable_cte_response;
    struct gecko_msg_cte_transmitter_disable_cte_response_rsp_t  rsp_cte_transmitter_disable_cte_response;
    struct gecko_msg_cte_transmitter_start_connectionless_cte_cmd_t cmd_cte_transmitter_start_connectionless_cte;
    struct gecko_msg_cte_transmitter_start_connectionless_cte_rsp_t rsp_cte_transmitter_start_connectionless_cte;
    struct gecko_msg_cte_transmitter_stop_connectionless_cte_cmd_t cmd_cte_transmitter_stop_connectionless_cte;
    struct gecko_msg_cte_transmitter_stop_connectionless_cte_rsp_t rsp_cte_transmitter_stop_connectionless_cte;
    struct gecko_msg_cte_transmitter_set_dtm_parameters_cmd_t    cmd_cte_transmitter_set_dtm_parameters;
    struct gecko_msg_cte_transmitter_set_dtm_parameters_rsp_t    rsp_cte_transmitter_set_dtm_parameters;
    struct gecko_msg_cte_transmitter_clear_dtm_parameters_rsp_t  rsp_cte_transmitter_clear_dtm_parameters;
    struct gecko_msg_cte_receiver_configure_cmd_t                cmd_cte_receiver_configure;
    struct gecko_msg_cte_receiver_configure_rsp_t                rsp_cte_receiver_configure;
    struct gecko_msg_cte_receiver_start_iq_sampling_cmd_t        cmd_cte_receiver_start_iq_sampling;
    struct gecko_msg_cte_receiver_start_iq_sampling_rsp_t        rsp_cte_receiver_start_iq_sampling;
    struct gecko_msg_cte_receiver_stop_iq_sampling_cmd_t         cmd_cte_receiver_stop_iq_sampling;
    struct gecko_msg_cte_receiver_stop_iq_sampling_rsp_t         rsp_cte_receiver_stop_iq_sampling;
    struct gecko_msg_cte_receiver_start_connectionless_iq_sampling_cmd_t cmd_cte_receiver_start_connectionless_iq_sampling;
    struct gecko_msg_cte_receiver_start_connectionless_iq_sampling_rsp_t rsp_cte_receiver_start_connectionless_iq_sampling;
    struct gecko_msg_cte_receiver_stop_connectionless_iq_sampling_cmd_t cmd_cte_receiver_stop_connectionless_iq_sampling;
    struct gecko_msg_cte_receiver_stop_connectionless_iq_sampling_rsp_t rsp_cte_receiver_stop_connectionless_iq_sampling;
    struct gecko_msg_cte_receiver_set_dtm_parameters_cmd_t       cmd_cte_receiver_set_dtm_parameters;
    struct gecko_msg_cte_receiver_set_dtm_parameters_rsp_t       rsp_cte_receiver_set_dtm_parameters;
    struct gecko_msg_cte_receiver_clear_dtm_parameters_rsp_t     rsp_cte_receiver_clear_dtm_parameters;
    struct gecko_msg_cte_receiver_iq_report_evt_t                evt_cte_receiver_iq_report;
    struct gecko_msg_mesh_sensor_server_init_cmd_t               cmd_mesh_sensor_server_init;
    struct gecko_msg_mesh_sensor_server_init_rsp_t               rsp_mesh_sensor_server_init;
    struct gecko_msg_mesh_sensor_server_deinit_cmd_t             cmd_mesh_sensor_server_deinit;
    struct gecko_msg_mesh_sensor_server_deinit_rsp_t             rsp_mesh_sensor_server_deinit;
    struct gecko_msg_mesh_sensor_server_send_descriptor_status_cmd_t cmd_mesh_sensor_server_send_descriptor_status;
    struct gecko_msg_mesh_sensor_server_send_descriptor_status_rsp_t rsp_mesh_sensor_server_send_descriptor_status;
    struct gecko_msg_mesh_sensor_server_send_status_cmd_t        cmd_mesh_sensor_server_send_status;
    struct gecko_msg_mesh_sensor_server_send_status_rsp_t        rsp_mesh_sensor_server_send_status;
    struct gecko_msg_mesh_sensor_server_send_column_status_cmd_t cmd_mesh_sensor_server_send_column_status;
    struct gecko_msg_mesh_sensor_server_send_column_status_rsp_t rsp_mesh_sensor_server_send_column_status;
    struct gecko_msg_mesh_sensor_server_send_series_status_cmd_t cmd_mesh_sensor_server_send_series_status;
    struct gecko_msg_mesh_sensor_server_send_series_status_rsp_t rsp_mesh_sensor_server_send_series_status;
    struct gecko_msg_mesh_sensor_server_get_request_evt_t        evt_mesh_sensor_server_get_request;
    struct gecko_msg_mesh_sensor_server_get_column_request_evt_t evt_mesh_sensor_server_get_column_request;
    struct gecko_msg_mesh_sensor_server_get_series_request_evt_t evt_mesh_sensor_server_get_series_request;
    struct gecko_msg_mesh_sensor_server_publish_evt_t            evt_mesh_sensor_server_publish;
    struct gecko_msg_mesh_sensor_setup_server_send_cadence_status_cmd_t cmd_mesh_sensor_setup_server_send_cadence_status;
    struct gecko_msg_mesh_sensor_setup_server_send_cadence_status_rsp_t rsp_mesh_sensor_setup_server_send_cadence_status;
    struct gecko_msg_mesh_sensor_setup_server_send_settings_status_cmd_t cmd_mesh_sensor_setup_server_send_settings_status;
    struct gecko_msg_mesh_sensor_setup_server_send_settings_status_rsp_t rsp_mesh_sensor_setup_server_send_settings_status;
    struct gecko_msg_mesh_sensor_setup_server_send_setting_status_cmd_t cmd_mesh_sensor_setup_server_send_setting_status;
    struct gecko_msg_mesh_sensor_setup_server_send_setting_status_rsp_t rsp_mesh_sensor_setup_server_send_setting_status;
    struct gecko_msg_mesh_sensor_setup_server_get_cadence_request_evt_t evt_mesh_sensor_setup_server_get_cadence_request;
    struct gecko_msg_mesh_sensor_setup_server_set_cadence_request_evt_t evt_mesh_sensor_setup_server_set_cadence_request;
    struct gecko_msg_mesh_sensor_setup_server_get_settings_request_evt_t evt_mesh_sensor_setup_server_get_settings_request;
    struct gecko_msg_mesh_sensor_setup_server_get_setting_request_evt_t evt_mesh_sensor_setup_server_get_setting_request;
    struct gecko_msg_mesh_sensor_setup_server_set_setting_request_evt_t evt_mesh_sensor_setup_server_set_setting_request;
    struct gecko_msg_mesh_sensor_setup_server_publish_evt_t      evt_mesh_sensor_setup_server_publish;
    struct gecko_msg_mesh_sensor_client_init_rsp_t               rsp_mesh_sensor_client_init;
    struct gecko_msg_mesh_sensor_client_deinit_rsp_t             rsp_mesh_sensor_client_deinit;
    struct gecko_msg_mesh_sensor_client_get_descriptor_cmd_t     cmd_mesh_sensor_client_get_descriptor;
    struct gecko_msg_mesh_sensor_client_get_descriptor_rsp_t     rsp_mesh_sensor_client_get_descriptor;
    struct gecko_msg_mesh_sensor_client_get_cmd_t                cmd_mesh_sensor_client_get;
    struct gecko_msg_mesh_sensor_client_get_rsp_t                rsp_mesh_sensor_client_get;
    struct gecko_msg_mesh_sensor_client_get_column_cmd_t         cmd_mesh_sensor_client_get_column;
    struct gecko_msg_mesh_sensor_client_get_column_rsp_t         rsp_mesh_sensor_client_get_column;
    struct gecko_msg_mesh_sensor_client_get_series_cmd_t         cmd_mesh_sensor_client_get_series;
    struct gecko_msg_mesh_sensor_client_get_series_rsp_t         rsp_mesh_sensor_client_get_series;
    struct gecko_msg_mesh_sensor_client_get_cadence_cmd_t        cmd_mesh_sensor_client_get_cadence;
    struct gecko_msg_mesh_sensor_client_get_cadence_rsp_t        rsp_mesh_sensor_client_get_cadence;
    struct gecko_msg_mesh_sensor_client_set_cadence_cmd_t        cmd_mesh_sensor_client_set_cadence;
    struct gecko_msg_mesh_sensor_client_set_cadence_rsp_t        rsp_mesh_sensor_client_set_cadence;
    struct gecko_msg_mesh_sensor_client_get_settings_cmd_t       cmd_mesh_sensor_client_get_settings;
    struct gecko_msg_mesh_sensor_client_get_settings_rsp_t       rsp_mesh_sensor_client_get_settings;
    struct gecko_msg_mesh_sensor_client_get_setting_cmd_t        cmd_mesh_sensor_client_get_setting;
    struct gecko_msg_mesh_sensor_client_get_setting_rsp_t        rsp_mesh_sensor_client_get_setting;
    struct gecko_msg_mesh_sensor_client_set_setting_cmd_t        cmd_mesh_sensor_client_set_setting;
    struct gecko_msg_mesh_sensor_client_set_setting_rsp_t        rsp_mesh_sensor_client_set_setting;
    struct gecko_msg_mesh_sensor_client_descriptor_status_evt_t  evt_mesh_sensor_client_descriptor_status;
    struct gecko_msg_mesh_sensor_client_cadence_status_evt_t     evt_mesh_sensor_client_cadence_status;
    struct gecko_msg_mesh_sensor_client_settings_status_evt_t    evt_mesh_sensor_client_settings_status;
    struct gecko_msg_mesh_sensor_client_setting_status_evt_t     evt_mesh_sensor_client_setting_status;
    struct gecko_msg_mesh_sensor_client_status_evt_t             evt_mesh_sensor_client_status;
    struct gecko_msg_mesh_sensor_client_column_status_evt_t      evt_mesh_sensor_client_column_status;
    struct gecko_msg_mesh_sensor_client_series_status_evt_t      evt_mesh_sensor_client_series_status;
    struct gecko_msg_mesh_sensor_client_publish_evt_t            evt_mesh_sensor_client_publish;
    struct gecko_msg_user_message_to_target_cmd_t                cmd_user_message_to_target;
    struct gecko_msg_user_message_to_target_rsp_t                rsp_user_message_to_target;
    struct gecko_msg_user_message_to_host_evt_t                  evt_user_message_to_host;

    uint8 payload[BGLIB_MSG_MAX_PAYLOAD];
}data;

});


void gecko_handle_command(uint32_t,void*);
void gecko_handle_command_noresponse(uint32_t,void*);

extern struct gecko_cmd_packet*  gecko_cmd_msg;
extern struct gecko_cmd_packet*  gecko_rsp_msg;

/** 
*
* gecko_cmd_dfu_reset
*
* This command can be used to reset the system. This command does not have a response, but it triggers one of the boot events (normal reset or boot to DFU mode) after re-boot.  
*
* @param dfu   Boot mode:                     
*  - 0: Normal reset
*  - 1: Boot to UART DFU mode
*  - 2: Boot to OTA DFU mode
* 
*
* Events generated
*
* gecko_evt_system_boot - Sent after the device has booted into normal mode
* gecko_evt_dfu_boot - Sent after the device has booted into UART DFU mode    
*
**/

static inline void* gecko_cmd_dfu_reset(uint8 dfu)
{
    
    gecko_cmd_msg->data.cmd_dfu_reset.dfu=dfu;
    gecko_cmd_msg->header=((gecko_cmd_dfu_reset_id+((1)<<8)));
    
    gecko_handle_command_noresponse(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    return 0;
}

/** 
*
* gecko_cmd_dfu_flash_set_address
*
* After re-booting the local device into DFU mode, this command can be used to define the starting address on the flash to where the new firmware will be written in. 
*
* @param address   The offset in the flash where the new firmware is uploaded to. Always use the value 0x00000000.    
*
**/

static inline struct gecko_msg_dfu_flash_set_address_rsp_t* gecko_cmd_dfu_flash_set_address(uint32 address)
{
    
    gecko_cmd_msg->data.cmd_dfu_flash_set_address.address=address;
    gecko_cmd_msg->header=((gecko_cmd_dfu_flash_set_address_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_dfu_flash_set_address;
}

/** 
*
* gecko_cmd_dfu_flash_upload
*
* This command can be used to upload the whole firmware image file into the Bluetooth device. The passed data length must be a multiple of 4 bytes. As the BGAPI command payload size is limited, multiple commands need to be issued one after the other until the whole .bin firmware image file is uploaded to the device. The next address of the flash sector in memory to write to is automatically updated by the bootloader after each individual command. 
*
* @param data   An array of data which will be written onto the flash.    
*
**/

static inline struct gecko_msg_dfu_flash_upload_rsp_t* gecko_cmd_dfu_flash_upload(uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_dfu_flash_upload.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_dfu_flash_upload;
    }

    
    gecko_cmd_msg->data.cmd_dfu_flash_upload.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_dfu_flash_upload.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_dfu_flash_upload_id+((1+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_dfu_flash_upload;
}

/** 
*
* gecko_cmd_dfu_flash_upload_finish
*
* This command can be used to tell to the device that the DFU file has been fully uploaded. To return the device back to normal mode the command "DFU Reset " must be issued next. 
*    
*
**/

static inline struct gecko_msg_dfu_flash_upload_finish_rsp_t* gecko_cmd_dfu_flash_upload_finish()
{
    
    gecko_cmd_msg->header=((gecko_cmd_dfu_flash_upload_finish_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_dfu_flash_upload_finish;
}

/** 
*
* gecko_cmd_system_hello
*
* This command does not trigger any event but the response to the command is used to verify that communication between the host and the device is working. 
*    
*
**/

static inline struct gecko_msg_system_hello_rsp_t* gecko_cmd_system_hello()
{
    
    gecko_cmd_msg->header=((gecko_cmd_system_hello_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_hello;
}

/** 
*
* gecko_cmd_system_reset
*
* This command can be used to reset the system. It does not have a response, but it triggers one of the boot events (normal reset or boot to DFU mode) depending on the selected boot mode. 
*
* @param dfu   Boot mode:      
*  - 0: Normal reset
*  - 1: Boot to UART DFU mode
*  - 2: Boot to OTA DFU mode
* 
*
* Events generated
*
* gecko_evt_system_boot - Sent after the device has booted into normal mode
* gecko_evt_dfu_boot - Sent after the device has booted into UART DFU mode    
*
**/

static inline void* gecko_cmd_system_reset(uint8 dfu)
{
    
    gecko_cmd_msg->data.cmd_system_reset.dfu=dfu;
    gecko_cmd_msg->header=((gecko_cmd_system_reset_id+((1)<<8)));
    
    gecko_handle_command_noresponse(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    return 0;
}

/** 
*
* gecko_cmd_system_get_bt_address
*
* This command can be used to read the Bluetooth public address used by the device. 
*    
*
**/

static inline struct gecko_msg_system_get_bt_address_rsp_t* gecko_cmd_system_get_bt_address()
{
    
    gecko_cmd_msg->header=((gecko_cmd_system_get_bt_address_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_get_bt_address;
}

/** 
*
* gecko_cmd_system_set_bt_address
*
* Deprecated. Replacement is "system_set_identity_address" command.                 
* This command can be used to set the Bluetooth public address used by
* the device. A valid address set with this command overrides the
* default Bluetooth public address programmed at production, and it will
* be effective in the next system reboot. The stack treats
* 00:00:00:00:00:00 and ff:ff:ff:ff:ff:ff as invalid addresses. Thus
* passing one of them into this command will cause the stack to use the
* default address in the next system reboot.
*  
*
* @param address   Bluetooth public address in little endian format    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_system_set_bt_address_rsp_t* gecko_cmd_system_set_bt_address(bd_addr address)
{
    
    memcpy(&gecko_cmd_msg->data.cmd_system_set_bt_address.address,&address,sizeof(bd_addr));
    gecko_cmd_msg->header=((gecko_cmd_system_set_bt_address_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_set_bt_address;
}

/** 
*
* gecko_cmd_system_set_tx_power
*
* This command can be used to set the global maximum TX power for Bluetooth.                 The returned value in the response is the selected maximum output power level after applying RF path compensation. If the GATT server contains a Tx Power service, the Tx Power Level attribute of the service will be updated accordingly.                 
* The selected power level may be less than the specified value if the
* device does not meet the power requirements. For Bluetooth connections
* the maximum TX power will be limited to 10 dBm if Adaptive Frequency
* Hopping (AFH) is not enabled.
* By default, the global maximum TX power value is 8 dBm.
* NOTE: This command should not be used while advertising, scanning or during connection.
*  
*
* @param power   TX power in 0.1dBm steps, for example the value of 10 is 1dBm and 55 is 5.5dBm    
*
**/

static inline struct gecko_msg_system_set_tx_power_rsp_t* gecko_cmd_system_set_tx_power(int16 power)
{
    
    gecko_cmd_msg->data.cmd_system_set_tx_power.power=power;
    gecko_cmd_msg->header=((gecko_cmd_system_set_tx_power_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_set_tx_power;
}

/** 
*
* gecko_cmd_system_get_random_data
*
* This command can be used to get random data up to 16 bytes.              
*
* @param length   Length of random data. Maximum length is 16 bytes.    
*
**/

static inline struct gecko_msg_system_get_random_data_rsp_t* gecko_cmd_system_get_random_data(uint8 length)
{
    
    gecko_cmd_msg->data.cmd_system_get_random_data.length=length;
    gecko_cmd_msg->header=((gecko_cmd_system_get_random_data_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_get_random_data;
}

/** 
*
* gecko_cmd_system_halt
*
* This command forces radio to idle state and allows device to sleep. Advertising, scanning, connections and software timers are halted by this commands. Halted operations are resumed by calling this command with parameter 0. Connections stay alive if system is resumed before connection supervision timeout.                 
* This command should only be used for a short time period (a few
* seconds at maximum).                 It halts Bluetooth activity, but
* all the tasks and operations are still existing inside
* stack with their own concepts of time. Halting the system for a long
* time period may                 have negative consequences on stack's
* internal states.
* NOTE:Software timer is also halted. Hardware interrupts are the only way to wake up from                 energy mode 2 when system is halted.
*  
*
* @param halt   Values:
*  - 1: halt
*  - 0: resume
*     
*
**/

static inline struct gecko_msg_system_halt_rsp_t* gecko_cmd_system_halt(uint8 halt)
{
    
    gecko_cmd_msg->data.cmd_system_halt.halt=halt;
    gecko_cmd_msg->header=((gecko_cmd_system_halt_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_halt;
}

/** 
*
* gecko_cmd_system_set_device_name
*
* This command can be used to set the device name. Currently it is possible to set the name which will be used during the OTA update. The name will be stored in persistent storage.             If the OTA device name is also set in gecko configuration, the name stored in persistent storage is overwritten with the name in gecko configuration during device boot. 
*
* @param type   Device name to set. Values:
*  - 0: OTA device name
* 
* @param name   Device name    
*
**/

static inline struct gecko_msg_system_set_device_name_rsp_t* gecko_cmd_system_set_device_name(uint8 type,uint8 name_len, const uint8* name_data)
{
    if ((uint16_t)name_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_system_set_device_name.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_system_set_device_name;
    }

    
    gecko_cmd_msg->data.cmd_system_set_device_name.type=type;
    gecko_cmd_msg->data.cmd_system_set_device_name.name.len=name_len;
    memcpy(gecko_cmd_msg->data.cmd_system_set_device_name.name.data,name_data,name_len);
    gecko_cmd_msg->header=((gecko_cmd_system_set_device_name_id+((2+name_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_set_device_name;
}

/** 
*
* gecko_cmd_system_linklayer_configure
*
* Send configuration data to linklayer. This command is used to fine tune low level Bluetooth operation. 
*
* @param key   Key to configure:      
*  - 1:HALT, same as system_halt command, value-0 Stop Radio 1- Start Radio
*  - 2:PRIORITY_RANGE, Sets RAIL priority_mapping offset field of linklayer Priority configuration structure to the first byte of value field.
*  - 3:SCAN_CHANNELS, Sets channels to scan on. First byte of value is channel map. 0x1 = Channel 37, 0x2 = Channel 38, 0x4 = Channel 39
*  - 4:SET_FLAGS, Set Link Layer configuration flags. value is little endian 32bit integer.      Flag Values:       
*  - 0x00000001 - Disable Feature Exchange when slave
*  - 0x00000002 - Disable Feature Exchange when master
*  - 5:CLR_FLAGS, value is flags to clear. Flags are same as in SET_FLAGS command.
*  - 7:SET_AFH_INTERVAL, Set afh_scan_interval field of Link Layer priority configuration structure.
*  - 0:
* 
* @param data   Configuration data. Length and contents of data field depend on the key value used.    
*
**/

static inline struct gecko_msg_system_linklayer_configure_rsp_t* gecko_cmd_system_linklayer_configure(uint8 key,uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_system_linklayer_configure.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_system_linklayer_configure;
    }

    
    gecko_cmd_msg->data.cmd_system_linklayer_configure.key=key;
    gecko_cmd_msg->data.cmd_system_linklayer_configure.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_system_linklayer_configure.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_system_linklayer_configure_id+((2+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_linklayer_configure;
}

/** 
*
* gecko_cmd_system_get_counters
*
* Get packet and error counters 
*
* @param reset   Reset counters if parameter value is nonzero    
*
**/

static inline struct gecko_msg_system_get_counters_rsp_t* gecko_cmd_system_get_counters(uint8 reset)
{
    
    gecko_cmd_msg->data.cmd_system_get_counters.reset=reset;
    gecko_cmd_msg->header=((gecko_cmd_system_get_counters_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_get_counters;
}

/** 
*
* gecko_cmd_system_data_buffer_write
*
* This command can be used to write data into system data buffer. Data will be appended to the end of existing data. 
*
* @param data   Data to write    
*
**/

static inline struct gecko_msg_system_data_buffer_write_rsp_t* gecko_cmd_system_data_buffer_write(uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_system_data_buffer_write.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_system_data_buffer_write;
    }

    
    gecko_cmd_msg->data.cmd_system_data_buffer_write.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_system_data_buffer_write.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_system_data_buffer_write_id+((1+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_data_buffer_write;
}

/** 
*
* gecko_cmd_system_set_identity_address
*
* This command can be used to set the device's Bluetooth identity address.                 The address can be a public device address or static random address.                 A valid address set with this command overrides the default Bluetooth public                 address that was programmed at production, and it will be effective in the next system reboot.                 The stack treats 00:00:00:00:00:00 and ff:ff:ff:ff:ff:ff as invalid addresses.                 Thus passing one of them into this command will cause the stack to use the default address in next system reboot.              
*
* @param address   Bluetooth identity address in little endian format
* @param type   Address type                     
*  - 0: Public device address
*  - 1: Static random address
*     
*
**/

static inline struct gecko_msg_system_set_identity_address_rsp_t* gecko_cmd_system_set_identity_address(bd_addr address,uint8 type)
{
    
    memcpy(&gecko_cmd_msg->data.cmd_system_set_identity_address.address,&address,sizeof(bd_addr));
    gecko_cmd_msg->data.cmd_system_set_identity_address.type=type;
    gecko_cmd_msg->header=((gecko_cmd_system_set_identity_address_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_set_identity_address;
}

/** 
*
* gecko_cmd_system_data_buffer_clear
*
* This command can be used to remove all data from system data buffer. 
*    
*
**/

static inline struct gecko_msg_system_data_buffer_clear_rsp_t* gecko_cmd_system_data_buffer_clear()
{
    
    gecko_cmd_msg->header=((gecko_cmd_system_data_buffer_clear_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_system_data_buffer_clear;
}

/** 
*
* gecko_cmd_le_gap_open
*
* Deprecated. Replacement is "le_gap_connect" command which allows to open a connection with a specified PHY.
* This command can be used to connect an advertising device with
* initiating PHY being the LE 1M PHY.
*  
*
* @param address   Address of the device to connect to
* @param address_type   Address type of the device to connect to
*
* Events generated
*
* gecko_evt_le_connection_opened - This event is triggered after the connection has been opened, and indicates whether the devices are already bonded and what is the role of the Bluetooth device (Slave or Master).
* gecko_evt_le_connection_parameters - This event indicates the connection parameters and security mode of the connection.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_open_rsp_t* gecko_cmd_le_gap_open(bd_addr address,uint8 address_type)
{
    
    memcpy(&gecko_cmd_msg->data.cmd_le_gap_open.address,&address,sizeof(bd_addr));
    gecko_cmd_msg->data.cmd_le_gap_open.address_type=address_type;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_open_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_open;
}

/** 
*
* gecko_cmd_le_gap_set_mode
*
* Deprecated. Use "le_gap_start_advertising" command for enabling the advertising, and "le_gap_stop_advertising" command for disabling the advertising.
* This command is only effective on the first advertising set (handle
* value 0). Other advertising sets are not affected.
*  
*
* @param discover   Discoverable mode
* @param connect   Connectable mode
*
* Events generated
*
* gecko_evt_le_gap_adv_timeout - Triggered when the number of advertising events has been done and advertising is stopped.
* gecko_evt_le_connection_opened - Triggered when a remote device opened a connection to this advertising device.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_set_mode_rsp_t* gecko_cmd_le_gap_set_mode(uint8 discover,uint8 connect)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_mode.discover=discover;
    gecko_cmd_msg->data.cmd_le_gap_set_mode.connect=connect;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_mode_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_mode;
}

/** 
*
* gecko_cmd_le_gap_discover
*
* Deprecated. Replacement is "le_gap_start_discovery"             command. To preserve the same functionality when migrating to this new command, use LE 1M PHY in scanning_phy parameter.
* This command can be used to start the GAP discovery procedure to scan
* for advertising devices on LE 1M PHY. To cancel an ongoing
* discovery process use the "le_gap_end_procedure" command.
*  
*
* @param mode   Bluetooth discovery Mode. For values see link
*
* Events generated
*
* gecko_evt_le_gap_scan_response - Every time an advertising packet is received, this event is triggered. The packets are not filtered in any way, so multiple events will be                     received for every advertising device in range.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_discover_rsp_t* gecko_cmd_le_gap_discover(uint8 mode)
{
    
    gecko_cmd_msg->data.cmd_le_gap_discover.mode=mode;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_discover_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_discover;
}

/** 
*
* gecko_cmd_le_gap_end_procedure
*
* This command can be used to end a current GAP procedure. 
*    
*
**/

static inline struct gecko_msg_le_gap_end_procedure_rsp_t* gecko_cmd_le_gap_end_procedure()
{
    
    gecko_cmd_msg->header=((gecko_cmd_le_gap_end_procedure_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_end_procedure;
}

/** 
*
* gecko_cmd_le_gap_set_adv_parameters
*
* Deprecated. Replacements are "le_gap_set_advertise_timing" command for setting the advertising intervals, and "le_gap_set_advertise_channel_map" command for setting the channel map.
* This command is only effective on the first advertising set (handle
* value 0). Other advertising sets are not affected.
*  
*
* @param interval_min   Minimum advertising interval. Value in units of 0.625 ms
*  - Range: 0x20 to 0xFFFF
*  - Time range: 20 ms to 40.96 s
* Default value: 100 ms
* @param interval_max   Maximum advertising interval. Value in units of 0.625 ms
*  - Range: 0x20 to 0xFFFF
*  - Time range: 20 ms to 40.96 s
*  - Note: interval_max should be bigger than interval_min
* Default value: 200 ms
* @param channel_map   Advertising channel map which determines which of the three channels will be used for advertising. This value is given as a bitmask. Values:
*  - 1: Advertise on CH37
*  - 2: Advertise on CH38
*  - 3: Advertise on CH37 and CH38
*  - 4: Advertise on CH39
*  - 5: Advertise on CH37 and CH39
*  - 6: Advertise on CH38 and CH39
*  - 7: Advertise on all channels
* Recommended value: 7
* Default value: 7
*     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_set_adv_parameters_rsp_t* gecko_cmd_le_gap_set_adv_parameters(uint16 interval_min,uint16 interval_max,uint8 channel_map)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_adv_parameters.interval_min=interval_min;
    gecko_cmd_msg->data.cmd_le_gap_set_adv_parameters.interval_max=interval_max;
    gecko_cmd_msg->data.cmd_le_gap_set_adv_parameters.channel_map=channel_map;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_adv_parameters_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_adv_parameters;
}

/** 
*
* gecko_cmd_le_gap_set_conn_parameters
*
* Deprecated. Replacement is "le_gap_set_conn_timing_parameters" command for setting timing parameters.
* This command can be used to set the default Bluetooth connection
* parameters. The configured values are valid for all subsequent
* connections that will              be established. For changing the
* parameters of an already established connection use the command
* "le_connection_set_parameters".
*  
*
* @param min_interval   Minimum value for the connection event interval. This must be set be less than or equal to max_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* Default value: 20 ms                 
* @param max_interval   Maximum value for the connection event interval. This must be set greater than or equal to min_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* Default value: 50 ms                 
* @param latency   Slave latency. This parameter defines how many connection intervals the slave can skip if it has no data to send
*  - Range: 0x0000 to 0x01f4
* Default value: 0                 
* @param timeout   Supervision timeout. The supervision timeout defines for how long the connection is maintained despite the devices being unable to communicate at the currently configured  connection intervals.
*  - Range: 0x000a to 0x0c80
*  - Time = Value x 10 ms
*  - Time Range: 100 ms to 32 s
*  - The value in milliseconds must be larger than (1 + latency) * max_interval * 2, where max_interval is given in milliseconds
* It is recommended that the supervision timeout is set at a value which allows communication attempts over at least a few connection intervals.
* Default value: 1000 ms
*     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_set_conn_parameters_rsp_t* gecko_cmd_le_gap_set_conn_parameters(uint16 min_interval,uint16 max_interval,uint16 latency,uint16 timeout)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_conn_parameters.min_interval=min_interval;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_parameters.max_interval=max_interval;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_parameters.latency=latency;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_parameters.timeout=timeout;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_conn_parameters_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_conn_parameters;
}

/** 
*
* gecko_cmd_le_gap_set_scan_parameters
*
* Deprecated. Replacements are "le_gap_set_discovery_timing" command for setting timing parameters, and "le_gap_set_discovery_type" command for the scan type.
* The parameters set by this command is only effective on the LE 1M PHY.
* For LE Coded PHY, above replacement command must be used.
*  
*
* @param scan_interval   Scanner interval. This is defined as the time interval from when the device started its last scan until it begins the subsequent scan, that is how often to scan
*  - Time = Value x 0.625 ms
*  - Range: 0x0004 to 0x4000
*  - Time Range: 2.5 ms to 10.24 s
* Default value: 10 ms 
* There is a variable delay when switching channels at the end of each
* scanning interval which is included in the scanning interval time
* itself. During this switch time no advertising packets will be
* received by the device. The switch time variation is dependent on use
* case, for example in case of scanning while keeping active connections
* the channel switch time might be longer than when only scanning
* without any active connections. Increasing the scanning interval will
* reduce the amount of time in which the device cannot receive
* advertising packets as it will switch channels less often.
* After every scan interval the scanner will change the frequency it
* operates at. It will cycle through all the three advertising channels
* in a round robin fashion. According to the specification all three
* channels must be used by a scanner.
* 
* @param scan_window   Scan window. The duration of the scan. scan_window shall be less than or equal to scan_interval
*  - Time = Value x 0.625 ms
*  - Range: 0x0004 to 0x4000
*  - Time Range: 2.5 ms to 10.24 s
* Default value: 10 ms Note that packet reception is aborted if it has been started before scan window ends.                 
* @param active   Scan type indicated by a flag. Values:
*  - 0: Passive scanning
*  - 1: Active scanning
*  - In passive scanning mode the device only listens to advertising packets and will not transmit any packet
*  - In active scanning mode the device will send out a scan request packet upon receiving advertising packet from a remote device and then it will listen to the scan response packet from remote device
* Default value: 0                     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_set_scan_parameters_rsp_t* gecko_cmd_le_gap_set_scan_parameters(uint16 scan_interval,uint16 scan_window,uint8 active)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_scan_parameters.scan_interval=scan_interval;
    gecko_cmd_msg->data.cmd_le_gap_set_scan_parameters.scan_window=scan_window;
    gecko_cmd_msg->data.cmd_le_gap_set_scan_parameters.active=active;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_scan_parameters_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_scan_parameters;
}

/** 
*
* gecko_cmd_le_gap_set_adv_data
*
* Deprecated. Use "le_gap_bt5_set_adv_data" command to set the advertising data and scan response data.
* This command is only effective on the first advertising set (handle
* value 0). Other advertising sets are not affected.
*  
*
* @param scan_rsp   This value selects if the data is intended for advertising packets, scan response packets or advertising packet in OTA. Values: 
*  - 0: Advertising packets
*  - 1: Scan response packets
*  - 2: OTA advertising packets
*  - 4: OTA scan response packets
* 
* @param adv_data   Data to be set    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_set_adv_data_rsp_t* gecko_cmd_le_gap_set_adv_data(uint8 scan_rsp,uint8 adv_data_len, const uint8* adv_data_data)
{
    if ((uint16_t)adv_data_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_le_gap_set_adv_data.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_le_gap_set_adv_data;
    }

    
    gecko_cmd_msg->data.cmd_le_gap_set_adv_data.scan_rsp=scan_rsp;
    gecko_cmd_msg->data.cmd_le_gap_set_adv_data.adv_data.len=adv_data_len;
    memcpy(gecko_cmd_msg->data.cmd_le_gap_set_adv_data.adv_data.data,adv_data_data,adv_data_len);
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_adv_data_id+((2+adv_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_adv_data;
}

/** 
*
* gecko_cmd_le_gap_set_adv_timeout
*
* Deprecated. New command "le_gap_set_advertise_timing" should be used for this functionality.
* This command is only effective on the first advertising set (handle
* value 0). Other advertising sets are not affected.
*  
*
* @param maxevents   If non-zero, indicates the maximum number of advertising events to send before stopping advertiser. Value 0 indicates no maximum number limit.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_set_adv_timeout_rsp_t* gecko_cmd_le_gap_set_adv_timeout(uint8 maxevents)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_adv_timeout.maxevents=maxevents;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_adv_timeout_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_adv_timeout;
}

/** 
*
* gecko_cmd_le_gap_bt5_set_mode
*
* Deprecated. Replacements are "le_gap_start_advertising" command to start the advertising, and "le_gap_stop_advertising" command to stop the advertising.             "le_gap_set_advertise_timing" command can be used for setting the maxevents and command "le_gap_set_advertise_configuration" can be used to for setting address types.
*  
*
* @param handle   Advertising set handle, number of sets available is defined in stack configuration
* @param discover   Discoverable mode
* @param connect   Connectable mode
* @param maxevents   If non-zero, indicates the maximum number of advertising events to send before stopping advertiser. Value 0 indicates no maximum number limit.
* @param address_type   Address type to use for packets
*
* Events generated
*
* gecko_evt_le_gap_adv_timeout - Triggered when the number of advertising events set by this command has been done and advertising is stopped on the given advertising set.
* gecko_evt_le_connection_opened - Triggered when a remote device opened a connection to the advertiser on the specified advertising set.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_bt5_set_mode_rsp_t* gecko_cmd_le_gap_bt5_set_mode(uint8 handle,uint8 discover,uint8 connect,uint16 maxevents,uint8 address_type)
{
    
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_mode.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_mode.discover=discover;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_mode.connect=connect;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_mode.maxevents=maxevents;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_mode.address_type=address_type;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_bt5_set_mode_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_bt5_set_mode;
}

/** 
*
* gecko_cmd_le_gap_bt5_set_adv_parameters
*
* Deprecated. Replacements are "le_gap_set_advertise_timing" command for setting the advertising intervals, "le_gap_set_advertise_channel_map" command for setting the channel map, and "le_gap_set_advertise_report_scan_request" command for enabling and disabling scan request notifications.
*  
*
* @param handle   Advertising set handle, number of sets available is defined in stack configuration
* @param interval_min   Minimum advertising interval. Value in units of 0.625 ms
*  - Range: 0x20 to 0xFFFF
*  - Time range: 20 ms to 40.96 s
* Default value: 100 ms
* @param interval_max   Maximum advertising interval. Value in units of 0.625 ms
*  - Range: 0x20 to 0xFFFF
*  - Time range: 20 ms to 40.96 s
*  - Note: interval_max should be bigger than interval_min
* Default value: 200 ms
* @param channel_map   Advertising channel map which determines which of the three channels will be used for advertising. This value is given as a bitmask. Values:
*  - 1: Advertise on CH37
*  - 2: Advertise on CH38
*  - 3: Advertise on CH37 and CH38
*  - 4: Advertise on CH39
*  - 5: Advertise on CH37 and CH39
*  - 6: Advertise on CH38 and CH39
*  - 7: Advertise on all channels
* Recommended value: 7
* Default value: 7
* 
* @param report_scan   If non-zero, enables scan request notification, and scan requests will be reported as events.
* Default value: 0
* 
*
* Events generated
*
* gecko_evt_le_gap_scan_request - Triggered when a scan request has been received during the advertising if scan request notification has been enabled by this command.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_gap_bt5_set_adv_parameters_rsp_t* gecko_cmd_le_gap_bt5_set_adv_parameters(uint8 handle,uint16 interval_min,uint16 interval_max,uint8 channel_map,uint8 report_scan)
{
    
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_parameters.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_parameters.interval_min=interval_min;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_parameters.interval_max=interval_max;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_parameters.channel_map=channel_map;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_parameters.report_scan=report_scan;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_bt5_set_adv_parameters_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_bt5_set_adv_parameters;
}

/** 
*
* gecko_cmd_le_gap_bt5_set_adv_data
*
* This command can be used to set user defined data in advertising packets, scan response packets             or periodic advertising packets. Maximum 253 bytes of data can be set for extended advertising.             For setting longer advertising data, use command "le_gap_set_long_advertising_data".             
* If advertising mode is currently enabled the new advertising data will
* be used immediately.             Advertising mode can be enabled using
* command             "le_gap_start_advertising".             Periodic
* advertising mode can be enabled using command
* "le_gap_start_periodic_advertising".
* The maximum data length is 31 bytes for legacy advertising. For
* extended advertising, the maximum length is 1650 bytes,
* but advertising parameters may limit the amount of data that can be
* sent in a single advertisement.
* The invalid parameter error will be returned in following situations:
*  - The data length is more than 31 bytes but the advertiser can only advertise using legacy advertising PDUs;
*  - The data is too long to fit into a single advertisement.
*  - Set the data of advertising data packet when the advertiser is advertising in scannable mode using extended advertising PDUs;
*  - Set the data of scan response data packet when the advertiser is advertising in connectable mode using extended advertising PDUs.
* Note that the user defined data may be overwritten by the system when
* the advertising is later enabled in other discoverable mode than
* user_data.
*  
*
* @param handle   Advertising set handle, number of sets available is defined in stack configuration
* @param scan_rsp   This value selects if the data is intended for advertising packets, scan response packets, periodic advertising packets or advertising packet in OTA. Values:                 
*  - 0: Advertising packets
*  - 1: Scan response packets
*  - 2: OTA advertising packets
*  - 4: OTA scan response packets
*  - 8: Periodic advertising packets
* 
* @param adv_data   Data to be set    
*
**/

static inline struct gecko_msg_le_gap_bt5_set_adv_data_rsp_t* gecko_cmd_le_gap_bt5_set_adv_data(uint8 handle,uint8 scan_rsp,uint8 adv_data_len, const uint8* adv_data_data)
{
    if ((uint16_t)adv_data_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_le_gap_bt5_set_adv_data.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_le_gap_bt5_set_adv_data;
    }

    
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_data.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_data.scan_rsp=scan_rsp;
    gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_data.adv_data.len=adv_data_len;
    memcpy(gecko_cmd_msg->data.cmd_le_gap_bt5_set_adv_data.adv_data.data,adv_data_data,adv_data_len);
    gecko_cmd_msg->header=((gecko_cmd_le_gap_bt5_set_adv_data_id+((3+adv_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_bt5_set_adv_data;
}

/** 
*
* gecko_cmd_le_gap_set_privacy_mode
*
* This command can be used to enable or disable privacy feature on all GAP roles. The new privacy mode will take effect for advertising on the next advertising enabling, for scanning on the next scan enabling, and for initiating on the next open connection command. When privacy is enabled and the device is advertising or scanning, the stack will maintain a periodic timer with the specified time interval as timeout value. At each timeout the stack will generate a new private resolvable address and use it in advertising data packets and scanning requests.             
* By default, privacy feature is disabled.
*  
*
* @param privacy   Values: 
*  - 0: Disable privacy
*  - 1: Enable privacy
* 
* @param interval   The minimum time interval between private address change. This parameter is ignored if this command is issued for disabling privacy mode. Values: 
*  - 0: Use default interval, 15 minutes
*  - others: The time interval in minutes
*     
*
**/

static inline struct gecko_msg_le_gap_set_privacy_mode_rsp_t* gecko_cmd_le_gap_set_privacy_mode(uint8 privacy,uint8 interval)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_privacy_mode.privacy=privacy;
    gecko_cmd_msg->data.cmd_le_gap_set_privacy_mode.interval=interval;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_privacy_mode_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_privacy_mode;
}

/** 
*
* gecko_cmd_le_gap_set_advertise_timing
*
* This command can be used to set the advertising timing parameters of the given advertising set. This setting will take effect on the next advertising enabling. 
*
* @param handle   Advertising set handle
* @param interval_min   Minimum advertising interval. Value in units of 0.625 ms
*  - Range: 0x20 to 0xFFFF
*  - Time range: 20 ms to 40.96 s
* Default value: 100 ms
* @param interval_max   Maximum advertising interval. Value in units of 0.625 ms
*  - Range: 0x20 to 0xFFFF
*  - Time range: 20 ms to 40.96 s
*  - Note: interval_max should be bigger than interval_min
* Default value: 200 ms
* @param duration   The advertising duration for this advertising set. Value 0 indicates no advertising duration limit and the advertising continues until it is disabled. A non-zero value sets the duration in units of 10 ms. The duration begins at the start of the first advertising event of this advertising set.
*  - Range: 0x0001 to 0xFFFF
*  - Time range: 10 ms to 655.35 s
* Default value: 0
* @param maxevents   If non-zero, indicates the maximum number of advertising events to send before stopping advertiser. Value 0 indicates no maximum number limit. 
* Default value: 0
*     
*
**/

static inline struct gecko_msg_le_gap_set_advertise_timing_rsp_t* gecko_cmd_le_gap_set_advertise_timing(uint8 handle,uint32 interval_min,uint32 interval_max,uint16 duration,uint8 maxevents)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_timing.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_timing.interval_min=interval_min;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_timing.interval_max=interval_max;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_timing.duration=duration;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_timing.maxevents=maxevents;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_advertise_timing_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_advertise_timing;
}

/** 
*
* gecko_cmd_le_gap_set_advertise_channel_map
*
* This command can be used to set the primary advertising channel map of the given advertising set. This setting will take effect on the next advertising enabling. 
*
* @param handle   Advertising set handle
* @param channel_map   Advertisement channel map which determines which of the three channels will be used for advertising. This value is given as a bitmask. Values:
*  - 1: Advertise on CH37
*  - 2: Advertise on CH38
*  - 3: Advertise on CH37 and CH38
*  - 4: Advertise on CH39
*  - 5: Advertise on CH37 and CH39
*  - 6: Advertise on CH38 and CH39
*  - 7: Advertise on all channels
* Recommended value: 7
* Default value: 7
*     
*
**/

static inline struct gecko_msg_le_gap_set_advertise_channel_map_rsp_t* gecko_cmd_le_gap_set_advertise_channel_map(uint8 handle,uint8 channel_map)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_channel_map.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_channel_map.channel_map=channel_map;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_advertise_channel_map_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_advertise_channel_map;
}

/** 
*
* gecko_cmd_le_gap_set_advertise_report_scan_request
*
* This command can be used to enable or disable the scan request notification of the given advertising set. This setting will take effect on the next advertising enabling. 
*
* @param handle   Advertising set handle
* @param report_scan_req   If non-zero, enables scan request notification, and scan requests will be reported as events.
* Default value: 0
* 
*
* Events generated
*
* gecko_evt_le_gap_scan_request - Triggered when a scan request has been received during the advertising if scan request notification has been enabled by this command.    
*
**/

static inline struct gecko_msg_le_gap_set_advertise_report_scan_request_rsp_t* gecko_cmd_le_gap_set_advertise_report_scan_request(uint8 handle,uint8 report_scan_req)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_report_scan_request.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_report_scan_request.report_scan_req=report_scan_req;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_advertise_report_scan_request_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_advertise_report_scan_request;
}

/** 
*
* gecko_cmd_le_gap_set_advertise_phy
*
* This command can be used to set the advertising PHYs of the given advertising set. This setting will take effect on the next advertising enabling. "Invalid Parameter" error will be returned if a PHY value is invalid or the device does not support a given PHY. 
*
* @param handle   Advertising set handle
* @param primary_phy   The PHY on which the advertising packets are transmitted on the primary advertising channel. If legacy advertising PDUs are being used, the PHY must be LE 1M.
* Values:
*  - 1: Advertising PHY is LE 1M
*  - 4: Advertising PHY is LE Coded
* Default: 1
* 
* @param secondary_phy   The PHY on which the advertising packets are transmitted on the secondary advertising channel.
* Values:
*  - 1: Advertising PHY is LE 1M
*  - 2: Advertising PHY is LE 2M
*  - 4: Advertising PHY is LE Coded
* Default: 1
*     
*
**/

static inline struct gecko_msg_le_gap_set_advertise_phy_rsp_t* gecko_cmd_le_gap_set_advertise_phy(uint8 handle,uint8 primary_phy,uint8 secondary_phy)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_phy.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_phy.primary_phy=primary_phy;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_phy.secondary_phy=secondary_phy;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_advertise_phy_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_advertise_phy;
}

/** 
*
* gecko_cmd_le_gap_set_advertise_configuration
*
* This command can be used to enable advertising configuration flags on
* the given advertising set. The                 configuration change
* will take effects on the next advertising enabling.
* These configuration flags can be disabled using
* "le_gap_clear_advertise_configuration" command.
*  
*
* @param handle   Advertising set handle
* @param configurations   Advertising configuration flags to enable. This value can be a bitmask of multiple flags. Flags:                 
*  - 1 (Bit 0): Use legacy advertising PDUs. 
*  - 2 (Bit 1): Omit advertiser's address from all PDUs (anonymous advertising). This flag is effective only in extended advertising.
*  - 4 (Bit 2): Use le_gap_non_resolvable address type. Advertising must be in non-connectable mode if this configuration is enabled.
*  - 8 (Bit 3): Include TX power in advertising packets. This flag is effective only in extended advertising.
* Default value: 1
*     
*
**/

static inline struct gecko_msg_le_gap_set_advertise_configuration_rsp_t* gecko_cmd_le_gap_set_advertise_configuration(uint8 handle,uint32 configurations)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_configuration.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_configuration.configurations=configurations;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_advertise_configuration_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_advertise_configuration;
}

/** 
*
* gecko_cmd_le_gap_clear_advertise_configuration
*
*  
*
* @param handle   Advertising set handle
* @param configurations   Advertising configuration flags to disable. This value can be a bitmask of multiple flags. See "le_gap_set_advertise_configuration" for possible flags.                     
*
**/

static inline struct gecko_msg_le_gap_clear_advertise_configuration_rsp_t* gecko_cmd_le_gap_clear_advertise_configuration(uint8 handle,uint32 configurations)
{
    
    gecko_cmd_msg->data.cmd_le_gap_clear_advertise_configuration.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_clear_advertise_configuration.configurations=configurations;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_clear_advertise_configuration_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_clear_advertise_configuration;
}

/** 
*
* gecko_cmd_le_gap_start_advertising
*
* This command can be used to start the advertising of the given advertising set with specified discoverable and connectable modes.             
* The number of concurrent connectable advertisings is also limited by
* MAX_CONNECTIONS configuration. For example, only one connectable
* advertising can be enabled if the device has (MAX_CONNECTIONS - 1)
* connections when this command is called. The limitation does not apply
* to non-connectable advertising.
* The default advertising configuration in the stack is set to using
* legacy advertising PDUs on LE 1M PHY. The stack will automatically
* select extended advertising PDUs if either of the followings has
* occurred under the default configuration:
*  - The connectable mode is set to le_gap_connectable_non_scannable.
*  - The primary advertising PHY has been set to LE Coded PHY by command "le_gap_set_advertise_phy".
*  - The user advertising data length is more than 31 bytes.
*  - Periodic advertising has been enabled.
* If currently set parameters can't be used then an error will be
* returned. Specifically, this command fails with "Connection Limit
* Exceeded" error if it may cause the number of connections exceeding
* the configured MAX_CONNECTIONS value. It fails with "Invalid
* Parameter" error if one of the following cases occur:
*  - Non-resolvable random address is used but the connectable mode is le_gap_connectable_scannable or le_gap_connectable_non_scannable.
*  - The connectable mode is le_gap_connectable_non_scannable, but using legacy advertising PDUs has been explicitly enabled with command "le_gap_set_advertise_configuration".
*  - The primary advertising PHY is LE Coded PHY but using legacy advertising PDUs has been explicitly enabled with command "le_gap_set_advertise_configuration".
*  - The connectable mode is le_gap_connectable_scannable but using extended advertising PDUs has been explicitly enabled or the primary advertising PHY has been set to LE Coded PHY.
* If advertising will be enabled in user_data mode,
* "le_gap_bt5_set_adv_data" should be used to set advertising and scan
* response data before issuing this command. When the advertising is
* enabled in other modes than user_data, the advertising and scan
* response data is generated by the stack using the following procedure:
*  - Add a Flags field to advertising data.
*  - Add a TX power level field to advertising data if TX power service exists in the local GATT database.
*  - Add a Slave Connection Interval Range field to advertising data if the GAP peripheral preferred connection parameters characteristic exists in the local GATT database.
*  - Add a list of 16-bit Service UUIDs to advertising data if there are one or more 16-bit service UUIDs to advertise. The list is complete if all advertised 16-bit UUIDs are in advertising data; otherwise the list is incomplete.
*  - Add a list of 128-bit service UUIDs to advertising data if there are one or more 128-bit service UUIDs to advertise and there is still free space for this field. The list is complete if all advertised 128-bit UUIDs are in advertising data; otherwise the list is incomplete. Note that an advertising data packet can contain at most one 128-bit service UUID.
*  - Try to add the full local name to advertising data if device is not in privacy mode. In case the full local name does not fit into the remaining free space, the advertised name is a shortened version by cutting off the end if the free space has at least 6 bytes; Otherwise, the local name is added to scan response data.
* Event "le_connection_opened" will be received when a remote device
* opens a connection to the advertiser on this advertising set.
*  
*
* @param handle   Advertising set handle
* @param discover   Discoverable mode
* @param connect   Connectable mode
*
* Events generated
*
* gecko_evt_le_gap_adv_timeout - Triggered when the number of advertising events set by this command has been done and advertising is stopped on the given advertising set.
* gecko_evt_le_connection_opened - Triggered when a remote device opened a connection to the advertiser on the specified advertising set.    
*
**/

static inline struct gecko_msg_le_gap_start_advertising_rsp_t* gecko_cmd_le_gap_start_advertising(uint8 handle,uint8 discover,uint8 connect)
{
    
    gecko_cmd_msg->data.cmd_le_gap_start_advertising.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_start_advertising.discover=discover;
    gecko_cmd_msg->data.cmd_le_gap_start_advertising.connect=connect;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_start_advertising_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_start_advertising;
}

/** 
*
* gecko_cmd_le_gap_stop_advertising
*
* This command can be used to stop the advertising of the given advertising set.              
*
* @param handle   Advertising set handle    
*
**/

static inline struct gecko_msg_le_gap_stop_advertising_rsp_t* gecko_cmd_le_gap_stop_advertising(uint8 handle)
{
    
    gecko_cmd_msg->data.cmd_le_gap_stop_advertising.handle=handle;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_stop_advertising_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_stop_advertising;
}

/** 
*
* gecko_cmd_le_gap_set_discovery_timing
*
* This command can be used to set the timing parameters of the specified PHYs. If the device is currently scanning for advertising devices the PHYs, new parameters will take effect when the scanning is restarted. 
*
* @param phys   The PHYs for which the parameters are set. 
*  - 1: LE 1M PHY
*  - 4: LE Coded PHY
*  - 5: LE 1M PHY and LE Coded PHY
* 
* @param scan_interval   Scan interval. This is defined as the time interval from when the device started its last scan until it begins the subsequent scan, that is how often to scan
*  - Time = Value x 0.625 ms
*  - Range: 0x0004 to 0xFFFF
*  - Time Range: 2.5 ms to 40.96 s
* Default value: 10 ms 
* There is a variable delay when switching channels at the end of each
* scanning interval which is included in the scanning interval time
* itself. During this switch time no advertising packets will be
* received by the device. The switch time variation is dependent on use
* case, for example in case of scanning while keeping active connections
* the channel switch time might be longer than when only scanning
* without any active connections. Increasing the scanning interval will
* reduce the amount of time in which the device cannot receive
* advertising packets as it will switch channels less often.
* After every scan interval the scanner will change the frequency it
* operates at. It will cycle through all the three advertising channels
* in a round robin fashion. According to the specification all three
* channels must be used by a scanner.
* 
* @param scan_window   Scan window. The duration of the scan. scan_window shall be less than or equal to scan_interval
*  - Time = Value x 0.625 ms
*  - Range: 0x0004 to 0xFFFF
*  - Time Range: 2.5 ms to 40.96 s
* Default value: 10 ms Note that packet reception is aborted if it has been started before scan window ends.     
*
**/

static inline struct gecko_msg_le_gap_set_discovery_timing_rsp_t* gecko_cmd_le_gap_set_discovery_timing(uint8 phys,uint16 scan_interval,uint16 scan_window)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_discovery_timing.phys=phys;
    gecko_cmd_msg->data.cmd_le_gap_set_discovery_timing.scan_interval=scan_interval;
    gecko_cmd_msg->data.cmd_le_gap_set_discovery_timing.scan_window=scan_window;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_discovery_timing_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_discovery_timing;
}

/** 
*
* gecko_cmd_le_gap_set_discovery_type
*
* This command can be used to set the scan type of the specified PHYs. If the device is currently scanning for advertising devices on the PHYs, new parameters will take effect when the scanning is restarted 
*
* @param phys   The PHYs for which the parameters are set. 
*  - 1: LE 1M PHY
*  - 4: LE Coded PHY
*  - 5: LE 1M PHY and LE Coded PHY
* 
* @param scan_type   Scan type indicated by a flag. Values:
*  - 0: Passive scanning
*  - 1: Active scanning
*  - In passive scanning mode the device only listens to advertising packets and will not transmit any packet
*  - In active scanning mode the device will send out a scan request packet upon receiving advertising packet from a remote device and then it will listen to the scan response packet from remote device
* Default value: 0                     
*
**/

static inline struct gecko_msg_le_gap_set_discovery_type_rsp_t* gecko_cmd_le_gap_set_discovery_type(uint8 phys,uint8 scan_type)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_discovery_type.phys=phys;
    gecko_cmd_msg->data.cmd_le_gap_set_discovery_type.scan_type=scan_type;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_discovery_type_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_discovery_type;
}

/** 
*
* gecko_cmd_le_gap_start_discovery
*
* This command can be used to start the GAP discovery procedure to scan
* for advertising devices on the specified scanning PHY, that is to
* perform a device discovery. To cancel an ongoing
* discovery process use the "le_gap_end_procedure" command.
* "Invalid Parameter" error will be returned if the scanning PHY value
* is invalid or the device does not support the PHY.
*  
*
* @param scanning_phy   The scanning PHY. Value: 
*  - 1: LE 1M PHY
*  - 4: LE Coded PHY
* 
* @param mode   Bluetooth discovery Mode. For values see link
*
* Events generated
*
* gecko_evt_le_gap_scan_response - Every time an advertising packet is received, this event is triggered. The packets are not filtered in any way, so multiple events will be                     received for every advertising device in range.    
*
**/

static inline struct gecko_msg_le_gap_start_discovery_rsp_t* gecko_cmd_le_gap_start_discovery(uint8 scanning_phy,uint8 mode)
{
    
    gecko_cmd_msg->data.cmd_le_gap_start_discovery.scanning_phy=scanning_phy;
    gecko_cmd_msg->data.cmd_le_gap_start_discovery.mode=mode;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_start_discovery_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_start_discovery;
}

/** 
*
* gecko_cmd_le_gap_set_data_channel_classification
*
* This command can be used to specify a channel classification for data channels. This classification persists until overwritten with a subsequent command or until the system is reset. 
*
* @param channel_map   This parameter is 5 bytes and contains 37 1-bit fields.{br}                 The nth such field (in the range 0 to 36) contains the value for the link layer channel index n.{br}                 
*  - 0: Channel n is bad.
*  - 1:  Channel n is unknown.
* The most significant bits are reserved and shall be set to 0 for future use.{br}                 At least two channels shall be marked as unknown.                     
*
**/

static inline struct gecko_msg_le_gap_set_data_channel_classification_rsp_t* gecko_cmd_le_gap_set_data_channel_classification(uint8 channel_map_len, const uint8* channel_map_data)
{
    if ((uint16_t)channel_map_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_le_gap_set_data_channel_classification.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_le_gap_set_data_channel_classification;
    }

    
    gecko_cmd_msg->data.cmd_le_gap_set_data_channel_classification.channel_map.len=channel_map_len;
    memcpy(gecko_cmd_msg->data.cmd_le_gap_set_data_channel_classification.channel_map.data,channel_map_data,channel_map_len);
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_data_channel_classification_id+((1+channel_map_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_data_channel_classification;
}

/** 
*
* gecko_cmd_le_gap_connect
*
* This command can be used to connect an advertising device with the
* specified initiating PHY.                 The Bluetooth stack will
* enter a state where it continuously                 scans for the
* connectable advertising packets from the remote                 device
* which matches the Bluetooth address given as a
* parameter. Scan parameters set in
* "le_gap_set_discovery_timing" are used in this operation.
* Upon receiving the advertising packet, the module                 will
* send a connection request packet to the target device to
* initiate a Bluetooth connection. To cancel an ongoing
* connection process use the "le_connection_close" command with the
* handle received in the response from this command.
* A connection is opened in no-security mode. If the GATT
* client needs to read or write the attributes on GATT server
* requiring encryption or authentication, it must first encrypt
* the connection using an appropriate authentication method.
* If a connection cannot be established at all for some reason (for
* example,                 the remote device has gone out of range, has
* entered into deep sleep, or                 is not advertising), the
* stack will try to connect forever.                 In this case the
* application will not get any event related to the connection request.
* To recover from this situation, application can implement a timeout
* and call                 "le_connection_close" to cancel the
* connection request.
* This command fails with "Connection Limit Exceeded" error if
* the number of connections attempted to be opened exceeds the
* configured MAX_CONNECTIONS value.
* This command fails with "Invalid Parameter" error if the initiating
* PHY value is invalid or the device does not support the PHY.
* Later calls of this command have to wait for the ongoing command
* to complete. A received event                   "le_connection_opened"
* indicates connection opened successfully and a received event
* "le_connection_closed"                  indicates connection failures
* have occurred.
*  
*
* @param address   Address of the device to connect to
* @param address_type   Address type of the device to connect to
* @param initiating_phy   The initiating PHY. Value: 
*  - 1: LE 1M PHY
*  - 4: LE Coded PHY
* 
*
* Events generated
*
* gecko_evt_le_connection_opened - This event is triggered after the connection has been opened, and indicates whether the devices are already bonded and what is the role of the Bluetooth device (Slave or Master).
* gecko_evt_le_connection_parameters - This event indicates the connection parameters and security mode of the connection.    
*
**/

static inline struct gecko_msg_le_gap_connect_rsp_t* gecko_cmd_le_gap_connect(bd_addr address,uint8 address_type,uint8 initiating_phy)
{
    
    memcpy(&gecko_cmd_msg->data.cmd_le_gap_connect.address,&address,sizeof(bd_addr));
    gecko_cmd_msg->data.cmd_le_gap_connect.address_type=address_type;
    gecko_cmd_msg->data.cmd_le_gap_connect.initiating_phy=initiating_phy;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_connect_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_connect;
}

/** 
*
* gecko_cmd_le_gap_set_advertise_tx_power
*
* This command can be used to limit the maximum advertising TX power on the given advertising set.                 If the value goes over the global value that has been set using                 "system_set_tx_power" command,                 the global value will be the maximum limit.                 The maximum TX power of legacy advertising is further constrainted to not go over +10 dBm.                 Extended advertising TX power can be +10 dBm and over if Adaptive Frequency Hopping has been enabled.                 
* This setting will take effect on the next advertising enabling.
* By default, maximum advertising TX power is limited by the global
* value.
*  
*
* @param handle   Advertising set handle
* @param power   TX power in 0.1dBm steps, for example the value of 10 is 1dBm and 55 is 5.5dBm    
*
**/

static inline struct gecko_msg_le_gap_set_advertise_tx_power_rsp_t* gecko_cmd_le_gap_set_advertise_tx_power(uint8 handle,int16 power)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_tx_power.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_advertise_tx_power.power=power;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_advertise_tx_power_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_advertise_tx_power;
}

/** 
*
* gecko_cmd_le_gap_set_discovery_extended_scan_response
*
* This command can be used to enable and disable the extended scan response event.                 When the extended scan response event is enabled, it replaces "le_gap_scan_response",                 that is, the stack will generate either "le_gap_extended_scan_response"                 or "le_gap_scan_response", but not both at a time.              
*
* @param enable   Values: 
*  - 0: Disable extended scan response event
*  - 1: Enable extended scan response event
*     
*
**/

static inline struct gecko_msg_le_gap_set_discovery_extended_scan_response_rsp_t* gecko_cmd_le_gap_set_discovery_extended_scan_response(uint8 enable)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_discovery_extended_scan_response.enable=enable;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_discovery_extended_scan_response_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_discovery_extended_scan_response;
}

/** 
*
* gecko_cmd_le_gap_start_periodic_advertising
*
* This command can be used to start periodic advertising on the given advertising set.                 The stack will enable the advertising set automatically if the set has not been enabled                 and the set can advertise using extended advertising PDUs.                 
* "Invalid Parameter" error will be returned if the application has
* configured                 to use legacy advertising PDUs or anonymous
* advertising, or advertising set has been                 enabled using
* legacy advertising PDUs.
*  
*
* @param handle   Advertising set handle
* @param interval_min   Minimum periodic advertising interval. Value in units of 1.25 ms
*  - Range: 0x06 to 0xFFFF
*  - Time range: 7.5 ms to 81.92 s
* Default value: 100 ms
* @param interval_max   Maximum periodic advertising interval. Value in units of 1.25 ms
*  - Range: 0x06 to 0xFFFF
*  - Time range: 7.5 ms to 81.92 s
*  - Note: interval_max should be bigger than interval_min
* Default value: 200 ms
* @param flags   Periodic advertising configurations. Bitmask of followings:                     
*  - Bit 0: Include TX power in advertising PDU
*     
*
**/

static inline struct gecko_msg_le_gap_start_periodic_advertising_rsp_t* gecko_cmd_le_gap_start_periodic_advertising(uint8 handle,uint16 interval_min,uint16 interval_max,uint32 flags)
{
    
    gecko_cmd_msg->data.cmd_le_gap_start_periodic_advertising.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_start_periodic_advertising.interval_min=interval_min;
    gecko_cmd_msg->data.cmd_le_gap_start_periodic_advertising.interval_max=interval_max;
    gecko_cmd_msg->data.cmd_le_gap_start_periodic_advertising.flags=flags;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_start_periodic_advertising_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_start_periodic_advertising;
}

/** 
*
* gecko_cmd_le_gap_stop_periodic_advertising
*
* This command can be used to stop the periodic advertising on the given advertising set.             
* This command does not affect the enable state of the advertising set.
*  
*
* @param handle   Advertising set handle    
*
**/

static inline struct gecko_msg_le_gap_stop_periodic_advertising_rsp_t* gecko_cmd_le_gap_stop_periodic_advertising(uint8 handle)
{
    
    gecko_cmd_msg->data.cmd_le_gap_stop_periodic_advertising.handle=handle;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_stop_periodic_advertising_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_stop_periodic_advertising;
}

/** 
*
* gecko_cmd_le_gap_set_long_advertising_data
*
* This command can be used to set advertising data for specified packet
* type and advertising set.             All data currently in the system
* data buffer will be extracted as the advertising data. The buffer
* will be emptied after this command regardless of the completion
* status.
* Prior to calling this command, data could be added to the buffer with
* one or multiple calls of             "system_data_buffer_write".
* See "le_gap_bt5_set_adv_data" for more details of advertising data.
*  
*
* @param handle   Advertising set handle
* @param packet_type   This value selects if the data is intended for advertising packets, scan response packets, or periodic advertising packets. Values:                     
*  - 0: Advertising packets
*  - 1: Scan response packets
*  - 2: OTA advertising packets
*  - 4: OTA scan response packets
*  - 8: Periodic advertising packets
*     
*
**/

static inline struct gecko_msg_le_gap_set_long_advertising_data_rsp_t* gecko_cmd_le_gap_set_long_advertising_data(uint8 handle,uint8 packet_type)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_long_advertising_data.handle=handle;
    gecko_cmd_msg->data.cmd_le_gap_set_long_advertising_data.packet_type=packet_type;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_long_advertising_data_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_long_advertising_data;
}

/** 
*
* gecko_cmd_le_gap_enable_whitelisting
*
* This command is used to enable whitelisting. To add devices to the whitelist either bond with the device or add it manually with "sm_add_to_whitelist" 
*
* @param enable   1 enable, 0 disable whitelisting.    
*
**/

static inline struct gecko_msg_le_gap_enable_whitelisting_rsp_t* gecko_cmd_le_gap_enable_whitelisting(uint8 enable)
{
    
    gecko_cmd_msg->data.cmd_le_gap_enable_whitelisting.enable=enable;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_enable_whitelisting_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_enable_whitelisting;
}

/** 
*
* gecko_cmd_le_gap_set_conn_timing_parameters
*
* This command can be used to set the default Bluetooth connection parameters. The configured values are valid for all subsequent connections that will              be established. For changing the parameters of an already established connection use the command "le_connection_set_timing_parameters".              
*
* @param min_interval   Minimum value for the connection event interval. This must be set be less than or equal to max_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* Default value: 20 ms                 
* @param max_interval   Maximum value for the connection event interval. This must be set greater than or equal to min_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* Default value: 50 ms                 
* @param latency   Slave latency. This parameter defines how many connection intervals the slave can skip if it has no data to send
*  - Range: 0x0000 to 0x01f4
* Default value: 0                 
* @param timeout   Supervision timeout. The supervision timeout defines for how long the connection is maintained despite the devices being unable to communicate at the currently configured  connection intervals.
*  - Range: 0x000a to 0x0c80
*  - Time = Value x 10 ms
*  - Time Range: 100 ms to 32 s
*  - The value in milliseconds must be larger than (1 + latency) * max_interval * 2, where max_interval is given in milliseconds
* It is recommended that the supervision timeout is set at a value which allows communication attempts over at least a few connection intervals.
* Default value: 1000 ms
* 
* @param min_ce_length   Minimum value for the connection event length. This must be set be less than or equal to max_ce_length.
*  - Time = Value x 0.625 ms
*  - Range: 0x0000 to 0xffff
* Default value: 0x0000                 
* @param max_ce_length   Maximum value for the connection event length. This must be set greater than or equal to min_ce_length.
*  - Time = Value x 0.625 ms
*  - Range: 0x0000 to 0xffff
* Default value: 0xffff                     
*
**/

static inline struct gecko_msg_le_gap_set_conn_timing_parameters_rsp_t* gecko_cmd_le_gap_set_conn_timing_parameters(uint16 min_interval,uint16 max_interval,uint16 latency,uint16 timeout,uint16 min_ce_length,uint16 max_ce_length)
{
    
    gecko_cmd_msg->data.cmd_le_gap_set_conn_timing_parameters.min_interval=min_interval;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_timing_parameters.max_interval=max_interval;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_timing_parameters.latency=latency;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_timing_parameters.timeout=timeout;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_timing_parameters.min_ce_length=min_ce_length;
    gecko_cmd_msg->data.cmd_le_gap_set_conn_timing_parameters.max_ce_length=max_ce_length;
    gecko_cmd_msg->header=((gecko_cmd_le_gap_set_conn_timing_parameters_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_gap_set_conn_timing_parameters;
}

/** 
*
* gecko_cmd_sync_open
*
* This command can be used to establish a synchronization with a
* periodic advertising from the specified             advertiser and
* begin receiving periodic advertising packets. Note that
* synchronization establishment can only             occur when scanning
* is enabled. While scanning is disabled, no attempt to synchronize will
* take place.
* The application should decide skip and timeout values based on the
* periodic advertising interval provided by             the advertiser.
* It is recommended to set skip and timeout at the values that allow a
* few receiving attempts.             Periodic advertising intervals are
* reported in event             "le_gap_extended_scan_response".
*  
*
* @param adv_sid   Advertising set identifier
* @param skip   The maximum number of periodic advertising packets that can be skipped after a successful receive.                     Range: 0x0000 to 0x01F3                  
* @param timeout   The maximum permitted time between successful receives. If this time is exceeded, synchronization is lost. Unit: 10 ms.                      
*  - Range: 0x06 to 0xFFFF
*  - Unit: 10 ms
*  - Time range: 100 ms ms to 163.84 s
* 
* @param address   Address of the advertiser
* @param address_type   Advertiser address type. Values: 
*  - 0: Public address
*  - 1: Random address
* 
*
* Events generated
*
* gecko_evt_sync_opened - This event is triggered after the synchronization has been established.
* gecko_evt_sync_data - This event indicates a periodic advertising packet has been received.    
*
**/

static inline struct gecko_msg_sync_open_rsp_t* gecko_cmd_sync_open(uint8 adv_sid,uint16 skip,uint16 timeout,bd_addr address,uint8 address_type)
{
    
    gecko_cmd_msg->data.cmd_sync_open.adv_sid=adv_sid;
    gecko_cmd_msg->data.cmd_sync_open.skip=skip;
    gecko_cmd_msg->data.cmd_sync_open.timeout=timeout;
    memcpy(&gecko_cmd_msg->data.cmd_sync_open.address,&address,sizeof(bd_addr));
    gecko_cmd_msg->data.cmd_sync_open.address_type=address_type;
    gecko_cmd_msg->header=((gecko_cmd_sync_open_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sync_open;
}

/** 
*
* gecko_cmd_sync_close
*
* This command can be used to close a synchronization with periodic advertising or cancel an ongoing             synchronization establishment procedure.          
*
* @param sync   Periodic advertising synchronization handle
*
* Events generated
*
* gecko_evt_sync_closed - This event is triggered after the synchronization has been closed.    
*
**/

static inline struct gecko_msg_sync_close_rsp_t* gecko_cmd_sync_close(uint8 sync)
{
    
    gecko_cmd_msg->data.cmd_sync_close.sync=sync;
    gecko_cmd_msg->header=((gecko_cmd_sync_close_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sync_close;
}

/** 
*
* gecko_cmd_le_connection_set_parameters
*
* Deprecated. Replacement is "le_connection_set_timing_parameters" command for setting timing parameters.
* This command can be used to request a change in the connection
* parameters of a Bluetooth connection.
*  
*
* @param connection   Connection Handle
* @param min_interval   Minimum value for the connection event interval. This must be set be less than or equal to max_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* 
* @param max_interval   Maximum value for the connection event interval. This must be set greater than or equal to min_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* 
* @param latency   Slave latency. This parameter defines how many connection intervals the slave can skip if it has no data to send
*  - Range: 0x0000 to 0x01f4
* Use 0x0000 for default value                 
* @param timeout   Supervision timeout. The supervision timeout defines for how long the connection is maintained despite the devices being unable to communicate at the currently configured  connection intervals.
*  - Range: 0x000a to 0x0c80
*  - Time = Value x 10 ms
*  - Time Range: 100 ms to 32 s
*  - The value in milliseconds must be larger than (1 + latency) * max_interval * 2, where max_interval is given in milliseconds
* It is recommended that the supervision timeout is set at a value which allows communication attempts over at least a few connection intervals.                 
*
* Events generated
*
* gecko_evt_le_connection_parameters - This event is triggered after new connection parameters has been applied on the connection.    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_le_connection_set_parameters_rsp_t* gecko_cmd_le_connection_set_parameters(uint8 connection,uint16 min_interval,uint16 max_interval,uint16 latency,uint16 timeout)
{
    
    gecko_cmd_msg->data.cmd_le_connection_set_parameters.connection=connection;
    gecko_cmd_msg->data.cmd_le_connection_set_parameters.min_interval=min_interval;
    gecko_cmd_msg->data.cmd_le_connection_set_parameters.max_interval=max_interval;
    gecko_cmd_msg->data.cmd_le_connection_set_parameters.latency=latency;
    gecko_cmd_msg->data.cmd_le_connection_set_parameters.timeout=timeout;
    gecko_cmd_msg->header=((gecko_cmd_le_connection_set_parameters_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_connection_set_parameters;
}

/** 
*
* gecko_cmd_le_connection_get_rssi
*
* This command can be used to get the latest RSSI value of a Bluetooth connection. 
*
* @param connection   Connection handle
*
* Events generated
*
* gecko_evt_le_connection_rssi - Triggered when this command has completed.    
*
**/

static inline struct gecko_msg_le_connection_get_rssi_rsp_t* gecko_cmd_le_connection_get_rssi(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_le_connection_get_rssi.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_le_connection_get_rssi_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_connection_get_rssi;
}

/** 
*
* gecko_cmd_le_connection_disable_slave_latency
*
* This command temporarily enables or disables slave latency. Used only when Bluetooth device is in slave role. 
*
* @param connection   Connection Handle
* @param disable   0 enable, 1 disable slave latency    
*
**/

static inline struct gecko_msg_le_connection_disable_slave_latency_rsp_t* gecko_cmd_le_connection_disable_slave_latency(uint8 connection,uint8 disable)
{
    
    gecko_cmd_msg->data.cmd_le_connection_disable_slave_latency.connection=connection;
    gecko_cmd_msg->data.cmd_le_connection_disable_slave_latency.disable=disable;
    gecko_cmd_msg->header=((gecko_cmd_le_connection_disable_slave_latency_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_connection_disable_slave_latency;
}

/** 
*
* gecko_cmd_le_connection_set_phy
*
* This command can be used to set preferred PHYs for connection. Preferred PHYs are connection specific. Event "le_connection_phy_status" is received when PHY update procedure has been completed. Other than preferred PHY can also be set if remote device does not accept any of the preferred PHYs. 
* NOTE: 2 Mbit and Coded PHYs are not supported by all devices.
*  
*
* @param connection   
* @param phy   Preferred PHYs for connection. This parameter is bitfield and multiple PHYs can be preferred by setting multiple bits.                         
*  - 0x01: 1 Mbit PHY
*  - 0x02: 2 Mbit PHY
*  - 0x04: 125 kbit Coded PHY (S=8)
*  - 0x08: 500 kbit Coded PHY (S=2)
* 
*
* Events generated
*
* gecko_evt_le_connection_phy_status -     
*
**/

static inline struct gecko_msg_le_connection_set_phy_rsp_t* gecko_cmd_le_connection_set_phy(uint8 connection,uint8 phy)
{
    
    gecko_cmd_msg->data.cmd_le_connection_set_phy.connection=connection;
    gecko_cmd_msg->data.cmd_le_connection_set_phy.phy=phy;
    gecko_cmd_msg->header=((gecko_cmd_le_connection_set_phy_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_connection_set_phy;
}

/** 
*
* gecko_cmd_le_connection_close
*
* This command can be used to close a Bluetooth connection or cancel an ongoing                 connection establishment process. The parameter is a connection handle which is reported in "le_connection_opened" event or le_gap_connect response. 
*
* @param connection   Handle of the connection to be closed
*
* Events generated
*
* gecko_evt_le_connection_closed -     
*
**/

static inline struct gecko_msg_le_connection_close_rsp_t* gecko_cmd_le_connection_close(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_le_connection_close.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_le_connection_close_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_connection_close;
}

/** 
*
* gecko_cmd_le_connection_set_timing_parameters
*
* This command can be used to request a change in the connection parameters of a Bluetooth  connection.              
*
* @param connection   Connection Handle
* @param min_interval   Minimum value for the connection event interval. This must be set be less than or equal to max_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* 
* @param max_interval   Maximum value for the connection event interval. This must be set greater than or equal to min_interval.
*  - Time = Value x 1.25 ms
*  - Range: 0x0006 to 0x0c80
*  - Time Range: 7.5 ms to 4 s
* 
* @param latency   Slave latency. This parameter defines how many connection intervals the slave can skip if it has no data to send
*  - Range: 0x0000 to 0x01f4
* Use 0x0000 for default value                 
* @param timeout   Supervision timeout. The supervision timeout defines for how long the connection is maintained despite the devices being unable to communicate at the currently configured  connection intervals.
*  - Range: 0x000a to 0x0c80
*  - Time = Value x 10 ms
*  - Time Range: 100 ms to 32 s
*  - The value in milliseconds must be larger than (1 + latency) * max_interval * 2, where max_interval is given in milliseconds
* It is recommended that the supervision timeout is set at a value which allows communication attempts over at least a few connection intervals.                 
* @param min_ce_length   Minimum value for the connection event length. This must be set be less than or equal to max_ce_length.
*  - Time = Value x 0.625 ms
*  - Range: 0x0000 to 0xffff
* 
* @param max_ce_length   Maximum value for the connection event length. This must be set greater than or equal to min_ce_length.
*  - Time = Value x 0.625 ms
*  - Range: 0x0000 to 0xffff
* 
*
* Events generated
*
* gecko_evt_le_connection_parameters - This event is triggered after new connection parameters has been applied on the connection.    
*
**/

static inline struct gecko_msg_le_connection_set_timing_parameters_rsp_t* gecko_cmd_le_connection_set_timing_parameters(uint8 connection,uint16 min_interval,uint16 max_interval,uint16 latency,uint16 timeout,uint16 min_ce_length,uint16 max_ce_length)
{
    
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.connection=connection;
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.min_interval=min_interval;
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.max_interval=max_interval;
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.latency=latency;
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.timeout=timeout;
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.min_ce_length=min_ce_length;
    gecko_cmd_msg->data.cmd_le_connection_set_timing_parameters.max_ce_length=max_ce_length;
    gecko_cmd_msg->header=((gecko_cmd_le_connection_set_timing_parameters_id+((13)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_le_connection_set_timing_parameters;
}

/** 
*
* gecko_cmd_gatt_set_max_mtu
*
* This command can be used to set the maximum size of ATT Message Transfer Units (MTU).             Functionality is the same as "gatt_server_set_max_mtu", and this setting applies to both GATT client and server.             If the given value is too large according to the maximum BGAPI payload size, the system will select the maximal possible             value as the maximum ATT_MTU. If maximum ATT_MTU is larger than 23, the GATT client in stack will automatically             send an MTU exchange request after a Bluetooth connection has been established.              
*
* @param max_mtu   Maximum size of Message Transfer Units (MTU) allowed
*  - Range:  23 to 250
* Default: 247    
*
**/

static inline struct gecko_msg_gatt_set_max_mtu_rsp_t* gecko_cmd_gatt_set_max_mtu(uint16 max_mtu)
{
    
    gecko_cmd_msg->data.cmd_gatt_set_max_mtu.max_mtu=max_mtu;
    gecko_cmd_msg->header=((gecko_cmd_gatt_set_max_mtu_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_set_max_mtu;
}

/** 
*
* gecko_cmd_gatt_discover_primary_services
*
* This command can be used to discover all the primary services of a remote GATT database. This command generates a unique gatt_service event for              every discovered primary service. Received "gatt_procedure_completed" event indicates that this GATT procedure              has successfully completed or failed with error. 
*
* @param connection   
*
* Events generated
*
* gecko_evt_gatt_service - Discovered service from remote GATT database
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_discover_primary_services_rsp_t* gecko_cmd_gatt_discover_primary_services(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_gatt_discover_primary_services.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_gatt_discover_primary_services_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_discover_primary_services;
}

/** 
*
* gecko_cmd_gatt_discover_primary_services_by_uuid
*
* This command can be used to discover primary services with the specified UUID in a remote GATT database. This command generates unique gatt_service event for every discovered primary service. Received "gatt_procedure_completed" event indicates that this GATT procedure has succesfully completed or failed with error. 
*
* @param connection   
* @param uuid   Service UUID
*
* Events generated
*
* gecko_evt_gatt_service - Discovered service from remote GATT database.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t* gecko_cmd_gatt_discover_primary_services_by_uuid(uint8 connection,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_gatt_discover_primary_services_by_uuid.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_discover_primary_services_by_uuid;
    }

    
    gecko_cmd_msg->data.cmd_gatt_discover_primary_services_by_uuid.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_discover_primary_services_by_uuid.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_discover_primary_services_by_uuid.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_discover_primary_services_by_uuid_id+((2+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_discover_primary_services_by_uuid;
}

/** 
*
* gecko_cmd_gatt_discover_characteristics
*
* This command can be used to discover all characteristics of the defined GATT service from a remote GATT database. This command generates a unique gatt_characteristic event for every discovered characteristic. Received "gatt_procedure_completed" event indicates that this GATT procedure has succesfully completed or failed with error. 
*
* @param connection   
* @param service   
*
* Events generated
*
* gecko_evt_gatt_characteristic - Discovered characteristic from remote GATT database.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_discover_characteristics_rsp_t* gecko_cmd_gatt_discover_characteristics(uint8 connection,uint32 service)
{
    
    gecko_cmd_msg->data.cmd_gatt_discover_characteristics.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_discover_characteristics.service=service;
    gecko_cmd_msg->header=((gecko_cmd_gatt_discover_characteristics_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_discover_characteristics;
}

/** 
*
* gecko_cmd_gatt_discover_characteristics_by_uuid
*
* This command can be used to discover all the characteristics of the specified GATT service in a remote GATT database having the specified UUID. This command generates a unique gatt_characteristic event for every discovered characteristic having the specified UUID. Received "gatt_procedure_completed" event indicates that this GATT procedure has successfully completed or failed with error.              
*
* @param connection   
* @param service   
* @param uuid   Characteristic UUID
*
* Events generated
*
* gecko_evt_gatt_characteristic - Discovered characteristic from remote GATT database.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t* gecko_cmd_gatt_discover_characteristics_by_uuid(uint8 connection,uint32 service,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 6)
    {
        gecko_rsp_msg->data.rsp_gatt_discover_characteristics_by_uuid.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_discover_characteristics_by_uuid;
    }

    
    gecko_cmd_msg->data.cmd_gatt_discover_characteristics_by_uuid.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_discover_characteristics_by_uuid.service=service;
    gecko_cmd_msg->data.cmd_gatt_discover_characteristics_by_uuid.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_discover_characteristics_by_uuid.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_discover_characteristics_by_uuid_id+((6+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_discover_characteristics_by_uuid;
}

/** 
*
* gecko_cmd_gatt_set_characteristic_notification
*
* This command can be used to enable or disable the notifications and indications being sent from a remote GATT server. This procedure discovers a              characteristic client configuration descriptor and writes the related configuration flags to a remote GATT database. A received "gatt_procedure_completed" event             indicates that this GATT procedure has successfully completed or that is has failed with an error. 
*
* @param connection   
* @param characteristic   
* @param flags   Characteristic client configuration flags
*
* Events generated
*
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.
* gecko_evt_gatt_characteristic_value - If an indication or notification has been enabled for a characteristic, this event is triggered                  whenever an indication or notification is sent by the remote GATT server. The triggering conditions on the GATT server side are defined by an                 upper level, for example by a profile; so it is possible that no values are ever received, or that it may take time, depending on how the server is configured.    
*
**/

static inline struct gecko_msg_gatt_set_characteristic_notification_rsp_t* gecko_cmd_gatt_set_characteristic_notification(uint8 connection,uint16 characteristic,uint8 flags)
{
    
    gecko_cmd_msg->data.cmd_gatt_set_characteristic_notification.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_set_characteristic_notification.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_set_characteristic_notification.flags=flags;
    gecko_cmd_msg->header=((gecko_cmd_gatt_set_characteristic_notification_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_set_characteristic_notification;
}

/** 
*
* gecko_cmd_gatt_discover_descriptors
*
* This command can be used to discover all the descriptors of the specified remote GATT characteristics in a remote GATT database. This command generates a unique gatt_descriptor event for every discovered descriptor. Received "gatt_procedure_completed" event indicates that this GATT procedure has succesfully completed or failed with error. 
*
* @param connection   
* @param characteristic   
*
* Events generated
*
* gecko_evt_gatt_descriptor - Discovered descriptor from remote GATT database.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_discover_descriptors_rsp_t* gecko_cmd_gatt_discover_descriptors(uint8 connection,uint16 characteristic)
{
    
    gecko_cmd_msg->data.cmd_gatt_discover_descriptors.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_discover_descriptors.characteristic=characteristic;
    gecko_cmd_msg->header=((gecko_cmd_gatt_discover_descriptors_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_discover_descriptors;
}

/** 
*
* gecko_cmd_gatt_read_characteristic_value
*
* This command can be used to read the value of a characteristic from a remote GATT database.              A single "gatt_characteristic_value" event is generated if the              characteristic value fits in one ATT PDU. Otherwise more than one "             gatt_characteristic_value" events are generated because the firmware will automatically use the "read long"              GATT procedure. A received "gatt_procedure_completed" event indicates             that all data has been read successfully or that an error response has been received.             
* Note that the GATT client does not verify if the requested atrribute
* is a characteristic value.             Thus before calling this
* command the application should make sure the attribute handle is for a
* characteristic             value in some means, for example, by
* performing characteristic discovery.
*  
*
* @param connection   
* @param characteristic   
*
* Events generated
*
* gecko_evt_gatt_characteristic_value - This event contains the data belonging to a characteristic sent by the GATT Server.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_read_characteristic_value_rsp_t* gecko_cmd_gatt_read_characteristic_value(uint8 connection,uint16 characteristic)
{
    
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value.characteristic=characteristic;
    gecko_cmd_msg->header=((gecko_cmd_gatt_read_characteristic_value_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_read_characteristic_value;
}

/** 
*
* gecko_cmd_gatt_read_characteristic_value_by_uuid
*
* This command can be used to read the characteristic value of a service from a remote GATT database             by giving the UUID of the characteristic and the handle of the service containing this characteristic. A single             "gatt_characteristic_value" event is generated if the characteristic             value fits in one ATT PDU. Otherwise more than one "             gatt_characteristic_value" events are generated because the firmware will automatically use the "read long" GATT procedure.              A received "gatt_procedure_completed" event indicates that all data has been read successfully or that an error response has been received. 
*
* @param connection   
* @param service   
* @param uuid   Characteristic UUID
*
* Events generated
*
* gecko_evt_gatt_characteristic_value - This event contains the data belonging to a characteristic sent by the GATT Server.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_read_characteristic_value_by_uuid_rsp_t* gecko_cmd_gatt_read_characteristic_value_by_uuid(uint8 connection,uint32 service,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 6)
    {
        gecko_rsp_msg->data.rsp_gatt_read_characteristic_value_by_uuid.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_read_characteristic_value_by_uuid;
    }

    
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_by_uuid.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_by_uuid.service=service;
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_by_uuid.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_by_uuid.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_read_characteristic_value_by_uuid_id+((6+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_read_characteristic_value_by_uuid;
}

/** 
*
* gecko_cmd_gatt_write_characteristic_value
*
* This command can be used to write the value of a characteristic in a remote GATT database. If the given value does not fit in one ATT PDU, "write long" GATT procedure is used automatically. Received              "gatt_procedure_completed" event indicates that all data has been written successfully or that an error response              has been received. 
*
* @param connection   
* @param characteristic   
* @param value   Characteristic value
*
* Events generated
*
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_write_characteristic_value_rsp_t* gecko_cmd_gatt_write_characteristic_value(uint8 connection,uint16 characteristic,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_gatt_write_characteristic_value.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_write_characteristic_value;
    }

    
    gecko_cmd_msg->data.cmd_gatt_write_characteristic_value.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_write_characteristic_value.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_write_characteristic_value.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_write_characteristic_value.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_write_characteristic_value_id+((4+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_write_characteristic_value;
}

/** 
*
* gecko_cmd_gatt_write_characteristic_value_without_response
*
* This command can be used to write the value of a characteristic in a             remote GATT server. This command does not generate any event. All failures on the server are ignored silently.              For example, if an error is generated in the remote GATT server and the given value is not written into database no error message will be reported to the local              GATT client. Note that this command cannot be used to write long values. At most ATT_MTU - 3 amount of data can be sent once. 
*
* @param connection   
* @param characteristic   
* @param value   Characteristic value    
*
**/

static inline struct gecko_msg_gatt_write_characteristic_value_without_response_rsp_t* gecko_cmd_gatt_write_characteristic_value_without_response(uint8 connection,uint16 characteristic,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_gatt_write_characteristic_value_without_response.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_write_characteristic_value_without_response;
    }

    
    gecko_cmd_msg->data.cmd_gatt_write_characteristic_value_without_response.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_write_characteristic_value_without_response.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_write_characteristic_value_without_response.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_write_characteristic_value_without_response.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_write_characteristic_value_without_response_id+((4+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_write_characteristic_value_without_response;
}

/** 
*
* gecko_cmd_gatt_prepare_characteristic_value_write
*
* This command can be used to add a characteristic value to the write
* queue of a remote GATT server.              This command can be used
* in cases where very long attributes need to be written, or a set of
* values needs to be written atomically. At most ATT_MTU - 5 amount of
* data can be sent once. Writes are executed or cancelled with the
* "execute_characteristic_value_write" command.             Whether the
* writes succeeded or not are indicated in the response of the
* "execute_characteristic_value_write" command.
* In all cases where the amount of data to transfer fits into the BGAPI
* payload the command              "gatt_write_characteristic_value" is
* recommended for writing long values since it transparently
* performs the prepare_write and execute_write commands.
*  
*
* @param connection   
* @param characteristic   
* @param offset   Offset of the characteristic value
* @param value   Value to write into the specified characteristic of the remote GATT database
*
* Events generated
*
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_prepare_characteristic_value_write_rsp_t* gecko_cmd_gatt_prepare_characteristic_value_write(uint8 connection,uint16 characteristic,uint16 offset,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 6)
    {
        gecko_rsp_msg->data.rsp_gatt_prepare_characteristic_value_write.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_prepare_characteristic_value_write;
    }

    
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_write.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_write.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_write.offset=offset;
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_write.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_write.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_prepare_characteristic_value_write_id+((6+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_prepare_characteristic_value_write;
}

/** 
*
* gecko_cmd_gatt_execute_characteristic_value_write
*
* This command can be used to commit or cancel previously queued writes to a long characteristic of a remote GATT server.             Writes are sent to queue with "prepare_characteristic_value_write" command.              Content, offset and length of queued values are validated by this procedure. A received "gatt_procedure_completed"             event indicates that all data has been written successfully or that an error response has been received.              
*
* @param connection   
* @param flags   
*
* Events generated
*
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_execute_characteristic_value_write_rsp_t* gecko_cmd_gatt_execute_characteristic_value_write(uint8 connection,uint8 flags)
{
    
    gecko_cmd_msg->data.cmd_gatt_execute_characteristic_value_write.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_execute_characteristic_value_write.flags=flags;
    gecko_cmd_msg->header=((gecko_cmd_gatt_execute_characteristic_value_write_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_execute_characteristic_value_write;
}

/** 
*
* gecko_cmd_gatt_send_characteristic_confirmation
*
* This command must be used to send a characteristic confirmation to a remote GATT server after receiving an indication.              The "gatt_characteristic_value_event" carries the att_opcode containing handle_value_indication (0x1d) which reveals              that an indication has been received and this must be confirmed with this command. Confirmation needs to be sent within 30 seconds, otherwise the GATT transactions              between the client and the server are discontinued. 
*
* @param connection       
*
**/

static inline struct gecko_msg_gatt_send_characteristic_confirmation_rsp_t* gecko_cmd_gatt_send_characteristic_confirmation(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_gatt_send_characteristic_confirmation.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_gatt_send_characteristic_confirmation_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_send_characteristic_confirmation;
}

/** 
*
* gecko_cmd_gatt_read_descriptor_value
*
* This command can be used to read the descriptor value of a characteristic in a remote GATT database. A single "             gatt_descriptor_value" event is generated if the descriptor value fits in one ATT PDU. Otherwise more than one "gatt_descriptor_value" events are generated because the firmware              will automatically use the "read long" GATT procedure. A received "gatt_procedure_completed" event indicates that all              data has been read successfully or that an error response has been received. 
*
* @param connection   
* @param descriptor   
*
* Events generated
*
* gecko_evt_gatt_descriptor_value - Descriptor value received from the remote GATT server.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_read_descriptor_value_rsp_t* gecko_cmd_gatt_read_descriptor_value(uint8 connection,uint16 descriptor)
{
    
    gecko_cmd_msg->data.cmd_gatt_read_descriptor_value.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_read_descriptor_value.descriptor=descriptor;
    gecko_cmd_msg->header=((gecko_cmd_gatt_read_descriptor_value_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_read_descriptor_value;
}

/** 
*
* gecko_cmd_gatt_write_descriptor_value
*
* This command can be used to write the value of a characteristic descriptor in a remote GATT database. If the given value does not fit in one ATT PDU, "write long" GATT procedure is used automatically. Received "gatt_procedure_completed"              event indicates that all data has been written succesfully or that an error response has been received.              
*
* @param connection   
* @param descriptor   
* @param value   Descriptor value
*
* Events generated
*
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_write_descriptor_value_rsp_t* gecko_cmd_gatt_write_descriptor_value(uint8 connection,uint16 descriptor,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_gatt_write_descriptor_value.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_write_descriptor_value;
    }

    
    gecko_cmd_msg->data.cmd_gatt_write_descriptor_value.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_write_descriptor_value.descriptor=descriptor;
    gecko_cmd_msg->data.cmd_gatt_write_descriptor_value.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_write_descriptor_value.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_write_descriptor_value_id+((4+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_write_descriptor_value;
}

/** 
*
* gecko_cmd_gatt_find_included_services
*
* This command can be used to find out if a service of a remote GATT database includes one or more other services. This command generates a unique gatt_service_completed event for each included service. This command generates a unique gatt_service event for every discovered service. Received "gatt_procedure_completed" event indicates that this GATT procedure has successfully completed or failed with error. 
*
* @param connection   
* @param service   
*
* Events generated
*
* gecko_evt_gatt_service - Discovered service from remote GATT database.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_find_included_services_rsp_t* gecko_cmd_gatt_find_included_services(uint8 connection,uint32 service)
{
    
    gecko_cmd_msg->data.cmd_gatt_find_included_services.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_find_included_services.service=service;
    gecko_cmd_msg->header=((gecko_cmd_gatt_find_included_services_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_find_included_services;
}

/** 
*
* gecko_cmd_gatt_read_multiple_characteristic_values
*
* This command can be used to read the values of multiple
* characteristics from a remote GATT database at once.              The
* GATT server will return the values in one ATT PDU as the response. If
* the total set of values is greater than             (ATT_MTU - 1)
* bytes in length, only the first (ATT_MTU - 1) bytes are included. A
* single             "gatt_characteristic_value" event is generated, in
* which the             characteristic is set to 0 and the data in value
* parameter is a concatenation of characteristic values in the order
* they were requested.             Received "gatt_procedure_completed"
* event indicates that this GATT procedure              has successfully
* completed or failed with error.
* This command should be used only for characteristics values that have
* known fixed size, except the last one that could have variable length.
*  
*
* @param connection   
* @param characteristic_list   Little endian encoded uint16 list of characteristics to be read.
*
* Events generated
*
* gecko_evt_gatt_characteristic_value - A concatenation of characteristic values in the order they were requested
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_read_multiple_characteristic_values_rsp_t* gecko_cmd_gatt_read_multiple_characteristic_values(uint8 connection,uint8 characteristic_list_len, const uint8* characteristic_list_data)
{
    if ((uint16_t)characteristic_list_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_gatt_read_multiple_characteristic_values.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_read_multiple_characteristic_values;
    }

    
    gecko_cmd_msg->data.cmd_gatt_read_multiple_characteristic_values.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_read_multiple_characteristic_values.characteristic_list.len=characteristic_list_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_read_multiple_characteristic_values.characteristic_list.data,characteristic_list_data,characteristic_list_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_read_multiple_characteristic_values_id+((2+characteristic_list_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_read_multiple_characteristic_values;
}

/** 
*
* gecko_cmd_gatt_read_characteristic_value_from_offset
*
* This command can be used to read a partial characteristic value with specified offset and maximum length              from a remote GATT database. It is equivalent to             "gatt_read_characteristic_value"             if both the offset and maximum length parameters are 0.             A single "gatt_characteristic_value" event is generated              if the value to read fits in one ATT PDU. Otherwise more than one "             gatt_characteristic_value" events are generated because the firmware will automatically use the "read long"              GATT procedure. A received "gatt_procedure_completed" event indicates             that all data has been read successfully or that an error response has been received. 
*
* @param connection   
* @param characteristic   
* @param offset   Offset of the characteristic value
* @param maxlen   Maximum bytes to read. If this parameter is 0 all characteristic value starting at given offset will be read.                     
*
* Events generated
*
* gecko_evt_gatt_characteristic_value - This event contains the data belonging to a characteristic sent by the GATT Server.
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_read_characteristic_value_from_offset_rsp_t* gecko_cmd_gatt_read_characteristic_value_from_offset(uint8 connection,uint16 characteristic,uint16 offset,uint16 maxlen)
{
    
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_from_offset.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_from_offset.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_from_offset.offset=offset;
    gecko_cmd_msg->data.cmd_gatt_read_characteristic_value_from_offset.maxlen=maxlen;
    gecko_cmd_msg->header=((gecko_cmd_gatt_read_characteristic_value_from_offset_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_read_characteristic_value_from_offset;
}

/** 
*
* gecko_cmd_gatt_prepare_characteristic_value_reliable_write
*
* This command can be used to add a characteristic value to the write
* queue of a remote GATT server and verify if the value was correctly
* received by the server. Received "gatt_procedure_completed" event
* indicates that this GATT procedure has succesfully completed or failed
* with error. Specifically, error code 0x0194 (data_corrupted) will be
* returned if the value received from the GATT server's response failed
* to pass the reliable write verification. At most ATT_MTU - 5 amount of
* data can be sent once. Writes are executed or cancelled with the
* "execute_characteristic_value_write" command.             Whether the
* writes succeeded or not are indicated in the response of the
* "execute_characteristic_value_write" command.
*  
*
* @param connection   
* @param characteristic   
* @param offset   Offset of the characteristic value
* @param value   Value to write into the specified characteristic of the remote GATT database
*
* Events generated
*
* gecko_evt_gatt_procedure_completed - Procedure has been successfully completed or failed with error.    
*
**/

static inline struct gecko_msg_gatt_prepare_characteristic_value_reliable_write_rsp_t* gecko_cmd_gatt_prepare_characteristic_value_reliable_write(uint8 connection,uint16 characteristic,uint16 offset,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 6)
    {
        gecko_rsp_msg->data.rsp_gatt_prepare_characteristic_value_reliable_write.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_prepare_characteristic_value_reliable_write;
    }

    
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_reliable_write.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_reliable_write.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_reliable_write.offset=offset;
    gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_reliable_write.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_prepare_characteristic_value_reliable_write.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_prepare_characteristic_value_reliable_write_id+((6+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_prepare_characteristic_value_reliable_write;
}

/** 
*
* gecko_cmd_gatt_server_read_attribute_value
*
* This command can be used to read the value of an attribute from a local GATT database. Only (maximum BGAPI payload size - 3) amount of data can be read once. The application can continue reading with increased offset value if it receives (maximum BGAPI payload size - 3) amount of data. 
*
* @param attribute   Attribute handle
* @param offset   Value offset    
*
**/

static inline struct gecko_msg_gatt_server_read_attribute_value_rsp_t* gecko_cmd_gatt_server_read_attribute_value(uint16 attribute,uint16 offset)
{
    
    gecko_cmd_msg->data.cmd_gatt_server_read_attribute_value.attribute=attribute;
    gecko_cmd_msg->data.cmd_gatt_server_read_attribute_value.offset=offset;
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_read_attribute_value_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_read_attribute_value;
}

/** 
*
* gecko_cmd_gatt_server_read_attribute_type
*
* This command can be used to read the type of an attribute from a local GATT database. The type is a UUID, usually 16 or 128 bits long. 
*
* @param attribute   Attribute handle    
*
**/

static inline struct gecko_msg_gatt_server_read_attribute_type_rsp_t* gecko_cmd_gatt_server_read_attribute_type(uint16 attribute)
{
    
    gecko_cmd_msg->data.cmd_gatt_server_read_attribute_type.attribute=attribute;
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_read_attribute_type_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_read_attribute_type;
}

/** 
*
* gecko_cmd_gatt_server_write_attribute_value
*
* This command can be used to write the value of an attribute in the local GATT database. Writing the value of a characteristic of the local GATT database              will not trigger notifications or indications to the remote GATT client in case such characteristic has property of indicate or notify and the client has enabled              notification or indication. Notifications and indications are sent to the remote GATT client using "             gatt_server_send_characteristic_notification" command. 
*
* @param attribute   Attribute handle
* @param offset   Value offset
* @param value   Value    
*
**/

static inline struct gecko_msg_gatt_server_write_attribute_value_rsp_t* gecko_cmd_gatt_server_write_attribute_value(uint16 attribute,uint16 offset,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 5)
    {
        gecko_rsp_msg->data.rsp_gatt_server_write_attribute_value.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_server_write_attribute_value;
    }

    
    gecko_cmd_msg->data.cmd_gatt_server_write_attribute_value.attribute=attribute;
    gecko_cmd_msg->data.cmd_gatt_server_write_attribute_value.offset=offset;
    gecko_cmd_msg->data.cmd_gatt_server_write_attribute_value.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_server_write_attribute_value.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_write_attribute_value_id+((5+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_write_attribute_value;
}

/** 
*
* gecko_cmd_gatt_server_send_user_read_response
*
* This command must be used to send a response to a "user_read_request" event. The response needs to be sent within 30 second, otherwise no more GATT              transactions are allowed by the remote side. If attr_errorcode is set to 0 the characteristic value is sent to the remote GATT client in the normal way. Other attr_errorcode              values will cause the local GATT server to send an attribute protocol error response instead of the actual data.             At most ATT_MTU - 1 amount of data can be sent once. Client will continue reading by sending new read request with             increased offset value if it receives ATT_MTU - 1 amount of data. 
*
* @param connection   
* @param characteristic   
* @param att_errorcode   
* @param value   Characteristic value to send to the GATT client. Ignored if att_errorcode is not 0.    
*
**/

static inline struct gecko_msg_gatt_server_send_user_read_response_rsp_t* gecko_cmd_gatt_server_send_user_read_response(uint8 connection,uint16 characteristic,uint8 att_errorcode,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 5)
    {
        gecko_rsp_msg->data.rsp_gatt_server_send_user_read_response.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_server_send_user_read_response;
    }

    
    gecko_cmd_msg->data.cmd_gatt_server_send_user_read_response.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_server_send_user_read_response.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_server_send_user_read_response.att_errorcode=att_errorcode;
    gecko_cmd_msg->data.cmd_gatt_server_send_user_read_response.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_server_send_user_read_response.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_send_user_read_response_id+((5+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_send_user_read_response;
}

/** 
*
* gecko_cmd_gatt_server_send_user_write_response
*
* This command must be used to send a response to a "gatt_server_user_write_request" event when parameter att_opcode in the event is Write Request (see "att_opcode"). The response needs to be sent within 30 seconds, otherwise no more GATT              transactions are allowed by the remote side. If attr_errorcode is set to 0 the ATT protocol's write response is sent to indicate to the remote GATT client that              the write operation was processed successfully. Other values will cause the local GATT server to send an ATT protocol error response. 
*
* @param connection   
* @param characteristic   
* @param att_errorcode       
*
**/

static inline struct gecko_msg_gatt_server_send_user_write_response_rsp_t* gecko_cmd_gatt_server_send_user_write_response(uint8 connection,uint16 characteristic,uint8 att_errorcode)
{
    
    gecko_cmd_msg->data.cmd_gatt_server_send_user_write_response.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_server_send_user_write_response.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_server_send_user_write_response.att_errorcode=att_errorcode;
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_send_user_write_response_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_send_user_write_response;
}

/** 
*
* gecko_cmd_gatt_server_send_characteristic_notification
*
* This command can be used to send notifications or indications to one or more remote GATT clients. At most ATT_MTU - 3 amount of data can be sent once.             
* A notification or indication is sent only if the client has enabled
* it by setting the corresponding flag to the Client Characteristic
* Configuration descriptor. In case the Client Characteristic
* Configuration descriptor supports both notification and indication,
* the stack will always send a notification even when the client has
* enabled both.
* A new indication to a GATT client cannot be sent until an outstanding
* indication procedure with the same client has completed. The procedure
* is completed when a confirmation from the client has been received.
* The confirmation is indicated              by
* "gatt_server_characteristic_status event".
* Error bg_err_wrong_state is returned if the characteristic does not
* have notification property, or if the client has not enabled the
* notification.             The same applies to indication property, and
* in addition, bg_err_wrong_state is returned if an indication procedure
* with the same client is outstanding.
*  
*
* @param connection   Handle of the connection over which the notification or indication is sent. Values: 
*  - 0xff: Sends notification or indication to all connected devices.
*  - Other: Connection handle
* 
* @param characteristic   Characteristic handle
* @param value   Value to be notified or indicated    
*
**/

static inline struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t* gecko_cmd_gatt_server_send_characteristic_notification(uint8 connection,uint16 characteristic,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_gatt_server_send_characteristic_notification.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_server_send_characteristic_notification;
    }

    
    gecko_cmd_msg->data.cmd_gatt_server_send_characteristic_notification.connection=connection;
    gecko_cmd_msg->data.cmd_gatt_server_send_characteristic_notification.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_gatt_server_send_characteristic_notification.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_server_send_characteristic_notification.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_send_characteristic_notification_id+((4+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_send_characteristic_notification;
}

/** 
*
* gecko_cmd_gatt_server_find_attribute
*
* This command can be used to find attributes of certain type from a local GATT database. Type is usually given as 16-bit or 128-bit UUID. 
*
* @param start   Search start handle
* @param type       
*
**/

static inline struct gecko_msg_gatt_server_find_attribute_rsp_t* gecko_cmd_gatt_server_find_attribute(uint16 start,uint8 type_len, const uint8* type_data)
{
    if ((uint16_t)type_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_gatt_server_find_attribute.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_gatt_server_find_attribute;
    }

    
    gecko_cmd_msg->data.cmd_gatt_server_find_attribute.start=start;
    gecko_cmd_msg->data.cmd_gatt_server_find_attribute.type.len=type_len;
    memcpy(gecko_cmd_msg->data.cmd_gatt_server_find_attribute.type.data,type_data,type_len);
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_find_attribute_id+((3+type_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_find_attribute;
}

/** 
*
* gecko_cmd_gatt_server_set_capabilities
*
* This command can be used to set which capabilities should be enabled in the local GATT database. A service is visible to remote GATT clients if at least one of its capabilities has been enabled. The same applies to a characteristic and its attributes. Capability identifiers and their corresponding bit flag values can be found in the auto-generated database header file. See UG118 for how to declare capabilities in GATT database.                 
* Changing the capabilities of a database effectively causes a database
* change (attributes being added or removed) from a remote GATT client
* point of view. If the database has a Generic Attribute service and
* Service Changed characteristic, the stack will monitor local database
* change status and manage service changed indications for a GATT client
* that has enabled the indication configuration of the Service Changed
* characteristic.
*  
*
* @param caps   Bit flags of capabilities to enable. Value 0 sets the default database capabilities.
* @param reserved   Value 0 should be used on this reserved field. None-zero values are reserved for future, do not use now.    
*
**/

static inline struct gecko_msg_gatt_server_set_capabilities_rsp_t* gecko_cmd_gatt_server_set_capabilities(uint32 caps,uint32 reserved)
{
    
    gecko_cmd_msg->data.cmd_gatt_server_set_capabilities.caps=caps;
    gecko_cmd_msg->data.cmd_gatt_server_set_capabilities.reserved=reserved;
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_set_capabilities_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_set_capabilities;
}

/** 
*
* gecko_cmd_gatt_server_set_max_mtu
*
* This command can be used to set the maximum size of ATT Message Transfer Units (MTU).             Functionality is the same as "gatt_set_max_mtu", and this setting applies to both GATT client and server.             If the given value is too large according to the maximum BGAPI payload size, the system will select the maximal possible             value as the maximum ATT_MTU. If maximum ATT_MTU is larger than 23, the GATT client in stack will automatically             send an MTU exchange request after a Bluetooth connection has been established.              
*
* @param max_mtu   Maximum size of Message Transfer Units (MTU) allowed
*  - Range:  23 to 250
* Default: 247    
*
**/

static inline struct gecko_msg_gatt_server_set_max_mtu_rsp_t* gecko_cmd_gatt_server_set_max_mtu(uint16 max_mtu)
{
    
    gecko_cmd_msg->data.cmd_gatt_server_set_max_mtu.max_mtu=max_mtu;
    gecko_cmd_msg->header=((gecko_cmd_gatt_server_set_max_mtu_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_gatt_server_set_max_mtu;
}

/** 
*
* gecko_cmd_hardware_set_soft_timer
*
* This command can be used to start a software timer. Multiple concurrent timers can be running simultaneously. There are 256 unique timer IDs available.             The maximum number of concurrent timers is configurable at device initialization. Up to 16 concurrent timers can be configured. The default configuration is 4. As the RAM for storing timer data is pre-allocated at initialization, an application should not configure the amount more than it needs for minimizing RAM usage. 
*
* @param time   Interval between how often to send events, in hardware clock ticks (1 second is equal to 32768 ticks).                 
* The smallest interval value supported is 328 which is around 10
* milliseconds, any parameters between 0 and 328 will be rounded up to
* 328.                 The maximum value is 2147483647, which
* corresponds to about 18.2 hours.
* If time is 0, removes the scheduled timer with the same handle.
* @param handle   Timer handle to use, is returned in timeout event
* @param single_shot   Timer mode. Values: 
*  - 0: false (timer is repeating)
*  - 1: true (timer runs only once)
* 
*
* Events generated
*
* gecko_evt_hardware_soft_timer - Sent after specified interval    
*
**/

static inline struct gecko_msg_hardware_set_soft_timer_rsp_t* gecko_cmd_hardware_set_soft_timer(uint32 time,uint8 handle,uint8 single_shot)
{
    
    gecko_cmd_msg->data.cmd_hardware_set_soft_timer.time=time;
    gecko_cmd_msg->data.cmd_hardware_set_soft_timer.handle=handle;
    gecko_cmd_msg->data.cmd_hardware_set_soft_timer.single_shot=single_shot;
    gecko_cmd_msg->header=((gecko_cmd_hardware_set_soft_timer_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_hardware_set_soft_timer;
}

/** 
*
* gecko_cmd_hardware_get_time
*
* Deprecated. Get elapsed time since last reset of RTCC 
*    
*
**/

static inline struct gecko_msg_hardware_get_time_rsp_t* gecko_cmd_hardware_get_time()
{
    
    gecko_cmd_msg->header=((gecko_cmd_hardware_get_time_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_hardware_get_time;
}

/** 
*
* gecko_cmd_hardware_set_lazy_soft_timer
*
* This command can be used to start a software timer with some slack. Slack parameter allows stack to optimize wake ups and save power. Timer event is triggered between time and time + slack. See also description of "hardware_set_soft_timer" command. 
*
* @param time   Interval between how often to send events, in hardware clock ticks (1 second is equal to 32768 ticks).                 
* The smallest interval value supported is 328 which is around 10
* milliseconds, any parameters between 0 and 328 will be rounded up to
* 328.                 The maximum value is 2147483647, which
* corresponds to about 18.2 hours.
* If time is 0, removes the scheduled timer with the same handle.
* @param slack   Slack time in hardware clock ticks
* @param handle   Timer handle to use, is returned in timeout event
* @param single_shot   Timer mode. Values: 
*  - 0: false (timer is repeating)
*  - 1: true (timer runs only once)
* 
*
* Events generated
*
* gecko_evt_hardware_soft_timer - Sent after specified interval    
*
**/

static inline struct gecko_msg_hardware_set_lazy_soft_timer_rsp_t* gecko_cmd_hardware_set_lazy_soft_timer(uint32 time,uint32 slack,uint8 handle,uint8 single_shot)
{
    
    gecko_cmd_msg->data.cmd_hardware_set_lazy_soft_timer.time=time;
    gecko_cmd_msg->data.cmd_hardware_set_lazy_soft_timer.slack=slack;
    gecko_cmd_msg->data.cmd_hardware_set_lazy_soft_timer.handle=handle;
    gecko_cmd_msg->data.cmd_hardware_set_lazy_soft_timer.single_shot=single_shot;
    gecko_cmd_msg->header=((gecko_cmd_hardware_set_lazy_soft_timer_id+((10)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_hardware_set_lazy_soft_timer;
}

/** 
*
* gecko_cmd_flash_ps_erase_all
*
* This command can be used to erase all PS keys and their corresponding values. 
*    
*
**/

static inline struct gecko_msg_flash_ps_erase_all_rsp_t* gecko_cmd_flash_ps_erase_all()
{
    
    gecko_cmd_msg->header=((gecko_cmd_flash_ps_erase_all_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_flash_ps_erase_all;
}

/** 
*
* gecko_cmd_flash_ps_save
*
* This command can be used to store a value into the specified PS key. Allowed PS keys are in range from 0x4000 to 0x407F. At most 56 bytes user data can be stored in one PS key. Error code 0x018a (command_too_long) will be returned if more than 56 bytes data is passed in. 
*
* @param key   PS key
* @param value   Value to store into the specified PS key.    
*
**/

static inline struct gecko_msg_flash_ps_save_rsp_t* gecko_cmd_flash_ps_save(uint16 key,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_flash_ps_save.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_flash_ps_save;
    }

    
    gecko_cmd_msg->data.cmd_flash_ps_save.key=key;
    gecko_cmd_msg->data.cmd_flash_ps_save.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_flash_ps_save.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_flash_ps_save_id+((3+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_flash_ps_save;
}

/** 
*
* gecko_cmd_flash_ps_load
*
* This command can be used for retrieving the value of the specified PS key. 
*
* @param key   PS key of the value to be retrieved    
*
**/

static inline struct gecko_msg_flash_ps_load_rsp_t* gecko_cmd_flash_ps_load(uint16 key)
{
    
    gecko_cmd_msg->data.cmd_flash_ps_load.key=key;
    gecko_cmd_msg->header=((gecko_cmd_flash_ps_load_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_flash_ps_load;
}

/** 
*
* gecko_cmd_flash_ps_erase
*
* This command can be used to erase a single PS key and its value from the persistent store. 
*
* @param key   PS key to erase    
*
**/

static inline struct gecko_msg_flash_ps_erase_rsp_t* gecko_cmd_flash_ps_erase(uint16 key)
{
    
    gecko_cmd_msg->data.cmd_flash_ps_erase.key=key;
    gecko_cmd_msg->header=((gecko_cmd_flash_ps_erase_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_flash_ps_erase;
}

/** 
*
* gecko_cmd_test_dtm_tx
*
* This command can be used to start a transmitter test. The test is meant to be used against a separate Bluetooth tester device. When the command is processed by the radio, a "test_dtm_completed" event is triggered. This event indicates if the test started successfully.                  
* In the transmitter test, the device sends packets continuously with a
* fixed interval. The type and length of each packet is set by
* packet_type and length parameters. Parameter phy specifies which PHY is used to transmit the packets. All devices support at least the 1M PHY. There is also a special packet type, test_pkt_carrier, which can be used to transmit continuous unmodulated carrier. The length field is ignored in this mode.
* The test may be stopped using the "test_dtm_end" command.
*  
*
* @param packet_type   Packet type to transmit
* @param length   Packet length in bytes
* Range: 0-255
* 
* @param channel   Bluetooth channel
* Range: 0-39
* Channel is (F - 2402) / 2,
* where F is frequency in MHz
* 
* @param phy   PHY to use
*
* Events generated
*
* gecko_evt_test_dtm_completed - This event is received when the command is processed.    
*
**/

static inline struct gecko_msg_test_dtm_tx_rsp_t* gecko_cmd_test_dtm_tx(uint8 packet_type,uint8 length,uint8 channel,uint8 phy)
{
    
    gecko_cmd_msg->data.cmd_test_dtm_tx.packet_type=packet_type;
    gecko_cmd_msg->data.cmd_test_dtm_tx.length=length;
    gecko_cmd_msg->data.cmd_test_dtm_tx.channel=channel;
    gecko_cmd_msg->data.cmd_test_dtm_tx.phy=phy;
    gecko_cmd_msg->header=((gecko_cmd_test_dtm_tx_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_test_dtm_tx;
}

/** 
*
* gecko_cmd_test_dtm_rx
*
* This command can be used to start a receiver test. The test is meant to be used against a separate Bluetooth tester device. When the command is processed by the radio, a "test_dtm_completed" event is triggered. This event indicates if the test started successfully.                  
* Parameter
* phy specifies which PHY is used to receive the packets. All devices support at least the 1M PHY.
* The test may be stopped using the "test_dtm_end" command. This will
* trigger another "test_dtm_completed" event, which carries the number
* of packets received during the test.
*  
*
* @param channel   Bluetooth channel
* Range: 0-39
* Channel is (F - 2402) / 2,
* where F is frequency in MHz
* 
* @param phy   PHY to use
*
* Events generated
*
* gecko_evt_test_dtm_completed - This event is received when the command is processed.    
*
**/

static inline struct gecko_msg_test_dtm_rx_rsp_t* gecko_cmd_test_dtm_rx(uint8 channel,uint8 phy)
{
    
    gecko_cmd_msg->data.cmd_test_dtm_rx.channel=channel;
    gecko_cmd_msg->data.cmd_test_dtm_rx.phy=phy;
    gecko_cmd_msg->header=((gecko_cmd_test_dtm_rx_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_test_dtm_rx;
}

/** 
*
* gecko_cmd_test_dtm_end
*
* This command can be used to end a transmitter or a receiver test. When the command is processed by the radio and the test has ended, a "test_dtm_completed" event is triggered.              
*
*
* Events generated
*
* gecko_evt_test_dtm_completed - This event is received when the command is processed.    
*
**/

static inline struct gecko_msg_test_dtm_end_rsp_t* gecko_cmd_test_dtm_end()
{
    
    gecko_cmd_msg->header=((gecko_cmd_test_dtm_end_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_test_dtm_end;
}

/** 
*
* gecko_cmd_sm_set_bondable_mode
*
* This command can be used to set whether the device should accept new bondings. By default, the device does not accept new bondings. 
*
* @param bondable   Bondable mode. Values:                    
*  - 0: New bondings not accepted
*  - 1: Bondings allowed
* Default value: 0
*     
*
**/

static inline struct gecko_msg_sm_set_bondable_mode_rsp_t* gecko_cmd_sm_set_bondable_mode(uint8 bondable)
{
    
    gecko_cmd_msg->data.cmd_sm_set_bondable_mode.bondable=bondable;
    gecko_cmd_msg->header=((gecko_cmd_sm_set_bondable_mode_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_set_bondable_mode;
}

/** 
*
* gecko_cmd_sm_configure
*
* This command can be used to configure security requirements and I/O capabilities of the system. 
*
* @param flags   Security requirement bitmask.                     
* Bit 0:
*  - 0: Allow bonding without MITM protection
*  - 1: Bonding requires MITM protection
* Bit 1:
*  - 0: Allow encryption without bonding
*  - 1: Encryption requires bonding. Note that this setting will also enable bonding.
* Bit 2:
*  - 0: Allow bonding with legacy pairing
*  - 1: Secure connections only
* Bit 3:
*  - 0: Bonding request does not need to be confirmed
*  - 1: Bonding requests need to be confirmed. Received bonding requests are notified with "sm_confirm_bonding events."
* Bit 4:
*  - 0: Allow all connections
*  - 1: Allow connections only from bonded devices
* Bit 5 to 7: Reserved
* Default value: 0x00
* 
* @param io_capabilities   I/O Capabilities. See link    
*
**/

static inline struct gecko_msg_sm_configure_rsp_t* gecko_cmd_sm_configure(uint8 flags,uint8 io_capabilities)
{
    
    gecko_cmd_msg->data.cmd_sm_configure.flags=flags;
    gecko_cmd_msg->data.cmd_sm_configure.io_capabilities=io_capabilities;
    gecko_cmd_msg->header=((gecko_cmd_sm_configure_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_configure;
}

/** 
*
* gecko_cmd_sm_store_bonding_configuration
*
* This command can be used to set maximum allowed bonding count and bonding policy. The actual maximum number of bondings that can be supported depends on how much user data is stored in the NVM and the NVM size. The default value is 14. 
*
* @param max_bonding_count   Maximum allowed bonding count. Range: 1 to 32
* @param policy_flags   Bonding policy. Values: 
*  - 0: If database is full, new bonding attempts will fail
*  - 1: New bonding will overwrite the oldest existing bonding
*  - 2: New bonding will overwrite longest time ago used existing bonding
*     
*
**/

static inline struct gecko_msg_sm_store_bonding_configuration_rsp_t* gecko_cmd_sm_store_bonding_configuration(uint8 max_bonding_count,uint8 policy_flags)
{
    
    gecko_cmd_msg->data.cmd_sm_store_bonding_configuration.max_bonding_count=max_bonding_count;
    gecko_cmd_msg->data.cmd_sm_store_bonding_configuration.policy_flags=policy_flags;
    gecko_cmd_msg->header=((gecko_cmd_sm_store_bonding_configuration_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_store_bonding_configuration;
}

/** 
*
* gecko_cmd_sm_increase_security
*
* This command can be used to enhance the security of a connection to current security requirements. On an unencrypted connection, this will encrypt the connection and will also perform bonding if requested by both devices. On an encrypted connection, this will cause the connection re-encrypted. 
*
* @param connection   Connection handle
*
* Events generated
*
* gecko_evt_le_connection_parameters - This event is triggered after increasing security has been completed successfully, and indicates the latest security mode of the connection.
* gecko_evt_sm_bonded - This event is triggered if pairing or bonding was performed in this operation and the result is success.
* gecko_evt_sm_bonding_failed - This event is triggered if pairing or bonding was performed in this operation and the result is failure.    
*
**/

static inline struct gecko_msg_sm_increase_security_rsp_t* gecko_cmd_sm_increase_security(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_sm_increase_security.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_sm_increase_security_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_increase_security;
}

/** 
*
* gecko_cmd_sm_delete_bonding
*
* This command can be used to delete specified bonding information or whitelist from Persistent Store. 
*
* @param bonding   Bonding handle    
*
**/

static inline struct gecko_msg_sm_delete_bonding_rsp_t* gecko_cmd_sm_delete_bonding(uint8 bonding)
{
    
    gecko_cmd_msg->data.cmd_sm_delete_bonding.bonding=bonding;
    gecko_cmd_msg->header=((gecko_cmd_sm_delete_bonding_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_delete_bonding;
}

/** 
*
* gecko_cmd_sm_delete_bondings
*
* This command can be used to delete all bonding information and whitelist from Persistent Store. 
*    
*
**/

static inline struct gecko_msg_sm_delete_bondings_rsp_t* gecko_cmd_sm_delete_bondings()
{
    
    gecko_cmd_msg->header=((gecko_cmd_sm_delete_bondings_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_delete_bondings;
}

/** 
*
* gecko_cmd_sm_enter_passkey
*
* This command can be used to enter a passkey after receiving a passkey request event. 
*
* @param connection   Connection handle
* @param passkey   Passkey. Valid range: 0-999999. Set -1 to cancel pairing.    
*
**/

static inline struct gecko_msg_sm_enter_passkey_rsp_t* gecko_cmd_sm_enter_passkey(uint8 connection,int32 passkey)
{
    
    gecko_cmd_msg->data.cmd_sm_enter_passkey.connection=connection;
    gecko_cmd_msg->data.cmd_sm_enter_passkey.passkey=passkey;
    gecko_cmd_msg->header=((gecko_cmd_sm_enter_passkey_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_enter_passkey;
}

/** 
*
* gecko_cmd_sm_passkey_confirm
*
* This command can be used for accepting or rejecting reported confirm value. 
*
* @param connection   Connection handle
* @param confirm   Accept confirm value. Values:
*  - 0: Reject
*  - 1: Accept confirm value
*     
*
**/

static inline struct gecko_msg_sm_passkey_confirm_rsp_t* gecko_cmd_sm_passkey_confirm(uint8 connection,uint8 confirm)
{
    
    gecko_cmd_msg->data.cmd_sm_passkey_confirm.connection=connection;
    gecko_cmd_msg->data.cmd_sm_passkey_confirm.confirm=confirm;
    gecko_cmd_msg->header=((gecko_cmd_sm_passkey_confirm_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_passkey_confirm;
}

/** 
*
* gecko_cmd_sm_set_oob_data
*
* This command can be used to set the OOB data (out-of-band encryption data) for legacy pairing for a device. The OOB data may be, for example, a PIN code exchanged over                  an alternate path like NFC. The device will not allow any other kind of bonding if OOB data is set. The OOB data cannot be set simultaneously with secure connections OOB data.               
*
* @param oob_data   OOB data. To set OOB data, send a 16-byte array. To clear OOB data, send a zero-length array.    
*
**/

static inline struct gecko_msg_sm_set_oob_data_rsp_t* gecko_cmd_sm_set_oob_data(uint8 oob_data_len, const uint8* oob_data_data)
{
    if ((uint16_t)oob_data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_sm_set_oob_data.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_sm_set_oob_data;
    }

    
    gecko_cmd_msg->data.cmd_sm_set_oob_data.oob_data.len=oob_data_len;
    memcpy(gecko_cmd_msg->data.cmd_sm_set_oob_data.oob_data.data,oob_data_data,oob_data_len);
    gecko_cmd_msg->header=((gecko_cmd_sm_set_oob_data_id+((1+oob_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_set_oob_data;
}

/** 
*
* gecko_cmd_sm_list_all_bondings
*
* This command can be used to list all bondings stored in the bonding database. Bondings are reported by using the "             sm_list_bonding_entry" event for each bonding and the report is ended with "sm_list_all_bondings_complete" event.              Recommended to be used only for debugging purposes, because reading from the Persistent Store is relatively slow. 
*
*
* Events generated
*
* gecko_evt_sm_list_bonding_entry - 
* gecko_evt_sm_list_all_bondings_complete -     
*
**/

static inline struct gecko_msg_sm_list_all_bondings_rsp_t* gecko_cmd_sm_list_all_bondings()
{
    
    gecko_cmd_msg->header=((gecko_cmd_sm_list_all_bondings_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_list_all_bondings;
}

/** 
*
* gecko_cmd_sm_bonding_confirm
*
* This command can be used for accepting or rejecting bonding request. 
*
* @param connection   Connection handle
* @param confirm   Accept bonding request. Values:
*  - 0: Reject
*  - 1: Accept bonding request
*     
*
**/

static inline struct gecko_msg_sm_bonding_confirm_rsp_t* gecko_cmd_sm_bonding_confirm(uint8 connection,uint8 confirm)
{
    
    gecko_cmd_msg->data.cmd_sm_bonding_confirm.connection=connection;
    gecko_cmd_msg->data.cmd_sm_bonding_confirm.confirm=confirm;
    gecko_cmd_msg->header=((gecko_cmd_sm_bonding_confirm_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_bonding_confirm;
}

/** 
*
* gecko_cmd_sm_set_debug_mode
*
* This command can be used to set Security Manager in debug mode. In this mode the secure connections bonding uses debug keys, so that the encrypted packet can be opened by Bluetooth protocol analyzer. To disable the debug mode, you need to restart the device. 
*    
*
**/

static inline struct gecko_msg_sm_set_debug_mode_rsp_t* gecko_cmd_sm_set_debug_mode()
{
    
    gecko_cmd_msg->header=((gecko_cmd_sm_set_debug_mode_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_set_debug_mode;
}

/** 
*
* gecko_cmd_sm_set_passkey
*
* This command can be used to enter a fixed passkey which will be used in the "sm_passkey_display" event. 
*
* @param passkey   Passkey. Valid range: 0-999999. Set -1 to disable and start using random passkeys.    
*
**/

static inline struct gecko_msg_sm_set_passkey_rsp_t* gecko_cmd_sm_set_passkey(int32 passkey)
{
    
    gecko_cmd_msg->data.cmd_sm_set_passkey.passkey=passkey;
    gecko_cmd_msg->header=((gecko_cmd_sm_set_passkey_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_set_passkey;
}

/** 
*
* gecko_cmd_sm_use_sc_oob
*
* This command can be used to enable the use of OOB data (out-of-band encryption data) for a device for secure connections pairing.                 The enabling will genarate new OOB data and confirm values which can be sent to the remote device.                 After enabling the secure connections OOB data, the remote devices OOB data can be set with "sm_set_sc_remote_oob_data".                 Calling this function will erase any set remote device OOB data and confirm values.                 The device will not allow any other kind of bonding if OOB data is set. The secure connections OOB data cannot be enabled simultaneously with legacy pairing OOB data.              
*
* @param enable   Enable OOB with secure connections pairing. Values: 
*  - 0: disable
*  - 1: enable
*     
*
**/

static inline struct gecko_msg_sm_use_sc_oob_rsp_t* gecko_cmd_sm_use_sc_oob(uint8 enable)
{
    
    gecko_cmd_msg->data.cmd_sm_use_sc_oob.enable=enable;
    gecko_cmd_msg->header=((gecko_cmd_sm_use_sc_oob_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_use_sc_oob;
}

/** 
*
* gecko_cmd_sm_set_sc_remote_oob_data
*
* This command can be used to set OOB data and confirm values (out-of-band encryption) received from the remote device for secure connections pairing.                 OOB data must be enabled with "sm_use_sc_oob" before setting the remote device OOB data.              
*
* @param oob_data   Remote device OOB data and confirm values. To set OOB data, send a 32-byte array.                         First 16-bytes is the OOB data and last 16-bytes the confirm value. To clear OOB data, send a zero-length array.    
*
**/

static inline struct gecko_msg_sm_set_sc_remote_oob_data_rsp_t* gecko_cmd_sm_set_sc_remote_oob_data(uint8 oob_data_len, const uint8* oob_data_data)
{
    if ((uint16_t)oob_data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_sm_set_sc_remote_oob_data.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_sm_set_sc_remote_oob_data;
    }

    
    gecko_cmd_msg->data.cmd_sm_set_sc_remote_oob_data.oob_data.len=oob_data_len;
    memcpy(gecko_cmd_msg->data.cmd_sm_set_sc_remote_oob_data.oob_data.data,oob_data_data,oob_data_len);
    gecko_cmd_msg->header=((gecko_cmd_sm_set_sc_remote_oob_data_id+((1+oob_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_set_sc_remote_oob_data;
}

/** 
*
* gecko_cmd_sm_add_to_whitelist
*
* This command can be used to add device to whitelist, which can be enabled with "le_gap_enable_whitelisting"              
*
* @param address   Address of the device added to whitelist
* @param address_type   Address type of the device added to whitelist    
*
**/

static inline struct gecko_msg_sm_add_to_whitelist_rsp_t* gecko_cmd_sm_add_to_whitelist(bd_addr address,uint8 address_type)
{
    
    memcpy(&gecko_cmd_msg->data.cmd_sm_add_to_whitelist.address,&address,sizeof(bd_addr));
    gecko_cmd_msg->data.cmd_sm_add_to_whitelist.address_type=address_type;
    gecko_cmd_msg->header=((gecko_cmd_sm_add_to_whitelist_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_add_to_whitelist;
}

/** 
*
* gecko_cmd_sm_set_minimum_key_size
*
* This command can be used to set the mimimun allowed key size used for bonding. The default value is 16 bytes.              
*
* @param minimum_key_size   Minimum allowed key size for bonding. Range: 7 to 16    
*
**/

static inline struct gecko_msg_sm_set_minimum_key_size_rsp_t* gecko_cmd_sm_set_minimum_key_size(uint8 minimum_key_size)
{
    
    gecko_cmd_msg->data.cmd_sm_set_minimum_key_size.minimum_key_size=minimum_key_size;
    gecko_cmd_msg->header=((gecko_cmd_sm_set_minimum_key_size_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_sm_set_minimum_key_size;
}

/** 
*
* gecko_cmd_homekit_configure
*
* This command can be used to configure the Apple HomeKit accessory and its settings. It is possible to reinitialize configuration in run time. New fast advertising parameters will be used for next fast advertising. 
*
* @param i2c_address   I2C address of Apple authentication coprocessor
* @param support_display   A flag to tell the display support is enabled in the accessory.                                  {br}A pin code will be randomly generated randomly during the pairing process and event "homekit_setupcode_display" event will be produced so the pin code can be shown on the display.                                 
*  - 0: Display support disabled
*  - 1: Display support enabled
* 
* @param hap_attribute_features   The value of Apple HomeKit pairing features supported in pairing service feature characteristic.                  
*  - 0x01: Supports Apple Authentication Coprocessor
*  - 0x02: Supports Software Authentication
*  - 0x00: Only for testing purposes when any authentication method is not available. The accessory will be discovered as non-authenticated
*  - other: Reserved
* 
* @param category   Apple HomeKit accessory category
* @param configuration_number   Apple HomeKit configuration number                                                          {br}By default, this starts from 1. Accessories must increment this value after a firmware update. This value must be managed by the application.
* @param fast_advert_interval   Fast advertising interval. {br}This is used during fast advertising in disconnected state after calling command "homekit_event_notification" when broadcast events advertising is finished.
* @param fast_advert_timeout   Fast advertising timeout. {br}This is used during fast advertising in disconnected state after calling command "homekit_event_notification" when broadcast events advertising is finished.                    
*  - Time = Value x 100 ms
* 
* @param flag   Apple HomeKit library configuration flag.                                 
*  - 0x00000001: Manual Bluetooth disconnection in HomeKit error case. When enabling a "homekit_disconnection_required" event will be produced when appear HomeKit error.
*  - 0x00000002: Manual set of scan response data. When enabling it is possible to use "le_gap_bt5_set_adv_data" command to set custom scan response data. In other case HomeKit library will use it to set accessory local name.                                  - other:  Reserved. Must be 0.
* 
* @param broadcast_advert_timeout   Broadcast events advertising timeout. {br}This is used during broadcast events advertising in disconnected state after calling command "homekit_event_notification"                    
*  - Time = Value x 100 ms
* 
* @param model_name   Model name characteristic value from HomeKit Accessory Information service. Mandatory in case of HomeKit software authentication usage.    
*
**/

static inline struct gecko_msg_homekit_configure_rsp_t* gecko_cmd_homekit_configure(uint8 i2c_address,uint8 support_display,uint8 hap_attribute_features,uint16 category,uint8 configuration_number,uint16 fast_advert_interval,uint16 fast_advert_timeout,uint32 flag,uint16 broadcast_advert_timeout,uint8 model_name_len, const uint8* model_name_data)
{
    if ((uint16_t)model_name_len > BGLIB_MSG_MAX_PAYLOAD - 17)
    {
        gecko_rsp_msg->data.rsp_homekit_configure.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_homekit_configure;
    }

    
    gecko_cmd_msg->data.cmd_homekit_configure.i2c_address=i2c_address;
    gecko_cmd_msg->data.cmd_homekit_configure.support_display=support_display;
    gecko_cmd_msg->data.cmd_homekit_configure.hap_attribute_features=hap_attribute_features;
    gecko_cmd_msg->data.cmd_homekit_configure.category=category;
    gecko_cmd_msg->data.cmd_homekit_configure.configuration_number=configuration_number;
    gecko_cmd_msg->data.cmd_homekit_configure.fast_advert_interval=fast_advert_interval;
    gecko_cmd_msg->data.cmd_homekit_configure.fast_advert_timeout=fast_advert_timeout;
    gecko_cmd_msg->data.cmd_homekit_configure.flag=flag;
    gecko_cmd_msg->data.cmd_homekit_configure.broadcast_advert_timeout=broadcast_advert_timeout;
    gecko_cmd_msg->data.cmd_homekit_configure.model_name.len=model_name_len;
    memcpy(gecko_cmd_msg->data.cmd_homekit_configure.model_name.data,model_name_data,model_name_len);
    gecko_cmd_msg->header=((gecko_cmd_homekit_configure_id+((17+model_name_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_configure;
}

/** 
*
* gecko_cmd_homekit_advertise
*
* This command can be used to start or stop Apple HomeKit accessory advertising. The command and parameters will take effect immediately. If the given parameters can't be used in the currently active mode, an error will be returned. 
*
* @param enable   A flag to enable or disable Apple HomeKit advertising                                 
*  - 1: Enable advertising
*  - 0: Disable advertising
* 
* @param interval_min   Minimum advertising interval. Please refer to GAP command: "le_gap_set_adv_parameters"
* @param interval_max   Maximum advertising interval. Please refer to GAP command: "le_gap_set_adv_parameters"
* @param channel_map   Advertising channel map. Please refer to GAP command: "le_gap_set_adv_parameters"    
*
**/

static inline struct gecko_msg_homekit_advertise_rsp_t* gecko_cmd_homekit_advertise(uint8 enable,uint16 interval_min,uint16 interval_max,uint8 channel_map)
{
    
    gecko_cmd_msg->data.cmd_homekit_advertise.enable=enable;
    gecko_cmd_msg->data.cmd_homekit_advertise.interval_min=interval_min;
    gecko_cmd_msg->data.cmd_homekit_advertise.interval_max=interval_max;
    gecko_cmd_msg->data.cmd_homekit_advertise.channel_map=channel_map;
    gecko_cmd_msg->header=((gecko_cmd_homekit_advertise_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_advertise;
}

/** 
*
* gecko_cmd_homekit_delete_pairings
*
* This command can be used to delete all Apple HomeKit pairing data. Additionally it resets all required HomeKit settings to factory state, e.g. it resets GSN value, generates new Device ID. 
*    
*
**/

static inline struct gecko_msg_homekit_delete_pairings_rsp_t* gecko_cmd_homekit_delete_pairings()
{
    
    gecko_cmd_msg->header=((gecko_cmd_homekit_delete_pairings_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_delete_pairings;
}

/** 
*
* gecko_cmd_homekit_check_authcp
*
* This command can be used to make an I2C test connection with Apple authentication co-processor and return error if communication failed. 
*    
*
**/

static inline struct gecko_msg_homekit_check_authcp_rsp_t* gecko_cmd_homekit_check_authcp()
{
    
    gecko_cmd_msg->header=((gecko_cmd_homekit_check_authcp_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_check_authcp;
}

/** 
*
* gecko_cmd_homekit_get_pairing_id
*
* This command can be used to get pairing ID of the connected iOS device. 
*
* @param connection       
*
**/

static inline struct gecko_msg_homekit_get_pairing_id_rsp_t* gecko_cmd_homekit_get_pairing_id(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_homekit_get_pairing_id.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_homekit_get_pairing_id_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_get_pairing_id;
}

/** 
*
* gecko_cmd_homekit_send_write_response
*
* This command can be used to send a response to a "homekit_write_request" event. The response needs to be sent within 30 seconds, otherwise no more GATT              transactions are allowed by the remote side.              {br}{br}If the status_code is set to 0 the HAP will send a response informing that the write operation was processed successfully and other values will cause the HAP to send a HAP error status response. 
*
* @param connection   
* @param characteristic   
* @param status_code   HomeKit status code.    
*
**/

static inline struct gecko_msg_homekit_send_write_response_rsp_t* gecko_cmd_homekit_send_write_response(uint8 connection,uint16 characteristic,uint8 status_code)
{
    
    gecko_cmd_msg->data.cmd_homekit_send_write_response.connection=connection;
    gecko_cmd_msg->data.cmd_homekit_send_write_response.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_homekit_send_write_response.status_code=status_code;
    gecko_cmd_msg->header=((gecko_cmd_homekit_send_write_response_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_send_write_response;
}

/** 
*
* gecko_cmd_homekit_send_read_response
*
* This command can be used to send a response to a "homekit_read_request" event. The response needs to be sent within 30 seconds, or otherwise no more GATT transactions are allowed by the remote side.             {br}{br}If status_code is set to 0, the characteristic value is sent to the remote GATT client through HomeKit library in a normal way. Other status_code values will cause a HAP error status response instead of the actual data being sent.             {br}{br}If the value data size is less than attribute_size then the Apple HomeKit library will send new "homekit_read_request" event with suitable offset. The Apple HomeKit library provides automatic formatting for the frame as well encryption. 
*
* @param connection   
* @param characteristic   
* @param status_code   HomeKit Status Code.
* @param attribute_size   Size of attribute value
* @param value   Characteristic value to send to the GATT client through HomeKit library. This is ignored if status_code is not set to 0.    
*
**/

static inline struct gecko_msg_homekit_send_read_response_rsp_t* gecko_cmd_homekit_send_read_response(uint8 connection,uint16 characteristic,uint8 status_code,uint16 attribute_size,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 7)
    {
        gecko_rsp_msg->data.rsp_homekit_send_read_response.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_homekit_send_read_response;
    }

    
    gecko_cmd_msg->data.cmd_homekit_send_read_response.connection=connection;
    gecko_cmd_msg->data.cmd_homekit_send_read_response.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_homekit_send_read_response.status_code=status_code;
    gecko_cmd_msg->data.cmd_homekit_send_read_response.attribute_size=attribute_size;
    gecko_cmd_msg->data.cmd_homekit_send_read_response.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_homekit_send_read_response.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_homekit_send_read_response_id+((7+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_send_read_response;
}

/** 
*
* gecko_cmd_homekit_gsn_action
*
* This command can be used to make suitable action by HomeKit library for GSN (Global State Number) value. 
*
* @param action   Actions:                  
*  - 0: Reset GSN value to default state
*  - 1: Store GSN value to a PS-key (flash)
*     
*
**/

static inline struct gecko_msg_homekit_gsn_action_rsp_t* gecko_cmd_homekit_gsn_action(uint8 action)
{
    
    gecko_cmd_msg->data.cmd_homekit_gsn_action.action=action;
    gecko_cmd_msg->header=((gecko_cmd_homekit_gsn_action_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_gsn_action;
}

/** 
*
* gecko_cmd_homekit_event_notification
*
* This command can be used to perform suitable actions for HomeKit notifications according to a connection state and the originator of the change. When device is in connected state and it is local change it sends empty indication to controller. When device is disconnected it starts broadcast events advertising, next after timeout it starts fast advertising. Broadcast and fast advertising parameters are set in "homekit_configure". After fast advertising timeout it reverts previous advertising settings. For both states it sets suitable Global State Number value accoring to HomeKit specification rules.              
*
* @param connection   Connection handle. Ignored for disconnected state.
* @param characteristic   
* @param change_originator   Characteristic value place where change come from:                                 
*  - 0: Remote change (from controller)
*  - 1: Local change (from accessory)
* 
* @param value   Broadcast notify value.    
*
**/

static inline struct gecko_msg_homekit_event_notification_rsp_t* gecko_cmd_homekit_event_notification(uint8 connection,uint16 characteristic,uint8 change_originator,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 5)
    {
        gecko_rsp_msg->data.rsp_homekit_event_notification.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_homekit_event_notification;
    }

    
    gecko_cmd_msg->data.cmd_homekit_event_notification.connection=connection;
    gecko_cmd_msg->data.cmd_homekit_event_notification.characteristic=characteristic;
    gecko_cmd_msg->data.cmd_homekit_event_notification.change_originator=change_originator;
    gecko_cmd_msg->data.cmd_homekit_event_notification.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_homekit_event_notification.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_homekit_event_notification_id+((5+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_event_notification;
}

/** 
*
* gecko_cmd_homekit_broadcast_action
*
* This command can be used to make suitable action by HomeKit library for broadcast advertisisng. 
*
* @param action   Actions:                                  
*  - 0x00: Delete broadcast advertising data. No additional parameters are required
*  - 0x01: Store broadcast advertising data (key, charactersitics configuration) to non volatile memory. No additional parameters are required
*  - other: Reserved
* 
* @param params   Additional parameters for action.    
*
**/

static inline struct gecko_msg_homekit_broadcast_action_rsp_t* gecko_cmd_homekit_broadcast_action(uint8 action,uint8 params_len, const uint8* params_data)
{
    if ((uint16_t)params_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_homekit_broadcast_action.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_homekit_broadcast_action;
    }

    
    gecko_cmd_msg->data.cmd_homekit_broadcast_action.action=action;
    gecko_cmd_msg->data.cmd_homekit_broadcast_action.params.len=params_len;
    memcpy(gecko_cmd_msg->data.cmd_homekit_broadcast_action.params.data,params_data,params_len);
    gecko_cmd_msg->header=((gecko_cmd_homekit_broadcast_action_id+((2+params_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_homekit_broadcast_action;
}

/** 
*
* gecko_cmd_mesh_node_init
*
* Initializes the Mesh stack in Node role. When
* initialization is complete a "node initialized"             event will
* be generated.
* This command must be issued before any other Bluetooth
* Mesh commands, except for "             set node UUID" command.
* Note that you may initialize a             device either in the
* Provisioner or the Node role, but not             both.
*  
*
*
* Events generated
*
* gecko_evt_mesh_node_initialized -     
*
**/

static inline struct gecko_msg_mesh_node_init_rsp_t* gecko_cmd_mesh_node_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_init;
}

/** 
*
* gecko_cmd_mesh_node_start_unprov_beaconing
*
* Start sending Unprovisioned Device Beacons.
* This command makes an unprovisioned device available for
* provisioning. The device will start to send periodic
* unprovisioned device beacons containing device UUID. It
* will also start listening for incoming Provisioner
* connection attempts on the specified bearers (PB-ADV,             PB-
* GATT, or both). In case of PB-GATT, the device will             also
* begin advertising its provisioning GATT service.
* At the beginning of a provisioning process a "provisioning
* started" event will be generated. When the device             receives
* provisioning data from the Provisioner a "node provisioned"
* event will be generates; if provisioning fails with an
* error, a "             provisioning failed" event will be generated
* instead.
* Once provisioned, the node elements have been allocated
* addresses and a network key has been deployed to the node,
* making the node ready for further configuration by the
* Provisioner. Note that the node is not yet fully ready             for
* communicating with other nodes on the network at this
* stage.
*  
*
* @param bearer   Bit mask for which bearer to use. Values are as follows: {br}                     
*  - 1 (bit 0): PB-ADV
*  - 2 (bit 1): PB-GATT
* Other bits are reserved and must not be used.                   
*
* Events generated
*
* gecko_evt_mesh_node_provisioning_started - 
* gecko_evt_mesh_node_provisioned - 
* gecko_evt_mesh_node_provisioning_failed -     
*
**/

static inline struct gecko_msg_mesh_node_start_unprov_beaconing_rsp_t* gecko_cmd_mesh_node_start_unprov_beaconing(uint8 bearer)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_start_unprov_beaconing.bearer=bearer;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_start_unprov_beaconing_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_start_unprov_beaconing;
}

/** 
*
* gecko_cmd_mesh_node_input_oob_request_rsp
*
* This command is used to provide the stack with the Input out-of-band authentication data which the Provisioner is displaying. 
*
* @param data   Raw 16-byte array containing the authentication data.    
*
**/

static inline struct gecko_msg_mesh_node_input_oob_request_rsp_rsp_t* gecko_cmd_mesh_node_input_oob_request_rsp(uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_mesh_node_input_oob_request_rsp.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_node_input_oob_request_rsp;
    }

    
    gecko_cmd_msg->data.cmd_mesh_node_input_oob_request_rsp.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_node_input_oob_request_rsp.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_input_oob_request_rsp_id+((1+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_input_oob_request_rsp;
}

/** 
*
* gecko_cmd_mesh_node_get_uuid
*
* Get the device UUID.
* Every Mesh device has a 128-bit UUID identifying the device.
* It is used primarily during provisioning, as it is broadcast
* in Unprovisioned Device Beacons to indicate that the device
* is ready to be provisioned.
* This command can be used for debugging purposes. During
* provisioning the stack automatically uses the UUID of the
* device and it does not need to be explicitly specified when
* "unprovisioned             device beaconing" is started.
*  
*    
*
**/

static inline struct gecko_msg_mesh_node_get_uuid_rsp_t* gecko_cmd_mesh_node_get_uuid()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_uuid_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_uuid;
}

/** 
*
* gecko_cmd_mesh_node_set_provisioning_data
*
* Used to provision devices completely             out-of-band.
* Provisioner's device database needs             to be populated with
* the corresponding values to             make the device reachable and
* configurable in the             Provisioner's network.
* See also the Provisioner command for             "adding a device"
* to Provisioner's device database.
* NOTE: the device must be             reset after this command has been issued.
*  
*
* @param device_key   Device Key for this Device, shared by the Provisioner
* @param network_key   Network Key the Provisioner has selected for this Device
* @param netkey_index   Index of the Network Key the Provisioner has selected for this Device
* @param iv_index   Current IV Index used in the network
* @param address   Address the Provisioner has allocated for this Device's Primary Element
* @param kr_in_progress   Set to 1 if Key Refresh is currently in progress, otherwise 0    
*
**/

static inline struct gecko_msg_mesh_node_set_provisioning_data_rsp_t* gecko_cmd_mesh_node_set_provisioning_data(aes_key_128 device_key,aes_key_128 network_key,uint16 netkey_index,uint32 iv_index,uint16 address,uint8 kr_in_progress)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_set_provisioning_data.device_key=device_key;
    gecko_cmd_msg->data.cmd_mesh_node_set_provisioning_data.network_key=network_key;
    gecko_cmd_msg->data.cmd_mesh_node_set_provisioning_data.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_node_set_provisioning_data.iv_index=iv_index;
    gecko_cmd_msg->data.cmd_mesh_node_set_provisioning_data.address=address;
    gecko_cmd_msg->data.cmd_mesh_node_set_provisioning_data.kr_in_progress=kr_in_progress;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_set_provisioning_data_id+((41)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_set_provisioning_data;
}

/** 
*
* gecko_cmd_mesh_node_init_oob
*
* Initializes the Mesh stack in Node role. When
* initialization is complete a "node initialized"             event will
* be generated.
* This command is the same as the "node initialization"
* command except for parameters defining whether OOB
* authentication data stored on the device can be used
* during provisioning.
* This command must be issued before any other Bluetooth
* Mesh commands, except for "             set node UUID" command.
* Note that you may initialize a device either in the
* Provisioner or the Node role, but not both.
*  
*
* @param public_key   If nonzero, use the ECC key stored                 in persistent storage during provisioning instead                 of an ephemeral key. 
* @param auth_methods   Allowed OOB authentication methods. The value                 is a bitmap so that multiple methods can be allowed.
* @param output_actions   Allowed OOB Output Action types
* @param output_size   Maximum Output OOB size Valid values range from 0 (feature not supported) to 8.
* @param input_actions   Allowed OOB Input Action types
* @param input_size   Maximum Input OOB size. Valid values range from 0 (feature not supported) to 8.
* @param oob_location   Defines the OOB data location bitmask                 
*
* Events generated
*
* gecko_evt_mesh_node_initialized -     
*
**/

static inline struct gecko_msg_mesh_node_init_oob_rsp_t* gecko_cmd_mesh_node_init_oob(uint8 public_key,uint8 auth_methods,uint16 output_actions,uint8 output_size,uint16 input_actions,uint8 input_size,uint16 oob_location)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.public_key=public_key;
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.auth_methods=auth_methods;
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.output_actions=output_actions;
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.output_size=output_size;
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.input_actions=input_actions;
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.input_size=input_size;
    gecko_cmd_msg->data.cmd_mesh_node_init_oob.oob_location=oob_location;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_init_oob_id+((10)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_init_oob;
}

/** 
*
* gecko_cmd_mesh_node_set_ivrecovery_mode
*
* Enable/disable IV index recovery mode
* If the node has not been in communication with the
* network for             a long time (e.g., due to having been turned
* off) it may have             missed IV index updates and isn't any
* more able to communicate             with other nodes. In this case,
* IV index recovery mode should             be enabled.
*  
*
* @param mode   Zero to disable; nonzero to enable    
*
**/

static inline struct gecko_msg_mesh_node_set_ivrecovery_mode_rsp_t* gecko_cmd_mesh_node_set_ivrecovery_mode(uint8 mode)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_set_ivrecovery_mode.mode=mode;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_set_ivrecovery_mode_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_set_ivrecovery_mode;
}

/** 
*
* gecko_cmd_mesh_node_get_ivrecovery_mode
*
* Get current IV index recovery mode state. See "set IV index recovery mode" for details. 
*    
*
**/

static inline struct gecko_msg_mesh_node_get_ivrecovery_mode_rsp_t* gecko_cmd_mesh_node_get_ivrecovery_mode()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_ivrecovery_mode_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_ivrecovery_mode;
}

/** 
*
* gecko_cmd_mesh_node_set_adv_event_filter
*
* Set filter for received advertising packet events.            
* As Mesh data traffic is carried over advertising events and
* the BLE stack is scanning continuously when Mesh stack is
* active, by default the Mesh stack filters out advertising
* events so that the application is not burdened by them.
* If the application needs to process some advertising           events
* it can use this command to unblock particular types           of
* events.
*  
*
* @param mask   Enabled advertising packet type                   
*  - 0x01: Connectable undirected advertising
*  - 0x02: Scannable undirected advertising
*  - 0x04: Non connectable undirected advertising
*  - 0x08: Scan Response
*  - 0x8000: Use gap data type. Don't use with other values
* 
* @param gap_data_type   Used when the type is set to 0x8000.  Events are                   generated when advertising packets contain any of                   the AD data types specified by this parameter. Type                   values are defined in the Bluetooth SIG Data Types                   Specification.{br} Values must be set as two digit                   hex number, maximum 8 items.                     
*
**/

static inline struct gecko_msg_mesh_node_set_adv_event_filter_rsp_t* gecko_cmd_mesh_node_set_adv_event_filter(uint16 mask,uint8 gap_data_type_len, const uint8* gap_data_type_data)
{
    if ((uint16_t)gap_data_type_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_mesh_node_set_adv_event_filter.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_node_set_adv_event_filter;
    }

    
    gecko_cmd_msg->data.cmd_mesh_node_set_adv_event_filter.mask=mask;
    gecko_cmd_msg->data.cmd_mesh_node_set_adv_event_filter.gap_data_type.len=gap_data_type_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_node_set_adv_event_filter.gap_data_type.data,gap_data_type_data,gap_data_type_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_set_adv_event_filter_id+((3+gap_data_type_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_set_adv_event_filter;
}

/** 
*
* gecko_cmd_mesh_node_get_statistics
*
*  
*    
*
**/

static inline struct gecko_msg_mesh_node_get_statistics_rsp_t* gecko_cmd_mesh_node_get_statistics()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_statistics_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_statistics;
}

/** 
*
* gecko_cmd_mesh_node_clear_statistics
*
*  
*    
*
**/

static inline struct gecko_msg_mesh_node_clear_statistics_rsp_t* gecko_cmd_mesh_node_clear_statistics()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_clear_statistics_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_clear_statistics;
}

/** 
*
* gecko_cmd_mesh_node_set_net_relay_delay
*
* Set network relay delay interval.             
* This             parameter determines how long a relay waits until it
* relays a network PDU; the value used is a random number
* within the specified interval.
* Note that this value affects the first instance of             the
* relayed network PDU; if relay retransmissions are             enabled
* the interval between retransmissions is defined             by the
* relay state, set by the Provisioner of the network             or by
* "set local relay             state" test command.
*  
*
* @param min   Minimum interval, in milliseconds.
* @param max   Maximum interval, in milliseconds. Must be equal to or greather than the minimum.     
*
**/

static inline struct gecko_msg_mesh_node_set_net_relay_delay_rsp_t* gecko_cmd_mesh_node_set_net_relay_delay(uint8 min,uint8 max)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_set_net_relay_delay.min=min;
    gecko_cmd_msg->data.cmd_mesh_node_set_net_relay_delay.max=max;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_set_net_relay_delay_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_set_net_relay_delay;
}

/** 
*
* gecko_cmd_mesh_node_get_net_relay_delay
*
* Get network relay delay interval.           See "set           network relay delay" command for details.              
*    
*
**/

static inline struct gecko_msg_mesh_node_get_net_relay_delay_rsp_t* gecko_cmd_mesh_node_get_net_relay_delay()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_net_relay_delay_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_net_relay_delay;
}

/** 
*
* gecko_cmd_mesh_node_get_ivupdate_state
*
* Get the current IV index update state in the network.            
*    
*
**/

static inline struct gecko_msg_mesh_node_get_ivupdate_state_rsp_t* gecko_cmd_mesh_node_get_ivupdate_state()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_ivupdate_state_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_ivupdate_state;
}

/** 
*
* gecko_cmd_mesh_node_request_ivupdate
*
* Attempt to request an IV index update in the           network.
* Each network layer PDU a node sends has a 24-bit sequence
* number attached to it; each element of a node keeps a
* sequence number counter which is incremented for every PDU
* sent out to the network. It is forbidden to repeat sequence
* numbers for a given IV index value, so if a node determines
* it is about to exhaust the available sequence numbers in one
* of its elements it needs to request an IV index update by
* issuing this command.
* Determining when a node may run out of sequence numbers           has
* to be done at the application as the stack cannot have
* knowledge of how often the application plans to transmit           to
* the network, i.e., how long the remaining sequence           numbers
* might last.
* See also the "get           remaining sequence numbers" command.
* Note that the call may fail for various reasons, for instance
* if an IV index update is already ongoing, or if not enough
* time has passed since the previous IV index update.
*  
*    
*
**/

static inline struct gecko_msg_mesh_node_request_ivupdate_rsp_t* gecko_cmd_mesh_node_request_ivupdate()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_request_ivupdate_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_request_ivupdate;
}

/** 
*
* gecko_cmd_mesh_node_get_seq_remaining
*
* Get the number of sequence numbers remaining on an element (before sequence numbers are exhausted). Note that every element of a node keeps a separate sequence number counter.            
*
* @param elem_index   The index of queried element    
*
**/

static inline struct gecko_msg_mesh_node_get_seq_remaining_rsp_t* gecko_cmd_mesh_node_get_seq_remaining(uint16 elem_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_get_seq_remaining.elem_index=elem_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_seq_remaining_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_seq_remaining;
}

/** 
*
* gecko_cmd_mesh_node_save_replay_protection_list
*
* Save the current replay protection list to            persistent
* storage.
* The replay protection list keeps track of the sequence
* numbers of packets from different sources received by the
* node. The node will not process messages associated with
* already used sequence numbers, thus being protected from
* replay attacks using previously recorded messages.
* The replay protection list is kept in RAM during            runtime.
* It needs to be saved to persistent storage            periodically and
* always before the device powers off.  As            the stack is not
* aware when this will happen the            application has the
* responsibility of calling this method            while the node is
* getting ready to power down but still            running.
*  
*    
*
**/

static inline struct gecko_msg_mesh_node_save_replay_protection_list_rsp_t* gecko_cmd_mesh_node_save_replay_protection_list()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_save_replay_protection_list_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_save_replay_protection_list;
}

/** 
*
* gecko_cmd_mesh_node_set_uuid
*
* Write device UUID into persistent storage. This           command must
* be called before initializing the mesh stack (before
* "mesh_node_init" or           "mesh_node_init_oob";
* otherwise the change will not take effect before a reboot.
* UUID should conform to the format defined in "RFC 4122"
* Note that UUID must not be changed when the device is provisioned
* to a network.
* Furthermore, UUID should remain constant if a device           has
* received a firmware update which requires reprovisioning of the
* device once the update has been applied (e.g., new elements are
* added by the update).
*  
*
* @param uuid   UUID to set    
*
**/

static inline struct gecko_msg_mesh_node_set_uuid_rsp_t* gecko_cmd_mesh_node_set_uuid(uuid_128 uuid)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_set_uuid.uuid=uuid;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_set_uuid_id+((16)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_set_uuid;
}

/** 
*
* gecko_cmd_mesh_node_get_element_address
*
* Get the unicast address configured to an element. 
*
* @param elem_index   The index of the target element, 0 is the primary element    
*
**/

static inline struct gecko_msg_mesh_node_get_element_address_rsp_t* gecko_cmd_mesh_node_get_element_address(uint16 elem_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_node_get_element_address.elem_index=elem_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_get_element_address_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_get_element_address;
}

/** 
*
* gecko_cmd_mesh_node_static_oob_request_rsp
*
* This command is used to provide the stack with static out-of-band authentication data which the stack requested. 
*
* @param data   Raw 16-byte array containing the authentication data.    
*
**/

static inline struct gecko_msg_mesh_node_static_oob_request_rsp_rsp_t* gecko_cmd_mesh_node_static_oob_request_rsp(uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_mesh_node_static_oob_request_rsp.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_node_static_oob_request_rsp;
    }

    
    gecko_cmd_msg->data.cmd_mesh_node_static_oob_request_rsp.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_node_static_oob_request_rsp.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_static_oob_request_rsp_id+((1+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_static_oob_request_rsp;
}

/** 
*
* gecko_cmd_mesh_node_reset
*
* Factory reset the mesh node.
* To complete procedure the application should do its             own cleanup duties and reset the hardware.              
*    
*
**/

static inline struct gecko_msg_mesh_node_reset_rsp_t* gecko_cmd_mesh_node_reset()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_node_reset_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_node_reset;
}

/** 
*
* gecko_cmd_mesh_prov_init
*
* Initializes the Mesh stack in Provisioner             role. When
* initialization is complete a "provisioner initialized
* event" will be generated.
* This command must be issued before any other Bluetooth
* Mesh commands. Note that you may initialize a device
* either in the Provisioner or the Node role, but not both.
*  
*
*
* Events generated
*
* gecko_evt_mesh_prov_initialized -     
*
**/

static inline struct gecko_msg_mesh_prov_init_rsp_t* gecko_cmd_mesh_prov_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_init;
}

/** 
*
* gecko_cmd_mesh_prov_scan_unprov_beacons
*
* Start scanning for unprovisioned device             beacons.
* Unprovisioned devices send out beacons             containing their
* UUID.  An "unprovisioned beacon             event" will be generated
* for each beacon seen. Once the             UUID of a device is known,
* the Provisioner may start             provisioning the device by
* issuing either the " provision device             over PB-ADV" or
* "provision             device over PB-GATT" command.
*  
*
*
* Events generated
*
* gecko_evt_mesh_prov_unprov_beacon -     
*
**/

static inline struct gecko_msg_mesh_prov_scan_unprov_beacons_rsp_t* gecko_cmd_mesh_prov_scan_unprov_beacons()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_scan_unprov_beacons_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_scan_unprov_beacons;
}

/** 
*
* gecko_cmd_mesh_prov_provision_device
*
* Provision a device into a network using the             advertisement
* bearer (PB-ADV)
* Issuing this command starts the provisioning process for
* the specified device. Once the process completes
* successfully, a "device             provisioned event" is generated.
* If provisioning does             not succeed, a " provisioning
* failed event" will be generated instead.
*  
*
* @param network_id   Index of the initial network key                     which is sent to the device during                     provisioning.
* @param uuid   UUID of the device to provision
*
* Events generated
*
* gecko_evt_mesh_prov_device_provisioned - 
* gecko_evt_mesh_prov_provisioning_failed -     
*
**/

static inline struct gecko_msg_mesh_prov_provision_device_rsp_t* gecko_cmd_mesh_prov_provision_device(uint8 network_id,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 2)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_provision_device.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_provision_device;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device.network_id=network_id;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_provision_device.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_provision_device_id+((2+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_provision_device;
}

/** 
*
* gecko_cmd_mesh_prov_create_network
*
* Creates a new network key on the Provisioner.
* The created key can be deployed on a Node using the "add network key"
* command.
*  
*
* @param key   Key value to use; set to zero-length array to generate random key.    
*
**/

static inline struct gecko_msg_mesh_prov_create_network_rsp_t* gecko_cmd_mesh_prov_create_network(uint8 key_len, const uint8* key_data)
{
    if ((uint16_t)key_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_create_network.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_create_network;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_create_network.key.len=key_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_create_network.key.data,key_data,key_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_create_network_id+((1+key_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_create_network;
}

/** 
*
* gecko_cmd_mesh_prov_get_dcd
*
* Deprecated. Replacement is "mesh_config_client_get_dcd" command.
* Get the DCD of the device from a remote             Configuration
* Server.  If the call succeeds, the retrieved             DCD will be
* returned in a "DCD status"             event.
*  
*
* @param address   Unicast address of the target Node's primary element
* @param page   page number for requested DCD, Use 0xff to get highest existing page    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_get_dcd_rsp_t* gecko_cmd_mesh_prov_get_dcd(uint16 address,uint8 page)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_get_dcd.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_get_dcd.page=page;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_get_dcd_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_get_dcd;
}

/** 
*
* gecko_cmd_mesh_prov_get_config
*
* Deprecated. Replacements are               "mesh_config_client_get_beacon",               "mesh_config_client_get_default_ttl",               "mesh_config_client_get_friend",               "mesh_config_client_get_gatt_proxy",               "mesh_config_client_get_identity",               "mesh_config_client_get_lpn_polltimeout", and               "mesh_config_client_get_relay"               commands.
* Get a configration state value of a Node.
* Node Configuration Server model state contains a number
* of node-wide values (for instance, Node's default TTL
* value) which are represented as single bytes; they can
* be queried with this command. See the " list of
* configuration states" for reference.
* Querying the more complex states (for instance,               model-
* application key bindings) should be done using the
* commands dedicated for the purpose; see, e.g., "get
* model application key bindings" command.
* Node response is reported with an "configuration status
* event".
*  
*
* @param address   Unicast address of the target Node's primary element
* @param id   The state to read
* @param netkey_index   Ignored for node-wide States.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_get_config_rsp_t* gecko_cmd_mesh_prov_get_config(uint16 address,uint16 id,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_get_config.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_get_config.id=id;
    gecko_cmd_msg->data.cmd_mesh_prov_get_config.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_get_config_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_get_config;
}

/** 
*
* gecko_cmd_mesh_prov_set_config
*
* Deprecated. Replacements are               "mesh_config_client_set_beacon",               "mesh_config_client_set_default_ttl",               "mesh_config_client_set_friend",               "mesh_config_client_set_gatt_proxy",               "mesh_config_client_set_identity", and               "mesh_config_client_set_relay"               commands.
* Set a configration state value of a Node.
* Node Configuration Server model state contains a number
* of node-wide values (for instance, Node's default TTL
* value) which are represented as single bytes; they can
* be modified with this command. See the " list of
* configuration states" for reference.
* Setting the more complex states should be done using the
* commands dedicated for the purpose as this command accepts
* only raw binary data as the value to set.
* Node response is reported with an "configuration status
* event".
*  
*
* @param address   Unicast address of the target Node's primary element
* @param id   The State to manipulate
* @param netkey_index   Ignored for node-wide States.
* @param value   Raw binary value
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_set_config_rsp_t* gecko_cmd_mesh_prov_set_config(uint16 address,uint16 id,uint16 netkey_index,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 7)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_set_config.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_set_config;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_set_config.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_set_config.id=id;
    gecko_cmd_msg->data.cmd_mesh_prov_set_config.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_set_config.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_set_config.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_set_config_id+((7+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_set_config;
}

/** 
*
* gecko_cmd_mesh_prov_create_appkey
*
* Creates a new application key on the Provisioner.
* An application key is always bound to a network key; that
* is, the application key is only valid in the context of a
* particular network key. The selected network key must
* exist on the Provisioner (see "create network             key"
* command).
* The created application key can be deployed on a Node
* using the "add             application key" command.
*  
*
* @param netkey_index   Index of the network key to which the application key will be bound
* @param key   Key value to use; set to zero-length array to generate random key.    
*
**/

static inline struct gecko_msg_mesh_prov_create_appkey_rsp_t* gecko_cmd_mesh_prov_create_appkey(uint16 netkey_index,uint8 key_len, const uint8* key_data)
{
    if ((uint16_t)key_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_create_appkey.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_create_appkey;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_create_appkey.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_create_appkey.key.len=key_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_create_appkey.key.data,key_data,key_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_create_appkey_id+((3+key_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_create_appkey;
}

/** 
*
* gecko_cmd_mesh_prov_oob_pkey_rsp
*
* This command is used to respond to prov_oob_pkey_request 
*
* @param pkey   Public Key read out-of-band    
*
**/

static inline struct gecko_msg_mesh_prov_oob_pkey_rsp_rsp_t* gecko_cmd_mesh_prov_oob_pkey_rsp(uint8 pkey_len, const uint8* pkey_data)
{
    if ((uint16_t)pkey_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_oob_pkey_rsp.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_oob_pkey_rsp;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_oob_pkey_rsp.pkey.len=pkey_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_oob_pkey_rsp.pkey.data,pkey_data,pkey_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_oob_pkey_rsp_id+((1+pkey_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_oob_pkey_rsp;
}

/** 
*
* gecko_cmd_mesh_prov_oob_auth_rsp
*
* This command is used to respond to prov_oob_auth_request 
*
* @param data   Output or Static OOB data    
*
**/

static inline struct gecko_msg_mesh_prov_oob_auth_rsp_rsp_t* gecko_cmd_mesh_prov_oob_auth_rsp(uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_oob_auth_rsp.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_oob_auth_rsp;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_oob_auth_rsp.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_oob_auth_rsp.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_oob_auth_rsp_id+((1+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_oob_auth_rsp;
}

/** 
*
* gecko_cmd_mesh_prov_set_oob_requirements
*
* Set the OOB requirements for devices to be Provisioned 
*
* @param public_key   Zero to not use OOB Public Key
* @param auth_methods   Allowed OOB authentication methods                     The value is a bitmap so that multiple methods can                     be allowed. 
* @param output_actions   Allowed OOB Output Action types
* @param input_actions   Allowed OOB Input Action types
* @param min_size   Minimum Input/Output OOB size. Values range from 0 (input/output OOB not used) to 8.
* @param max_size   Maximum Input/Output OOB size. Must be smaller than or equal to the minimum size. Values range from 0 (input/output OOB not used) to 8.    
*
**/

static inline struct gecko_msg_mesh_prov_set_oob_requirements_rsp_t* gecko_cmd_mesh_prov_set_oob_requirements(uint8 public_key,uint8 auth_methods,uint16 output_actions,uint16 input_actions,uint8 min_size,uint8 max_size)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_set_oob_requirements.public_key=public_key;
    gecko_cmd_msg->data.cmd_mesh_prov_set_oob_requirements.auth_methods=auth_methods;
    gecko_cmd_msg->data.cmd_mesh_prov_set_oob_requirements.output_actions=output_actions;
    gecko_cmd_msg->data.cmd_mesh_prov_set_oob_requirements.input_actions=input_actions;
    gecko_cmd_msg->data.cmd_mesh_prov_set_oob_requirements.min_size=min_size;
    gecko_cmd_msg->data.cmd_mesh_prov_set_oob_requirements.max_size=max_size;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_set_oob_requirements_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_set_oob_requirements;
}

/** 
*
* gecko_cmd_mesh_prov_key_refresh_start
*
* Start a key refresh procedure in the network.
* A key refresh procedure updates a network key, and
* optionally application keys associated with it, in all
* nodes of the network except for blacklisted nodes. After
* the refresh procedure is complete the old keys will be
* discarded. Thus the blacklisted nodes which did not
* receive new keys will be shut out of the network at the
* completion of the procedure.
*  
*
* @param netkey_index   Index of the network key to update
* @param num_appkeys   Number of application keys to update;                     may be zero.
* @param appkey_indices   Indices of the application keys to                     update, represented as little-endian two byte                     sequences; the array must contain num_appkeys                     indices and thus 2*num_appkeys bytes in                     total.
*
* Events generated
*
* gecko_evt_mesh_prov_key_refresh_node_update - 
* gecko_evt_mesh_prov_key_refresh_phase_update - 
* gecko_evt_mesh_prov_key_refresh_complete -     
*
**/

static inline struct gecko_msg_mesh_prov_key_refresh_start_rsp_t* gecko_cmd_mesh_prov_key_refresh_start(uint16 netkey_index,uint8 num_appkeys,uint8 appkey_indices_len, const uint8* appkey_indices_data)
{
    if ((uint16_t)appkey_indices_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_key_refresh_start.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_key_refresh_start;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_key_refresh_start.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_key_refresh_start.num_appkeys=num_appkeys;
    gecko_cmd_msg->data.cmd_mesh_prov_key_refresh_start.appkey_indices.len=appkey_indices_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_key_refresh_start.appkey_indices.data,appkey_indices_data,appkey_indices_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_key_refresh_start_id+((4+appkey_indices_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_key_refresh_start;
}

/** 
*
* gecko_cmd_mesh_prov_get_key_refresh_blacklist
*
* Check the key refresh blacklist status of a           node. Blacklisted nodes do not participate in the key           refresh procedure, and can thus be shut out of the           network.  
*
* @param key   Network key index
* @param uuid   UUID of the Device    
*
**/

static inline struct gecko_msg_mesh_prov_get_key_refresh_blacklist_rsp_t* gecko_cmd_mesh_prov_get_key_refresh_blacklist(uint16 key,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_get_key_refresh_blacklist.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_get_key_refresh_blacklist;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_blacklist.key=key;
    gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_blacklist.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_blacklist.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_get_key_refresh_blacklist_id+((3+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_get_key_refresh_blacklist;
}

/** 
*
* gecko_cmd_mesh_prov_set_key_refresh_blacklist
*
* Set the key refresh blacklist status of a           node. Blacklisted nodes do not participate in the key           refresh procedure, and can thus be shut out of the           network.  
*
* @param key   Network key index
* @param status   Nonzero for blacklisted node
* @param uuid   UUID of the Device    
*
**/

static inline struct gecko_msg_mesh_prov_set_key_refresh_blacklist_rsp_t* gecko_cmd_mesh_prov_set_key_refresh_blacklist(uint16 key,uint8 status,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_set_key_refresh_blacklist.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_set_key_refresh_blacklist;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_blacklist.key=key;
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_blacklist.status=status;
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_blacklist.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_blacklist.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_set_key_refresh_blacklist_id+((4+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_set_key_refresh_blacklist;
}

/** 
*
* gecko_cmd_mesh_prov_appkey_add
*
* Deprecated. Replacement is "mesh_config_client_add_appkey" command.
* Push an application key to a node. The key
* must exist on the Provisioner (see "create application
* key" command).
* An application key is always bound to a network key; that
* is, the application key is only valid in the context of a
* particular network key. The selected network key must
* exist on the Provisioner (see "create network             key"
* command) and must have been deployed on the node             prior to
* this command (either during provisioning or with             an "add
* network             key" command).
* Node response is reported with an "configuration status
* event".
*  
*
* @param address   Unicast address of the target node's primary element
* @param netkey_index   The network key index to which the application key is bound
* @param appkey_index   The index of the application key to push to the node.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_appkey_add_rsp_t* gecko_cmd_mesh_prov_appkey_add(uint16 address,uint16 netkey_index,uint16 appkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_add.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_add.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_add.appkey_index=appkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_appkey_add_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_appkey_add;
}

/** 
*
* gecko_cmd_mesh_prov_appkey_delete
*
* Deprecated. Replacement is "mesh_config_client_remove_appkey" command.
* Delete an application key on a node.
* Note that the deleted key will be removed from any model
* bindings on the node at the same time automatically; there
* is no need to explicitly delete them using "model-application
* key unbind command".
* Node response is reported with an "configuration status
* event".
*  
*
* @param address   Unicast address of the target Node's primary element
* @param netkey_index   Index of the network key to which the application kkey is bound on the node
* @param appkey_index   Index of the application key to delete
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_appkey_delete_rsp_t* gecko_cmd_mesh_prov_appkey_delete(uint16 address,uint16 netkey_index,uint16 appkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_delete.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_delete.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_delete.appkey_index=appkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_appkey_delete_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_appkey_delete;
}

/** 
*
* gecko_cmd_mesh_prov_model_app_bind
*
* Deprecated. Replacement is "mesh_config_client_bind_model" command.
* Bind a model to an application key. Node response is reported with a
* "configuration status" event.
*  
*
* @param address   Unicast address of the target node's primary element
* @param elem_address   Unicast address of the element containing the model being configured
* @param netkey_index   The network key index used for encrypting the request. 
* @param appkey_index   The application key to use for binding
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_app_bind_rsp_t* gecko_cmd_mesh_prov_model_app_bind(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_bind.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_bind.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_bind.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_bind.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_bind.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_bind.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_app_bind_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_app_bind;
}

/** 
*
* gecko_cmd_mesh_prov_model_app_unbind
*
* Deprecated. Replacement is "mesh_config_client_unbind_model" command.
* Remove application key binding from a model. Node response is reported
* with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node's primary element
* @param elem_address   Unicast address of the element containing the model being configured
* @param netkey_index   The network key index used for encrypting the request. 
* @param appkey_index   The index of the application key used in the binding to be removed
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_app_unbind_rsp_t* gecko_cmd_mesh_prov_model_app_unbind(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_unbind.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_unbind.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_unbind.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_unbind.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_unbind.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_unbind.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_app_unbind_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_app_unbind;
}

/** 
*
* gecko_cmd_mesh_prov_model_app_get
*
* Deprecated. Replacement is "mesh_config_client_list_bindings" command.
* Get application keys to which the model is bound. Node response is
* reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node's primary element
* @param elem_address   Unicast address of the element containing the model being configured
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_app_get_rsp_t* gecko_cmd_mesh_prov_model_app_get(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_get.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_get.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_get.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_app_get.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_app_get_id+((10)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_app_get;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_add
*
* Deprecated. Replacement is "mesh_config_client_add_model_sub" command.
* Add an address to a model's subscription list. Node response is
* reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param sub_address   The address to add to the subscription list. Note that the address has to be a group address. 
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_add_rsp_t* gecko_cmd_mesh_prov_model_sub_add(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_add_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_add;
}

/** 
*
* gecko_cmd_mesh_prov_model_pub_set
*
* Deprecated. Replacement is "mesh_config_client_set_model_pub" command.
* Set a model's publication address, key, and parameters.  Node response
* is reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param appkey_index   The application key index to use for the published messages. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param pub_address   The address to publish to. Can be a unicast address, a virtual address, or a group address; can also be the unassigned address to stop the model from publishing. 
* @param ttl   Publication time-to-live value
* @param period   Publication period encoded as step count and step resolution. The encoding is as follows:                   
*  - Bits 0..5: Step count
*  - Bits 6..7: Step resolution:                     
*  - 00: 100 milliseconds
*  - 01: 1 second
*  - 10: 10 seconds
*  - 11: 10 minutes
* 
* @param retrans   Retransmission count and interval;                   controls how many
* times the model re-publishes the                   same message after
* the initial publish transmission,                   and the cadence of
* retransmissions.
* Retransmission count is encoded in the three                   low
* bits of the value, ranging from 0 to 7. Default
* value is 0 (no retransmissions).
* Retransmission interval is encoded in the                   five high
* bits of the value, ranging from 0 to 31,                   in
* 50-millisecond units. Value of 0 corresponds to                   50
* ms, while value of 31 corresponds to 1600 ms.
* 
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_pub_set_rsp_t* gecko_cmd_mesh_prov_model_pub_set(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id,uint16 pub_address,uint8 ttl,uint8 period,uint8 retrans)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.pub_address=pub_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.period=period;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set.retrans=retrans;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_pub_set_id+((17)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set;
}

/** 
*
* gecko_cmd_mesh_prov_provision_gatt_device
*
* Provision a device into a network using the             GATT bearer
* (PB-GATT)
* Issuing this command starts the provisioning process for
* the specified device. Once the process completes
* successfully, a "device             provisioned event" is generated.
* If provisioning does             not succeed, a " provisioning
* failed event" will be generated instead.
* Note that this command is available only if GATT
* functionality is compiled in to the firmware. If that is
* not the case, the command will return with a "not
* implemented" return code.
*  
*
* @param network_id   Index of the initial network key                     which is sent to the device during                     provisioning.
* @param connection   Connection handle for the device to be provisioned                    
* @param uuid   UUID of the Device to provision.
*
* Events generated
*
* gecko_evt_mesh_prov_device_provisioned - 
* gecko_evt_mesh_prov_provisioning_failed -     
*
**/

static inline struct gecko_msg_mesh_prov_provision_gatt_device_rsp_t* gecko_cmd_mesh_prov_provision_gatt_device(uint8 network_id,uint8 connection,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_provision_gatt_device.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_provision_gatt_device;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device.network_id=network_id;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device.connection=connection;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_provision_gatt_device_id+((3+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_provision_gatt_device;
}

/** 
*
* gecko_cmd_mesh_prov_ddb_get
*
* Get a Provisioner device database entry with matching UUID. 
*
* @param uuid   UUID of the Device to retrieve    
*
**/

static inline struct gecko_msg_mesh_prov_ddb_get_rsp_t* gecko_cmd_mesh_prov_ddb_get(uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_ddb_get.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_ddb_get;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_get.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_ddb_get.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_ddb_get_id+((1+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_ddb_get;
}

/** 
*
* gecko_cmd_mesh_prov_ddb_delete
*
* Delete node information from Provisioner             database.  This should be followed by a " key refresh             procedure" updating the keys of the remaining nodes to             make sure the deleted node is shut off from the             network. 
*
* @param uuid   UUID of the node to delete    
*
**/

static inline struct gecko_msg_mesh_prov_ddb_delete_rsp_t* gecko_cmd_mesh_prov_ddb_delete(uuid_128 uuid)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_delete.uuid=uuid;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_ddb_delete_id+((16)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_ddb_delete;
}

/** 
*
* gecko_cmd_mesh_prov_ddb_add
*
* Add a new node entry to the Provisioner's             device database.  Note that the device key, primary element             address, and network key need to be deployed to the node being             added in order for it to be configurable. See "set node             provisioning data" command.  
*
* @param uuid   UUID of the node to add
* @param device_key   Device key value for the node
* @param netkey_index   Index of the network key the node shall use for configuration.
* @param address   Unicast address to allocate for the node's primary element
* @param elements   Number of elements the Device has    
*
**/

static inline struct gecko_msg_mesh_prov_ddb_add_rsp_t* gecko_cmd_mesh_prov_ddb_add(uuid_128 uuid,aes_key_128 device_key,uint16 netkey_index,uint16 address,uint8 elements)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_add.uuid=uuid;
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_add.device_key=device_key;
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_add.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_add.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_ddb_add.elements=elements;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_ddb_add_id+((37)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_ddb_add;
}

/** 
*
* gecko_cmd_mesh_prov_ddb_list_devices
*
* Lists nodes known by this Provisioner. A             number of "database             listing" events will be generated. 
*
*
* Events generated
*
* gecko_evt_mesh_prov_ddb_list -     
*
**/

static inline struct gecko_msg_mesh_prov_ddb_list_devices_rsp_t* gecko_cmd_mesh_prov_ddb_list_devices()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_ddb_list_devices_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_ddb_list_devices;
}

/** 
*
* gecko_cmd_mesh_prov_network_add
*
* Deprecated. Replacement is "mesh_config_client_add_netkey" command.
* Push a Network Key to a Node. The key             must exist on the
* Provisioner (see "create network             key" command).
* Node response is reported with an "configuration status
* event".
*  
*
* @param address   Unicast address of the target Node's primary element
* @param netkey_index   The index of the key to push to the Node.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_network_add_rsp_t* gecko_cmd_mesh_prov_network_add(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_network_add.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_network_add.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_network_add_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_network_add;
}

/** 
*
* gecko_cmd_mesh_prov_network_delete
*
* Deprecated. Replacement is "mesh_config_client_remove_netkey" command.
* Delete a network key on a node.
* When a network key is deleted the application keys             bound
* to it are deleted automatically; there is no             need to
* explicitly use the "             delete application key" command.
* Note that it is not possible to delete the key used in
* encrypting the command itself (which is the first network
* key deployed to the node during provisioning) as otherwise
* the node would not be able to respond.
* Node response is reported with an "configuration status
* event".
*  
*
* @param address   Unicast address of the target node's primary element
* @param netkey_index   The index of the key to delete
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_network_delete_rsp_t* gecko_cmd_mesh_prov_network_delete(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_network_delete.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_network_delete.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_network_delete_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_network_delete;
}

/** 
*
* gecko_cmd_mesh_prov_nettx_get
*
* Deprecated. Replacement is "mesh_config_client_get_network_transmit" command.
* Retrieve network layer transmission parameters of a node.
*  
*
* @param address   Unicast address of the target node    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_nettx_get_rsp_t* gecko_cmd_mesh_prov_nettx_get(uint16 address)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_nettx_get.address=address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_nettx_get_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_nettx_get;
}

/** 
*
* gecko_cmd_mesh_prov_nettx_set
*
* Deprecated. Replacement is "mesh_config_client_set_network_transmit" command.
* Set network layer transmission parameters of a node.
*  
*
* @param address   Unicast address of the target node
* @param count   Retransmission count (excluding initial transmission).                   Range: 0..7; the default value is 0 (no retransmissions). 
* @param interval   Rettransmission interval in 10-millisecond steps    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_nettx_set_rsp_t* gecko_cmd_mesh_prov_nettx_set(uint16 address,uint8 count,uint8 interval)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_nettx_set.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_nettx_set.count=count;
    gecko_cmd_msg->data.cmd_mesh_prov_nettx_set.interval=interval;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_nettx_set_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_nettx_set;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_del
*
* Deprecated. Replacement is "mesh_config_client_remove_model_sub" command.
* Remove an address from a model's subscription             list. Node
* response is reported with a "configuration             status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param sub_address   The address to remove from the subscription list
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_del_rsp_t* gecko_cmd_mesh_prov_model_sub_del(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_del_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_del;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_add_va
*
* Deprecated. Replacement is "mesh_config_client_add_model_sub_va" command.
* Add an virtual address to a model's subscription list. Node response
* is reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param sub_address   The Label UUID to add to the subscription list. The array must be exactly 16 bytes long.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_add_va_rsp_t* gecko_cmd_mesh_prov_model_sub_add_va(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id,uint8 sub_address_len, const uint8* sub_address_data)
{
    if ((uint16_t)sub_address_len > BGLIB_MSG_MAX_PAYLOAD - 11)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_model_sub_add_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_add_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.sub_address.len=sub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_model_sub_add_va.sub_address.data,sub_address_data,sub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_add_va_id+((11+sub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_add_va;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_del_va
*
* Deprecated. Replacement is "mesh_config_client_remove_model_sub_va" command.
* Remove a virtual address from a Model's subscription list. Node
* response is reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param sub_address   The Label UUID to remove from the subscription list. The array must be exactly 16 bytes long.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_del_va_rsp_t* gecko_cmd_mesh_prov_model_sub_del_va(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id,uint8 sub_address_len, const uint8* sub_address_data)
{
    if ((uint16_t)sub_address_len > BGLIB_MSG_MAX_PAYLOAD - 11)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_model_sub_del_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_del_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.sub_address.len=sub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_model_sub_del_va.sub_address.data,sub_address_data,sub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_del_va_id+((11+sub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_del_va;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_set
*
* Deprecated. Replacement is "mesh_config_client_set_model_sub" command.
* Set an address to a model's subscription list, overwriting previous
* contents. Node response is reported with a "configuration status"
* event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param sub_address   The address to set as the subscription list. Note that the address has to be a group address.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_set_rsp_t* gecko_cmd_mesh_prov_model_sub_set(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_set_id+((12)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_set;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_set_va
*
* Deprecated. Replacement is "mesh_config_client_set_model_sub_va" command.
* Set a virtual address to a model's subscription list, overwriting
* previous contents. Node response is reported with a "configuration
* status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param sub_address   The Label UUID to set as the subscription list. The byte array must be exactly 16 bytes long.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_set_va_rsp_t* gecko_cmd_mesh_prov_model_sub_set_va(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id,uint8 sub_address_len, const uint8* sub_address_data)
{
    if ((uint16_t)sub_address_len > BGLIB_MSG_MAX_PAYLOAD - 11)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_model_sub_set_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_set_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.sub_address.len=sub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_model_sub_set_va.sub_address.data,sub_address_data,sub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_set_va_id+((11+sub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_set_va;
}

/** 
*
* gecko_cmd_mesh_prov_heartbeat_publication_get
*
* Deprecated. Replacement is "mesh_config_client_get_heartbeat_pub" command.
* Get heartbeat publication state of a             node. Node response
* will be reported as a "heartbeat             publication status"
* event.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   Network key index used to encrypt the request.
*
* Events generated
*
* gecko_evt_mesh_prov_heartbeat_publication_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_heartbeat_publication_get_rsp_t* gecko_cmd_mesh_prov_heartbeat_publication_get(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_get.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_heartbeat_publication_get_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_heartbeat_publication_get;
}

/** 
*
* gecko_cmd_mesh_prov_heartbeat_publication_set
*
* Deprecated. Replacement is "mesh_config_client_set_heartbeat_pub" command.
* Set heartbeat publication state of a             node. Node response
* will be reported as a "heartbeat             publication status"
* event.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   The Network Key index used in encrypting                     the request. 
* @param publication_address   Heartbeat publication address. The address cannot be                   a virtual address. Note that it can be the unassigned address, in                   which case the heartbeat publishing is disabled. 
* @param count_log   Heartbeat publication count setting. Valid values are as follows:{br}                   
*  - 0x00: Heartbeat messages are not sent
*  - 0x01 .. 0x11: Node shall send 2^(n-1) heartbeat messages
*  - 0x12 .. 0xfe: Prohibited
*  - 0xff: Hearbeat messages are sent indefinitely
* 
* @param period_log   Heartbeat publication period setting. Valid values are as follows:{br}                   
*  - 0x00: Heartbeat messages are not sent
*  - 0x01 .. 0x11: Node shall send a heartbeat message every 2^(n-1) seconds
*  - 0x12 .. 0xff: Prohibited
* 
* @param ttl   Time-to-live parameter for heartbeat messages
* @param features   Heartbeat trigger setting. For bits set in the bitmask,                   reconfiguration of the node feature associated with the bit will                   result in the node emitting a heartbeat message. Valid values are as follows:                   
*  - Bit 0: Relay feature
*  - Bit 1: Proxy feature
*  - Bit 2: Friend feature
*  - Bit 3: Low power feature
* Remaining bits are reserved for future use.                   
* @param publication_netkey_index   Index of the network key used to encrypt heartbeat messages.
*
* Events generated
*
* gecko_evt_mesh_prov_heartbeat_publication_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_heartbeat_publication_set_rsp_t* gecko_cmd_mesh_prov_heartbeat_publication_set(uint16 address,uint16 netkey_index,uint16 publication_address,uint8 count_log,uint8 period_log,uint8 ttl,uint16 features,uint16 publication_netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.publication_address=publication_address;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.count_log=count_log;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.period_log=period_log;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.features=features;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_publication_set.publication_netkey_index=publication_netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_heartbeat_publication_set_id+((13)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_heartbeat_publication_set;
}

/** 
*
* gecko_cmd_mesh_prov_heartbeat_subscription_get
*
* Deprecated. Replacement is "mesh_config_client_get_heartbeat_sub" command.
* Get node heartbeat subscription state. The             node will
* respond with a "subscription             status" event.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   The network key index used to encrypt the request
*
* Events generated
*
* gecko_evt_mesh_prov_heartbeat_subscription_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_heartbeat_subscription_get_rsp_t* gecko_cmd_mesh_prov_heartbeat_subscription_get(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_get.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_heartbeat_subscription_get_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_heartbeat_subscription_get;
}

/** 
*
* gecko_cmd_mesh_prov_heartbeat_subscription_set
*
* Deprecated. Replacement is "mesh_config_client_set_heartbeat_sub" command.
* Get node heartbeat subscription state. The             node will
* respond with a "subscription             status" event.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   The network key index used in encrypting                     the request. 
* @param subscription_source   Source address for heartbeat messages. Must                   be either a unicast address or the unassigned address, in                   which case heartbeat messages are not processed. 
* @param subscription_destination   Destination address for heartbeat messages. The                   address must be either the unicast address of the primary element                   of the node, a group address, or the unassigned address. If it                   is the unassigned address, heartbeat messages are not processed. 
* @param period_log   Heartbeat subscription period setting. Valid values are as follows:{br}                   
*  - 0x00: Heartbeat messages are not received
*  - 0x01 .. 0x11: Node shall receive heartbeat messages for 2^(n-1) seconds
*  - 0x12 .. 0xff: Prohibited
* 
*
* Events generated
*
* gecko_evt_mesh_prov_heartbeat_subscription_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_heartbeat_subscription_set_rsp_t* gecko_cmd_mesh_prov_heartbeat_subscription_set(uint16 address,uint16 netkey_index,uint16 subscription_source,uint16 subscription_destination,uint8 period_log)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_set.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_set.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_set.subscription_source=subscription_source;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_set.subscription_destination=subscription_destination;
    gecko_cmd_msg->data.cmd_mesh_prov_heartbeat_subscription_set.period_log=period_log;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_heartbeat_subscription_set_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_heartbeat_subscription_set;
}

/** 
*
* gecko_cmd_mesh_prov_relay_get
*
* Deprecated. Replacement is "mesh_config_client_get_relay" command.
* Get node relay retransmission state. The             node will respond
* with a "subscription             status" event.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   The network key index used in encrypting                     the request. 
*
* Events generated
*
* gecko_evt_mesh_prov_relay_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_relay_get_rsp_t* gecko_cmd_mesh_prov_relay_get(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_relay_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_relay_get.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_relay_get_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_relay_get;
}

/** 
*
* gecko_cmd_mesh_prov_relay_set
*
* Deprecated. Replacement is "mesh_config_client_set_relay" command.
* Set node relay retransmission state. The             node will respond
* with a "relay             status" event.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   The network key index used in encrypting                     the request. 
* @param relay   Relay state. Valid values are as follows:{br}                   
*  - 0x00: Relaying disabled
*  - 0x01: Relaying enabled
* 
* @param count   Relay retransmit count. Value must be between 0 and 7;                   default value is 0 (no retransmissions).
* @param interval   Relay retransmit interval in milliseconds. Value must be between 0 and 31;                   it represents 10-millisecond increments, starting at 10 ms.
*
* Events generated
*
* gecko_evt_mesh_prov_relay_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_relay_set_rsp_t* gecko_cmd_mesh_prov_relay_set(uint16 address,uint16 netkey_index,uint8 relay,uint8 count,uint8 interval)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_relay_set.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_relay_set.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_relay_set.relay=relay;
    gecko_cmd_msg->data.cmd_mesh_prov_relay_set.count=count;
    gecko_cmd_msg->data.cmd_mesh_prov_relay_set.interval=interval;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_relay_set_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_relay_set;
}

/** 
*
* gecko_cmd_mesh_prov_reset_node
*
* Deprecated. Replacement is "mesh_config_client_reset_node" command.
* Send a reset request to a node.
* If a node replies to the request, a "node reset" event will
* be generated. Note that the reply packet may get lost and
* the node has reset itself even in the absence of the
* event.
* Also note that for securely removing a node from the network
* a key refresh, with the removed node blacklisted, should be
* done.
*  
*
* @param address   Unicast address of the target node
* @param netkey_index   The network key index used in encrypting                     the request. 
*
* Events generated
*
* gecko_evt_mesh_prov_node_reset -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_reset_node_rsp_t* gecko_cmd_mesh_prov_reset_node(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_reset_node.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_reset_node.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_reset_node_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_reset_node;
}

/** 
*
* gecko_cmd_mesh_prov_appkey_get
*
* Deprecated. Replacement is "mesh_config_client_list_appkeys" command.
* Get a list of application keys bound to a network key on a node.
* This command is used to return a list of application key
* indices for the application keys bound to a particular
* network key on a node.
* Node response is reported with a number of "application key list
* "events, terminated by a "application key list             end" event.
*  
*
* @param address   Unicast address of the target Node's primary element
* @param netkey_index   Index of the network key to which the application keys are bound on the node
*
* Events generated
*
* gecko_evt_mesh_prov_appkey_list_end -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_appkey_get_rsp_t* gecko_cmd_mesh_prov_appkey_get(uint16 address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_appkey_get.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_appkey_get_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_appkey_get;
}

/** 
*
* gecko_cmd_mesh_prov_network_get
*
* Deprecated. Replacement is "mesh_config_client_list_netkeys" command.
* Get a list of network keys bound from a node.
* This command is used to return a list of network key
* indices of network keys deployed to a node.
* Node response is reported with a number of "network key list
* "events, terminated by a "network key list             end" event.
*  
*
* @param address   Unicast address of the target Node's primary element
*
* Events generated
*
* gecko_evt_mesh_prov_network_list_end -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_network_get_rsp_t* gecko_cmd_mesh_prov_network_get(uint16 address)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_network_get.address=address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_network_get_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_network_get;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_clear
*
* Deprecated. Replacement is "mesh_config_client_clear_model_sub" command.
* Clear all addresses from a model's subscription list. Node response is
* reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_clear_rsp_t* gecko_cmd_mesh_prov_model_sub_clear(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_clear.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_clear.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_clear.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_clear.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_clear.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_clear_id+((10)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_clear;
}

/** 
*
* gecko_cmd_mesh_prov_model_pub_get
*
* Deprecated. Replacement is "mesh_config_client_get_model_pub" command.
* Get a model's publication address, key, and parameters.  Node response
* is reported with a "model publication parameters" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
*
* Events generated
*
* gecko_evt_mesh_prov_model_pub_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_pub_get_rsp_t* gecko_cmd_mesh_prov_model_pub_get(uint16 address,uint16 elem_address,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_get.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_get.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_get.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_pub_get_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_get;
}

/** 
*
* gecko_cmd_mesh_prov_model_pub_set_va
*
* Deprecated. Replacement is "mesh_config_client_set_model_pub_va" command.
* Set a model's publication virtual address, key, and parameters.  Node
* response is reported with a "configuration status" event.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param appkey_index   The application key index to use for the published messages. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param ttl   Publication time-to-live value
* @param period   Publication period encoded as step count and step resolution. The encoding is as follows:                   
*  - Bits 0..5: Step count
*  - Bits 6..7: Step resolution:                     
*  - 00: 100 milliseconds
*  - 01: 1 second
*  - 10: 10 seconds
*  - 11: 10 minutes
* 
* @param retrans   Refer to the documentation of "model publication set command" for details.
* @param pub_address   The Label UUID to publish to. The byte array must be exactly 16 bytes long.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_pub_set_va_rsp_t* gecko_cmd_mesh_prov_model_pub_set_va(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id,uint8 ttl,uint8 period,uint8 retrans,uint8 pub_address_len, const uint8* pub_address_data)
{
    if ((uint16_t)pub_address_len > BGLIB_MSG_MAX_PAYLOAD - 16)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.period=period;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.retrans=retrans;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.pub_address.len=pub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va.pub_address.data,pub_address_data,pub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_pub_set_va_id+((16+pub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_va;
}

/** 
*
* gecko_cmd_mesh_prov_model_pub_set_cred
*
* Deprecated. Replacement is "mesh_config_client_set_model_pub" command.
* This command is otherwise the same as "the regular model publication
* set command" but it also has a parameter for setting the Friendship
* Credential Flag.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param appkey_index   The application key index to use for the published messages. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param pub_address   The address to publish to. Can be a unicast address, a virtual address, or a group address; can also be the unassigned address to stop the model from publishing. 
* @param ttl   Publication time-to-live value
* @param period   Publication period encoded as step count and step resolution. The encoding is as follows:                   
*  - Bits 0..5: Step count
*  - Bits 6..7: Step resolution:                     
*  - 00: 100 milliseconds
*  - 01: 1 second
*  - 10: 10 seconds
*  - 11: 10 minutes
* 
* @param retrans   Refer to the documentation of "model publication set command" for details.
* @param credentials   Friendship credential flag. If zero,                   publication is done using normal credentials; if                   one, it is done with friendship credentials, meaning                   only the friend can decrypt the published message                   and relay it forward using the normal credentials.                   The default value is 0.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_pub_set_cred_rsp_t* gecko_cmd_mesh_prov_model_pub_set_cred(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id,uint16 pub_address,uint8 ttl,uint8 period,uint8 retrans,uint8 credentials)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.pub_address=pub_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.period=period;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.retrans=retrans;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_cred.credentials=credentials;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_pub_set_cred_id+((18)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_cred;
}

/** 
*
* gecko_cmd_mesh_prov_model_pub_set_va_cred
*
* Deprecated. Replacement is "mesh_config_client_set_model_pub_va" command.
* This command is otherwise the same as "the regular model publication
* set virtual address command" but it also has a parameter for setting
* the Friendship Credential Flag.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param appkey_index   The application key index to use for the published messages. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param ttl   Publication time-to-live value
* @param period   Publication period encoded as step count and step resolution. The encoding is as follows:                   
*  - Bits 0..5: Step count
*  - Bits 6..7: Step resolution:                     
*  - 00: 100 milliseconds
*  - 01: 1 second
*  - 10: 10 seconds
*  - 11: 10 minutes
* 
* @param retrans   Refer to the documentation of "model publication set command" for details.
* @param credentials   Friendship credential flag. If zero,                   publication is done using normal credentials; if                   one, it is done with friendship credentials, meaning                   only the friend can decrypt the published message                   and relay it forward using the normal credentials.                   The default value is 0.
* @param pub_address   The Label UUID to publish to. The byte array must be exactly 16 bytes long.
*
* Events generated
*
* gecko_evt_mesh_prov_config_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_pub_set_va_cred_rsp_t* gecko_cmd_mesh_prov_model_pub_set_va_cred(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id,uint8 ttl,uint8 period,uint8 retrans,uint8 credentials,uint8 pub_address_len, const uint8* pub_address_data)
{
    if ((uint16_t)pub_address_len > BGLIB_MSG_MAX_PAYLOAD - 17)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_va_cred.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_va_cred;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.period=period;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.retrans=retrans;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.credentials=credentials;
    gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.pub_address.len=pub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_model_pub_set_va_cred.pub_address.data,pub_address_data,pub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_pub_set_va_cred_id+((17+pub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_pub_set_va_cred;
}

/** 
*
* gecko_cmd_mesh_prov_model_sub_get
*
* Deprecated. Replacement is "mesh_config_client_list_subs" command.
* Get a model's subscription list. Node response is reported with
* "subscription list entry" and "subscription list entries end" events.
*  
*
* @param address   Unicast address of the target node
* @param elem_address   Unicast address of the element containing the model to be configured. 
* @param netkey_index   The network key index used for encrypting the request. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
*
* Events generated
*
* gecko_evt_mesh_prov_model_sub_addr - 
* gecko_evt_mesh_prov_model_sub_addr_end -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_model_sub_get_rsp_t* gecko_cmd_mesh_prov_model_sub_get(uint16 address,uint16 elem_address,uint16 netkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_get.elem_address=elem_address;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_get.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_get.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_prov_model_sub_get.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_model_sub_get_id+((10)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_model_sub_get;
}

/** 
*
* gecko_cmd_mesh_prov_friend_timeout_get
*
* Deprecated. Replacement is "mesh_config_client_get_lpn_polltimeout" command.
* LPN poll timeout request. Result is reported in a " friend poll
* timeout             status" event.
*  
*
* @param address   Unicast address of the friend node
* @param netkey_index   The network key index used in encrypting                     the request. 
* @param lpn_address   Unicast address of the LPN node
*
* Events generated
*
* gecko_evt_mesh_prov_friend_timeout_status -     
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_friend_timeout_get_rsp_t* gecko_cmd_mesh_prov_friend_timeout_get(uint16 address,uint16 netkey_index,uint16 lpn_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_friend_timeout_get.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_friend_timeout_get.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_friend_timeout_get.lpn_address=lpn_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_friend_timeout_get_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_friend_timeout_get;
}

/** 
*
* gecko_cmd_mesh_prov_get_default_configuration_timeout
*
* Deprecated. Replacement is "mesh_config_client_get_default_timeout" command.
* Get the default timeout for configuration             client requests.
* If there is no response when the timeout             expires a
* configuration request is considered to have             failed.
*  
*    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_get_default_configuration_timeout_rsp_t* gecko_cmd_mesh_prov_get_default_configuration_timeout()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_get_default_configuration_timeout_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_get_default_configuration_timeout;
}

/** 
*
* gecko_cmd_mesh_prov_set_default_configuration_timeout
*
* Deprecated. Replacement is "mesh_config_client_set_default_timeout" command.
* Set the default timeout for configuration             client requests.
* If there is no response when the timeout             expires a
* configuration request is considered to have             failed.
*  
*
* @param timeout   Timeout in milliseconds. Default timeout                   is 5s (5000 ms)
* @param lpn_timeout   Timeout in milliseconds when                   communicating with an LPN node. Default LPN timeout is                   120s (120000 ms)    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_prov_set_default_configuration_timeout_rsp_t* gecko_cmd_mesh_prov_set_default_configuration_timeout(uint32 timeout,uint32 lpn_timeout)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_set_default_configuration_timeout.timeout=timeout;
    gecko_cmd_msg->data.cmd_mesh_prov_set_default_configuration_timeout.lpn_timeout=lpn_timeout;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_set_default_configuration_timeout_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_set_default_configuration_timeout;
}

/** 
*
* gecko_cmd_mesh_prov_provision_device_with_address
*
* Provision a device into a network using the             advertisement
* bearer (PB-ADV). Application may specify             the unicast
* addresses given for the device elements with             this command.
* Issuing this command starts the provisioning process for
* the specified device. Once the process completes
* successfully, a "device             provisioned event" is generated.
* If provisioning does             not succeed, a " provisioning
* failed event" will be generated instead.
*  
*
* @param network_id   Index of the initial network key                     which is sent to the device during                     provisioning.
* @param address   Unicast address for the primary element of th edevice. If unassigned address (0x0000) is given, the stack will automatically assign an address. Note that element addresses are contiguous and must all be unicast addresses; primary address must be chosen so that also the last element's address will still be a unicast address. 
* @param elements   Number of elements in device to be provisioned. If the primary element address is set to unassigned, this needs to be set to zero. 
* @param attention_timer   Attention timer value, in seconds, tells for how long the device being provisioned should attract human attention.
* @param uuid   UUID of the device to provision
*
* Events generated
*
* gecko_evt_mesh_prov_device_provisioned - 
* gecko_evt_mesh_prov_provisioning_failed -     
*
**/

static inline struct gecko_msg_mesh_prov_provision_device_with_address_rsp_t* gecko_cmd_mesh_prov_provision_device_with_address(uint8 network_id,uint16 address,uint8 elements,uint8 attention_timer,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 6)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_provision_device_with_address.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_provision_device_with_address;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device_with_address.network_id=network_id;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device_with_address.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device_with_address.elements=elements;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device_with_address.attention_timer=attention_timer;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_device_with_address.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_provision_device_with_address.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_provision_device_with_address_id+((6+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_provision_device_with_address;
}

/** 
*
* gecko_cmd_mesh_prov_provision_gatt_device_with_address
*
* Provision a device into a network using the             GATT bearer
* (PB-GATT). Application may specify the unicast             addresses
* given for the device elements with this command.
* Issuing this command starts the provisioning process for
* the specified device. Once the process completes
* successfully, a "device             provisioned event" is generated.
* If provisioning does             not succeed, a " provisioning
* failed event" will be generated instead.
* Note that this command is available only if GATT
* functionality is compiled in to the firmware. If that is
* not the case, the command will return with a "not
* implemented" return code.
*  
*
* @param network_id   Index of the initial network key                     which is sent to the device during                     provisioning.
* @param connection   Connection handle for the device to be provisioned                    
* @param address   Unicast address for the primary element of th edevice. If unassigned address (0x0000) is given, the stack will automatically assign an address. Note that element addresses are contiguous and must all be unicast addresses; primary address must be chosen so that also the last element's address will still be a unicast address. 
* @param elements   Number of elements in device to be provisioned. If the primary element address is set to unassigned, this needs to be set to zero. 
* @param attention_timer   Attention timer value, in seconds, tells for how long the device being provisioned should attract human attention.
* @param uuid   UUID of the device to provision
*
* Events generated
*
* gecko_evt_mesh_prov_device_provisioned - 
* gecko_evt_mesh_prov_provisioning_failed -     
*
**/

static inline struct gecko_msg_mesh_prov_provision_gatt_device_with_address_rsp_t* gecko_cmd_mesh_prov_provision_gatt_device_with_address(uint8 network_id,uint8 connection,uint16 address,uint8 elements,uint8 attention_timer,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 7)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_provision_gatt_device_with_address.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_provision_gatt_device_with_address;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.network_id=network_id;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.connection=connection;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.elements=elements;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.attention_timer=attention_timer;
    gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_provision_gatt_device_with_address.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_provision_gatt_device_with_address_id+((7+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_provision_gatt_device_with_address;
}

/** 
*
* gecko_cmd_mesh_prov_initialize_network
*
* Initialize mesh network and assign provisioner address and IV index for the network.             If this command is not invoked prior to invoking              "mesh prov create network"             then the network will be initialized with default address and IV index.              
*
* @param address   Address to assign for provisioner.                     
* @param ivi   IV index of the network.                         
*
**/

static inline struct gecko_msg_mesh_prov_initialize_network_rsp_t* gecko_cmd_mesh_prov_initialize_network(uint16 address,uint32 ivi)
{
    
    gecko_cmd_msg->data.cmd_mesh_prov_initialize_network.address=address;
    gecko_cmd_msg->data.cmd_mesh_prov_initialize_network.ivi=ivi;
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_initialize_network_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_initialize_network;
}

/** 
*
* gecko_cmd_mesh_prov_get_key_refresh_appkey_blacklist
*
* Check the application key refresh blacklist           status of a node. Nodes which are blacklisted for a given           application key do not receive updates for that particular           application key, but do participate in the key refresh           procedure as a whole. This enables the Provisioner to           set up and update restricted sets of application keys           across the nodes. 
*
* @param netkey_index   Network key index
* @param appkey_index   Application key index
* @param uuid   UUID of the Device    
*
**/

static inline struct gecko_msg_mesh_prov_get_key_refresh_appkey_blacklist_rsp_t* gecko_cmd_mesh_prov_get_key_refresh_appkey_blacklist(uint16 netkey_index,uint16 appkey_index,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 5)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_get_key_refresh_appkey_blacklist.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_get_key_refresh_appkey_blacklist;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_appkey_blacklist.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_appkey_blacklist.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_appkey_blacklist.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_get_key_refresh_appkey_blacklist.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_get_key_refresh_appkey_blacklist_id+((5+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_get_key_refresh_appkey_blacklist;
}

/** 
*
* gecko_cmd_mesh_prov_set_key_refresh_appkey_blacklist
*
* Set the application key refresh blacklist           status of a node. Nodes which are blacklisted for a given           application key do not receive updates for that particular           application key, but do participate in the key refresh           procedure as a whole. This enables the Provisioner to           set up and update restricted sets of application keys           across the nodes.            
*
* @param netkey_index   Network key index
* @param appkey_index   Application key index
* @param status   Nonzero for blacklisted node
* @param uuid   UUID of the Device    
*
**/

static inline struct gecko_msg_mesh_prov_set_key_refresh_appkey_blacklist_rsp_t* gecko_cmd_mesh_prov_set_key_refresh_appkey_blacklist(uint16 netkey_index,uint16 appkey_index,uint8 status,uint8 uuid_len, const uint8* uuid_data)
{
    if ((uint16_t)uuid_len > BGLIB_MSG_MAX_PAYLOAD - 6)
    {
        gecko_rsp_msg->data.rsp_mesh_prov_set_key_refresh_appkey_blacklist.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_prov_set_key_refresh_appkey_blacklist;
    }

    
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_appkey_blacklist.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_appkey_blacklist.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_appkey_blacklist.status=status;
    gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_appkey_blacklist.uuid.len=uuid_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_prov_set_key_refresh_appkey_blacklist.uuid.data,uuid_data,uuid_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_set_key_refresh_appkey_blacklist_id+((6+uuid_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_set_key_refresh_appkey_blacklist;
}

/** 
*
* gecko_cmd_mesh_prov_stop_scan_unprov_beacons
*
* Stop scanning for unprovisioned device             beacons.
*  
*    
*
**/

static inline struct gecko_msg_mesh_prov_stop_scan_unprov_beacons_rsp_t* gecko_cmd_mesh_prov_stop_scan_unprov_beacons()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_prov_stop_scan_unprov_beacons_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_prov_stop_scan_unprov_beacons;
}

/** 
*
* gecko_cmd_mesh_proxy_connect
*
* Start connecting a proxy client to a proxy server.           Once the connection is complete, a "           connection established" event will be generated. LE-connection must be opened prior           to opening proxy connection 
*
* @param connection   Connection handle    
*
**/

static inline struct gecko_msg_mesh_proxy_connect_rsp_t* gecko_cmd_mesh_proxy_connect(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_mesh_proxy_connect.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_mesh_proxy_connect_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_proxy_connect;
}

/** 
*
* gecko_cmd_mesh_proxy_disconnect
*
* Disconnect. This call can be used also for a connection           which is not yet fully formed. 
*
* @param handle   Proxy handle    
*
**/

static inline struct gecko_msg_mesh_proxy_disconnect_rsp_t* gecko_cmd_mesh_proxy_disconnect(uint32 handle)
{
    
    gecko_cmd_msg->data.cmd_mesh_proxy_disconnect.handle=handle;
    gecko_cmd_msg->header=((gecko_cmd_mesh_proxy_disconnect_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_proxy_disconnect;
}

/** 
*
* gecko_cmd_mesh_proxy_set_filter_type
*
* Set up proxy filtering type. At the proxy server side this is a local           configuration, while on the proxy client a proxy configuration PDU will be sent           to the proxy server. 
*
* @param handle   Proxy handle
* @param type   Filter type: 0x00 for whitelist, 0x01 for blacklist.
* @param key   Network key index used in encrypting the request to                 the proxy server.    
*
**/

static inline struct gecko_msg_mesh_proxy_set_filter_type_rsp_t* gecko_cmd_mesh_proxy_set_filter_type(uint32 handle,uint8 type,uint16 key)
{
    
    gecko_cmd_msg->data.cmd_mesh_proxy_set_filter_type.handle=handle;
    gecko_cmd_msg->data.cmd_mesh_proxy_set_filter_type.type=type;
    gecko_cmd_msg->data.cmd_mesh_proxy_set_filter_type.key=key;
    gecko_cmd_msg->header=((gecko_cmd_mesh_proxy_set_filter_type_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_proxy_set_filter_type;
}

/** 
*
* gecko_cmd_mesh_proxy_allow
*
* Allow messages destined to the given address to be forwarded           over the proxy connection to the proxy client. At the proxy server side this is a local           configuration, while on the proxy client a proxy configuration PDU will be sent           to the proxy server. 
*
* @param handle   Proxy handle
* @param address   Destination address to allow. The address may be either                 a unicast address, a group address, or a virtual address. 
* @param key   Network key index used in encrypting the request to                 the proxy server.    
*
**/

static inline struct gecko_msg_mesh_proxy_allow_rsp_t* gecko_cmd_mesh_proxy_allow(uint32 handle,uint16 address,uint16 key)
{
    
    gecko_cmd_msg->data.cmd_mesh_proxy_allow.handle=handle;
    gecko_cmd_msg->data.cmd_mesh_proxy_allow.address=address;
    gecko_cmd_msg->data.cmd_mesh_proxy_allow.key=key;
    gecko_cmd_msg->header=((gecko_cmd_mesh_proxy_allow_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_proxy_allow;
}

/** 
*
* gecko_cmd_mesh_proxy_deny
*
* Block messages destined to the given address from being forwarded           over the proxy connection to the proxy client. At the proxy server side this is a local           configuration, while on the proxy client a proxy configuration PDU will be sent           to the proxy server. 
*
* @param handle   Proxy handle
* @param address   Destination address to block. The address may be either                 a unicast address, a group address, or a virtual address. 
* @param key   Network key index used in encrypting the request to                 the proxy server.    
*
**/

static inline struct gecko_msg_mesh_proxy_deny_rsp_t* gecko_cmd_mesh_proxy_deny(uint32 handle,uint16 address,uint16 key)
{
    
    gecko_cmd_msg->data.cmd_mesh_proxy_deny.handle=handle;
    gecko_cmd_msg->data.cmd_mesh_proxy_deny.address=address;
    gecko_cmd_msg->data.cmd_mesh_proxy_deny.key=key;
    gecko_cmd_msg->header=((gecko_cmd_mesh_proxy_deny_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_proxy_deny;
}

/** 
*
* gecko_cmd_mesh_vendor_model_send
*
* Send vendor specific data
* Note that due to bgapi event length restrictions the
* message sent may need to be fragmented into several
* commands. If this is the case, the application must issue
* the commands in the correct order and mark the command
* carrying the last message fragment with the final flag set
* to a nonzero value. The stack will not start sending the
* message until the complete message is provided by the
* application. Fragments from multiple messages must not             be
* interleaved.
*  
*
* @param elem_index   Sending model element index
* @param vendor_id   Vendor ID of the sending model
* @param model_id   Model ID of the sending model
* @param destination_address   Destination address of the message. Can be a unicast                   address, a group address, or a virtual address. 
* @param va_index   Index of the destination Label UUID (used only is the destination address is a virtual address)
* @param appkey_index   The application key index used.
* @param nonrelayed   If the message is a response to                     a received message, set this parameter according                     to what was received in the receive event;                     otherwise set to nonzero if the message should                     affect only devices in the immediate radio                     neighborhood.                     
* @param opcode   Message opcode
* @param final   Whether this payload chunk is the                     final one of the message or whether more will                     follow
* @param payload   Payload data (either complete or                   partial; see final parameter).     
*
**/

static inline struct gecko_msg_mesh_vendor_model_send_rsp_t* gecko_cmd_mesh_vendor_model_send(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint16 destination_address,int8 va_index,uint16 appkey_index,uint8 nonrelayed,uint8 opcode,uint8 final,uint8 payload_len, const uint8* payload_data)
{
    if ((uint16_t)payload_len > BGLIB_MSG_MAX_PAYLOAD - 15)
    {
        gecko_rsp_msg->data.rsp_mesh_vendor_model_send.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_vendor_model_send;
    }

    
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.destination_address=destination_address;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.va_index=va_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.nonrelayed=nonrelayed;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.opcode=opcode;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.final=final;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_send.payload.len=payload_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_vendor_model_send.payload.data,payload_data,payload_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_vendor_model_send_id+((15+payload_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_vendor_model_send;
}

/** 
*
* gecko_cmd_mesh_vendor_model_set_publication
*
* Set vendor model publication message.
* The model publication message will be sent out when model
* publication occurs either periodically (if the model is
* configured for periodc publishing) or explicitly (see "vendor model
* publish             command".
* Note that due to bgapi length requirements the message may
* need to be fragmented over multiple commands.             If this is
* the case, the application must issue             the commands in the
* correct order and mark the command             carrying the last
* message fragment with the final flag set             to a nonzero
* value. The stack will not assign the message             to the model
* until the complete message is provided by the             application.
* To disable publication the publication message may be
* erased using the " clear             vendor model publication message"
* command.
*  
*
* @param elem_index   Publishing model element index
* @param vendor_id   Vendor ID of the model
* @param model_id   Model ID of the model
* @param opcode   Message opcode
* @param final   Whether this payload chunk is the                     final one of the message or whether more will                     follow
* @param payload   Payload data (either complete or                     partial; see final parameter).     
*
**/

static inline struct gecko_msg_mesh_vendor_model_set_publication_rsp_t* gecko_cmd_mesh_vendor_model_set_publication(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint8 opcode,uint8 final,uint8 payload_len, const uint8* payload_data)
{
    if ((uint16_t)payload_len > BGLIB_MSG_MAX_PAYLOAD - 9)
    {
        gecko_rsp_msg->data.rsp_mesh_vendor_model_set_publication.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_vendor_model_set_publication;
    }

    
    gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.opcode=opcode;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.final=final;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.payload.len=payload_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_vendor_model_set_publication.payload.data,payload_data,payload_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_vendor_model_set_publication_id+((9+payload_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_vendor_model_set_publication;
}

/** 
*
* gecko_cmd_mesh_vendor_model_clear_publication
*
* Clear vendor model publication message.
* Clearing the model publication message disables model
* publishing; it can be re-enabled by defining the publication
* message using the "             set vendor model publication" command.
*  
*
* @param elem_index   Publishing model element index
* @param vendor_id   Vendor ID of the model
* @param model_id   Model ID of the model    
*
**/

static inline struct gecko_msg_mesh_vendor_model_clear_publication_rsp_t* gecko_cmd_mesh_vendor_model_clear_publication(uint16 elem_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_vendor_model_clear_publication.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_clear_publication.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_clear_publication.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_vendor_model_clear_publication_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_vendor_model_clear_publication;
}

/** 
*
* gecko_cmd_mesh_vendor_model_publish
*
* Publish vendor model publication message.
* Sends the stored publication message to the network             using
* the application key and destination address             stored in the
* model publication parameters.
*  
*
* @param elem_index   Publishing model element index
* @param vendor_id   Vendor ID of the model
* @param model_id   Model ID of the model    
*
**/

static inline struct gecko_msg_mesh_vendor_model_publish_rsp_t* gecko_cmd_mesh_vendor_model_publish(uint16 elem_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_vendor_model_publish.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_publish.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_publish.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_vendor_model_publish_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_vendor_model_publish;
}

/** 
*
* gecko_cmd_mesh_vendor_model_init
*
* Initialize the vendor model. This function             has to be
* called before the model can be used. Note that             the model
* can be deinitialized if it is not needed             anymore; see
* "deinitialization             command".
* It is necessary to define the opcodes the model is able to
* receive at initialization. This enables the stack to pass
* only valid messages up to the model during runtime.  Per
* Mesh specification there are up to 64 opcodes per vendor,
* ranging from 0 to 63. Specifying opcodes outside of that
* range will result in an error response. Duplicate opcodes
* in the array do not result in an error, but will of course
* be recorded only once.
*  
*
* @param elem_index   Model element index
* @param vendor_id   Vendor ID of the model
* @param model_id   Model ID of the model
* @param publish   Indicates if the model is a publish                     model (nonzero) or not (zero). 
* @param opcodes   Array of opcodes the model can handle.     
*
**/

static inline struct gecko_msg_mesh_vendor_model_init_rsp_t* gecko_cmd_mesh_vendor_model_init(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint8 publish,uint8 opcodes_len, const uint8* opcodes_data)
{
    if ((uint16_t)opcodes_len > BGLIB_MSG_MAX_PAYLOAD - 8)
    {
        gecko_rsp_msg->data.rsp_mesh_vendor_model_init.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_vendor_model_init;
    }

    
    gecko_cmd_msg->data.cmd_mesh_vendor_model_init.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_init.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_init.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_init.publish=publish;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_init.opcodes.len=opcodes_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_vendor_model_init.opcodes.data,opcodes_data,opcodes_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_vendor_model_init_id+((8+opcodes_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_vendor_model_init;
}

/** 
*
* gecko_cmd_mesh_vendor_model_deinit
*
* Deinitialize the model. After this call the             model cannot be used until it is initialized again; see "initialization             command".  
*
* @param elem_index   Model element index
* @param vendor_id   Vendor ID of the model
* @param model_id   Model ID of the model    
*
**/

static inline struct gecko_msg_mesh_vendor_model_deinit_rsp_t* gecko_cmd_mesh_vendor_model_deinit(uint16 elem_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_vendor_model_deinit.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_deinit.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_vendor_model_deinit.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_vendor_model_deinit_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_vendor_model_deinit;
}

/** 
*
* gecko_cmd_mesh_health_client_get
*
* Get the registered fault status of a Health             Server model
* or models in the network.
* Besides the immediate result code the response or
* responses (in case the destination server address is a
* group address) from the network will generate "             server
* status report events".
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.
* @param vendor_id   Bluetooth vendor ID used in the request.    
*
**/

static inline struct gecko_msg_mesh_health_client_get_rsp_t* gecko_cmd_mesh_health_client_get(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint16 vendor_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_get.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_get.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_get.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_get.vendor_id=vendor_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_get_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_get;
}

/** 
*
* gecko_cmd_mesh_health_client_clear
*
* Clear the fault status of a Health             Server model or models
* in the network.
* Besides the immediate result code the response or
* responses (in case the destination server address is a
* group address) from the network will generate "             server
* status report events".
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.
* @param vendor_id   Bluetooth vendor ID used in the request.
* @param reliable   If nonzero a reliable model message is used.    
*
**/

static inline struct gecko_msg_mesh_health_client_clear_rsp_t* gecko_cmd_mesh_health_client_clear(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint16 vendor_id,uint8 reliable)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_clear.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_clear.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_clear.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_clear.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_health_client_clear.reliable=reliable;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_clear_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_clear;
}

/** 
*
* gecko_cmd_mesh_health_client_test
*
* Execute a self test on a             server model or models in the
* network
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.
* @param test_id   Test ID used in the request. 
* @param vendor_id   Bluetooth vendor ID used in the request.
* @param reliable   If nonzero a reliable model message is used.    
*
**/

static inline struct gecko_msg_mesh_health_client_test_rsp_t* gecko_cmd_mesh_health_client_test(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 test_id,uint16 vendor_id,uint8 reliable)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_test.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_test.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_test.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_test.test_id=test_id;
    gecko_cmd_msg->data.cmd_mesh_health_client_test.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_health_client_test.reliable=reliable;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_test_id+((10)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_test;
}

/** 
*
* gecko_cmd_mesh_health_client_get_period
*
* Get the health period log of a Health             Server model or
* models in the network.
* Besides the immediate result code the response or
* responses (in case the destination server address is a
* group address) from the network will generate "             server
* status report events".
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.    
*
**/

static inline struct gecko_msg_mesh_health_client_get_period_rsp_t* gecko_cmd_mesh_health_client_get_period(uint16 elem_index,uint16 server_address,uint16 appkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_get_period.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_get_period.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_get_period.appkey_index=appkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_get_period_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_get_period;
}

/** 
*
* gecko_cmd_mesh_health_client_set_period
*
* Set the health period divisor of a Health             Server model or
* models in the network.
* Besides the immediate result code the response or
* responses (in case the destination server address is a
* group address) from the network will generate "             server
* status report events".
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.
* @param period   Health period divisor value.
* @param reliable   If nonzero a reliable model message is used.    
*
**/

static inline struct gecko_msg_mesh_health_client_set_period_rsp_t* gecko_cmd_mesh_health_client_set_period(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 period,uint8 reliable)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_set_period.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_period.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_period.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_period.period=period;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_period.reliable=reliable;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_set_period_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_set_period;
}

/** 
*
* gecko_cmd_mesh_health_client_get_attention
*
* Get the attention timer value of a Health             Server model or
* models in the network.
* Besides the immediate result code the response or
* responses (in case the destination server address is a
* group address) from the network will generate "             server
* status report events".
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.    
*
**/

static inline struct gecko_msg_mesh_health_client_get_attention_rsp_t* gecko_cmd_mesh_health_client_get_attention(uint16 elem_index,uint16 server_address,uint16 appkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_get_attention.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_get_attention.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_get_attention.appkey_index=appkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_get_attention_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_get_attention;
}

/** 
*
* gecko_cmd_mesh_health_client_set_attention
*
* Set the attention timer value of a Health             Server model or
* models in the network.
* Besides the immediate result code the response or
* responses (in case the destination server address is a
* group address) from the network will generate "             server
* status report events".
*  
*
* @param elem_index   Client model element index.                     Identifies the client model used for sending the                     request.
* @param server_address   Destination server model address. May                     be a unicast address or a group                     address.
* @param appkey_index   The application key index to use in                     encrypting the request.
* @param attention   Attention timer period in seconds
* @param reliable   If nonzero a reliable model message is used.    
*
**/

static inline struct gecko_msg_mesh_health_client_set_attention_rsp_t* gecko_cmd_mesh_health_client_set_attention(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 attention,uint8 reliable)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_client_set_attention.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_attention.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_attention.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_attention.attention=attention;
    gecko_cmd_msg->data.cmd_mesh_health_client_set_attention.reliable=reliable;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_client_set_attention_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_client_set_attention;
}

/** 
*
* gecko_cmd_mesh_health_server_set_fault
*
* Set fault condition on an element. 
*
* @param elem_index   Index of the element on which the fault is occurring.               
* @param id   Fault ID. Refer to the Mesh Profile specification for                 IDs defined by the Bluetooth SIG.                   
*
**/

static inline struct gecko_msg_mesh_health_server_set_fault_rsp_t* gecko_cmd_mesh_health_server_set_fault(uint16 elem_index,uint8 id)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_server_set_fault.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_server_set_fault.id=id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_server_set_fault_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_server_set_fault;
}

/** 
*
* gecko_cmd_mesh_health_server_clear_fault
*
* Clear fault condition on an element. 
*
* @param elem_index   Index of the element on which the fault is no longer occurring.                   
* @param id   Fault ID. Refer to the Mesh Profile specification for                     IDs defined by the Bluetooth SIG.                       
*
**/

static inline struct gecko_msg_mesh_health_server_clear_fault_rsp_t* gecko_cmd_mesh_health_server_clear_fault(uint16 elem_index,uint8 id)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_server_clear_fault.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_server_clear_fault.id=id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_server_clear_fault_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_server_clear_fault;
}

/** 
*
* gecko_cmd_mesh_health_server_test_response
*
* Indicate to the stack that a test request has             been completed, and that the status may be communicated to             the Health Client which made the test request.               
*
* @param elem_index   Server model element index.                     Identifies the Server model that received the                     request as well as the element on which the test                     is to be performed.
* @param client_address   Address of the client model which                     sent the message
* @param appkey_index   The application key index to use in                     encrypting the request.
* @param vendor_id   Bluetooth vendor ID used in the request.                       
*
**/

static inline struct gecko_msg_mesh_health_server_test_response_rsp_t* gecko_cmd_mesh_health_server_test_response(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint16 vendor_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_health_server_test_response.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_health_server_test_response.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_health_server_test_response.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_health_server_test_response.vendor_id=vendor_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_health_server_test_response_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_health_server_test_response;
}

/** 
*
* gecko_cmd_mesh_generic_client_get
*
* Get the current state of a server             model or models in the
* network. Besides the immediate             result code, the response
* or responses from the network             will generate server state
* report events for the replies             received.
* The server model responses will be reported in             "
* server status" events.
*  
*
* @param model_id   Client model ID
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param type   Model-specific state type, identifying                   the kind of state to retrieve. See get                   state types list for details.
*
* Events generated
*
* gecko_evt_mesh_generic_client_server_status -     
*
**/

static inline struct gecko_msg_mesh_generic_client_get_rsp_t* gecko_cmd_mesh_generic_client_get(uint16 model_id,uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 type)
{
    
    gecko_cmd_msg->data.cmd_mesh_generic_client_get.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get.type=type;
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_client_get_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_client_get;
}

/** 
*
* gecko_cmd_mesh_generic_client_set
*
* Set the current state of a server             model or models in the
* network. Besides the immediate             result code, the response
* or responses from the network             will generate erver state
* report events for the replies             received.
* The server model responses will be reported in " server
* status" events. Note that for responses to be generated
* the corresponding flag needs to be set.
*  
*
* @param model_id   Client model ID
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param tid   Transaction identifier. This applies to those                   messages the Mesh Model specification defines as transactional                   and can be left as zero for others.
* @param transition   Transition time (in milliseconds) for                     the state
* change. If both the transition time and                     the delay
* are zero the transition is immediate.
* This applies to messages the Mesh Model
* specification                     defines to have transition and delay
* times, and                     can be left as zero for others.
* 
* @param delay   Delay time (in milliseconds) before                     starting the
* state change. If both the transition                     time and the
* delay are zero the transition is                     immediate.
* This applies to messages the Mesh Model
* specification                     defines to have transition and delay
* times, and                     can be left as zero for others.
* 
* @param flags   Message flags. Bitmask of the following:                     
*  - Bit 0: Response required. If nonzero client expects                     a response from the server
*  - Bit 1: Default transition timer.                     If nonzero client requests                     that server uses its default transition timer and the                     supplied transition and delay values are ignored.
* 
* @param type   Model-specific request type. See set                     request types list for details.
* @param parameters   Message-specific set request parameters                     serialized into a byte array.
*
* Events generated
*
* gecko_evt_mesh_generic_client_server_status -     
*
**/

static inline struct gecko_msg_mesh_generic_client_set_rsp_t* gecko_cmd_mesh_generic_client_set(uint16 model_id,uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 tid,uint32 transition,uint16 delay,uint16 flags,uint8 type,uint8 parameters_len, const uint8* parameters_data)
{
    if ((uint16_t)parameters_len > BGLIB_MSG_MAX_PAYLOAD - 19)
    {
        gecko_rsp_msg->data.rsp_mesh_generic_client_set.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_generic_client_set;
    }

    
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.tid=tid;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.transition=transition;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.delay=delay;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.type=type;
    gecko_cmd_msg->data.cmd_mesh_generic_client_set.parameters.len=parameters_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_generic_client_set.parameters.data,parameters_data,parameters_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_client_set_id+((19+parameters_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_client_set;
}

/** 
*
* gecko_cmd_mesh_generic_client_publish
*
* Publish a set request to the network using             the publish
* address and publish application key of the             model. The
* message will be received by the server models             which
* subscribe to the publish address, and there's no             need to
* explicitly specify a destination address or             application
* key.
* The server model responses will be reported in " server
* status" events. Note that for responses to be generated
* the corresponding flag needs to be set.
*  
*
* @param model_id   Client model ID
* @param elem_index   Client model element index
* @param tid   Transaction identifier
* @param transition   Transition time (in milliseconds) for                     the state
* change. If both the transition time and                     the delay
* are zero the transition is immediate.
* This applies to messages the Mesh Model
* specification                     defines to have transition and delay
* times, and                     can be left as zero for others.
* 
* @param delay   Delay time (in milliseconds) before                     starting the
* state change. If both the transition                     time and the
* delay are zero the transition is                     immediate.
* This applies to messages the Mesh Model
* specification                     defines to have transition and delay
* times, and                     can be left as zero for others.
* 
* @param flags   Message flags. Bitmask of the following:                     
*  - Bit 0: Response required. If nonzero client expects                     a response from the server
*  - Bit 1: Default transition timer.                     If nonzero client requests                     that server uses its default transition timer and the                     supplied transition and delay values are ignored.
* 
* @param type   Model-specific request type. See                     set request types list for details.
* @param parameters   Message-specific set request parameters                     serialized into a byte array.
*
* Events generated
*
* gecko_evt_mesh_generic_client_server_status -     
*
**/

static inline struct gecko_msg_mesh_generic_client_publish_rsp_t* gecko_cmd_mesh_generic_client_publish(uint16 model_id,uint16 elem_index,uint8 tid,uint32 transition,uint16 delay,uint16 flags,uint8 type,uint8 parameters_len, const uint8* parameters_data)
{
    if ((uint16_t)parameters_len > BGLIB_MSG_MAX_PAYLOAD - 15)
    {
        gecko_rsp_msg->data.rsp_mesh_generic_client_publish.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_generic_client_publish;
    }

    
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.tid=tid;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.transition=transition;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.delay=delay;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.type=type;
    gecko_cmd_msg->data.cmd_mesh_generic_client_publish.parameters.len=parameters_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_generic_client_publish.parameters.data,parameters_data,parameters_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_client_publish_id+((15+parameters_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_client_publish;
}

/** 
*
* gecko_cmd_mesh_generic_client_get_params
*
* Get the current state of a server             model or models in the
* network, with additional parameters             detailing the request.
* Besides the immediate             result code, the response or
* responses from the network             will generate server state
* report events for the replies             received.
* The server model responses will be reported in             "
* server status" events.
* This call is used to query properties, for which the
* property ID is given as a parameter.
*  
*
* @param model_id   Client model ID
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param type   Model-specific state type, identifying                   the kind of state to retrieve. See get                   state types list for details.
* @param parameters   Message-specific get request parameters                     serialized into a byte array.
*
* Events generated
*
* gecko_evt_mesh_generic_client_server_status -     
*
**/

static inline struct gecko_msg_mesh_generic_client_get_params_rsp_t* gecko_cmd_mesh_generic_client_get_params(uint16 model_id,uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 type,uint8 parameters_len, const uint8* parameters_data)
{
    if ((uint16_t)parameters_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_generic_client_get_params.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_generic_client_get_params;
    }

    
    gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.type=type;
    gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.parameters.len=parameters_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_generic_client_get_params.parameters.data,parameters_data,parameters_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_client_get_params_id+((10+parameters_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_client_get_params;
}

/** 
*
* gecko_cmd_mesh_generic_client_init
*
* Initialize generic client models 
*    
*
**/

static inline struct gecko_msg_mesh_generic_client_init_rsp_t* gecko_cmd_mesh_generic_client_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_client_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_client_init;
}

/** 
*
* gecko_cmd_mesh_generic_server_response
*
* Server response to a client request. This             command must be used when an application updates the             server model state as a response to a " client request"             event which required a response.  
*
* @param model_id   Server model ID
* @param elem_index   Server model element index
* @param client_address   Address of the client model which                     sent the message
* @param appkey_index   The application key index used.
* @param remaining   Time (in milliseconds) remaining                     before transition from current state to target                     state is complete. Set to zero if no transition is                     taking place or if transition time does not apply                     to the state change. 
* @param flags   Message flags. Bitmask of the following:                     
*  - Bit 0: Nonrelayed. If nonzero indicates a response                     to a nonrelayed request.
* 
* @param type   Model-specific state type,                     identifying the kind of state to be updated. See                     get state types list for details.
* @param parameters   Message-specific parameters,                   serialized into a byte array    
*
**/

static inline struct gecko_msg_mesh_generic_server_response_rsp_t* gecko_cmd_mesh_generic_server_response(uint16 model_id,uint16 elem_index,uint16 client_address,uint16 appkey_index,uint32 remaining,uint16 flags,uint8 type,uint8 parameters_len, const uint8* parameters_data)
{
    if ((uint16_t)parameters_len > BGLIB_MSG_MAX_PAYLOAD - 16)
    {
        gecko_rsp_msg->data.rsp_mesh_generic_server_response.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_generic_server_response;
    }

    
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.remaining=remaining;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.type=type;
    gecko_cmd_msg->data.cmd_mesh_generic_server_response.parameters.len=parameters_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_generic_server_response.parameters.data,parameters_data,parameters_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_server_response_id+((16+parameters_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_server_response;
}

/** 
*
* gecko_cmd_mesh_generic_server_update
*
* Server state update. This             command must be used when an application updates the             server model state as a response to a " client request"             event which did not require a response, but also when the             application state changes spontaneously or as a result of some             external (non-Mesh) event.  
*
* @param model_id   Server model ID
* @param elem_index   Server model element index
* @param remaining   Time (in milliseconds) remaining                     before transition from current state to target                     state is complete. Set to zero if no transition is                     taking place or if transition time does not apply                     to the state change. 
* @param type   Model-specific state type,                     identifying the kind of state to be updated. See                      get state types list for details.
* @param parameters   Message-specific parameters,                   serialized into a byte array    
*
**/

static inline struct gecko_msg_mesh_generic_server_update_rsp_t* gecko_cmd_mesh_generic_server_update(uint16 model_id,uint16 elem_index,uint32 remaining,uint8 type,uint8 parameters_len, const uint8* parameters_data)
{
    if ((uint16_t)parameters_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_generic_server_update.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_generic_server_update;
    }

    
    gecko_cmd_msg->data.cmd_mesh_generic_server_update.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_server_update.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_server_update.remaining=remaining;
    gecko_cmd_msg->data.cmd_mesh_generic_server_update.type=type;
    gecko_cmd_msg->data.cmd_mesh_generic_server_update.parameters.len=parameters_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_generic_server_update.parameters.data,parameters_data,parameters_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_server_update_id+((10+parameters_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_server_update;
}

/** 
*
* gecko_cmd_mesh_generic_server_publish
*
* Publish server state into the network using            the publish parameters configured into the            model. The message is constructed using the cached            state in the stack. 
*
* @param model_id   Server model ID
* @param elem_index   Server model element index
* @param type   Model-specific state type,                     identifying the kind of state used in the                     published message. See get state types list                     for details.    
*
**/

static inline struct gecko_msg_mesh_generic_server_publish_rsp_t* gecko_cmd_mesh_generic_server_publish(uint16 model_id,uint16 elem_index,uint8 type)
{
    
    gecko_cmd_msg->data.cmd_mesh_generic_server_publish.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_generic_server_publish.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_generic_server_publish.type=type;
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_server_publish_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_server_publish;
}

/** 
*
* gecko_cmd_mesh_generic_server_init
*
* Initialize generic server models 
*    
*
**/

static inline struct gecko_msg_mesh_generic_server_init_rsp_t* gecko_cmd_mesh_generic_server_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_generic_server_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_generic_server_init;
}

/** 
*
* gecko_cmd_coex_set_options
*
* This command is used to configure coexistence options at runtime. 
*
* @param mask   Mask defines which coexistence options are changed.
* @param options   Value of options to be changed. This parameter is used together with mask parameter.    
*
**/

static inline struct gecko_msg_coex_set_options_rsp_t* gecko_cmd_coex_set_options(uint32 mask,uint32 options)
{
    
    gecko_cmd_msg->data.cmd_coex_set_options.mask=mask;
    gecko_cmd_msg->data.cmd_coex_set_options.options=options;
    gecko_cmd_msg->header=((gecko_cmd_coex_set_options_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_coex_set_options;
}

/** 
*
* gecko_cmd_coex_get_counters
*
* This command is used to read coexistence statistic counters from the device. Response contains the list of uint32 type counter values. Counters in the list are in following order: low priority requested, high priority requested, low priority denied, high priority denied, low priority tx aborted, high priority tx aborted. 
*
* @param reset   Reset counter values    
*
**/

static inline struct gecko_msg_coex_get_counters_rsp_t* gecko_cmd_coex_get_counters(uint8 reset)
{
    
    gecko_cmd_msg->data.cmd_coex_get_counters.reset=reset;
    gecko_cmd_msg->header=((gecko_cmd_coex_get_counters_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_coex_get_counters;
}

/** 
*
* gecko_cmd_mesh_test_get_nettx
*
* Get the network transmit state of a node.            
*    
*
**/

static inline struct gecko_msg_mesh_test_get_nettx_rsp_t* gecko_cmd_mesh_test_get_nettx()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_nettx_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_nettx;
}

/** 
*
* gecko_cmd_mesh_test_set_nettx
*
* Set the network transmit state of a node locally.           Normally, the network transmit state is controlled by the Provisioner.           This command overrides any setting done by the Provisioner.            
*
* @param count   Number of network layer transmissions beyond the initial one. Range: 0-7.
* @param interval   Transmit interval steps. The interval between transmissions is a random value between 10*(1+steps) and 10*(2+steps) milliseconds; e.g. for a value of 2 the interval would be between 30 and 40 milliseconds. Range: 0-31.    
*
**/

static inline struct gecko_msg_mesh_test_set_nettx_rsp_t* gecko_cmd_mesh_test_set_nettx(uint8 count,uint8 interval)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_nettx.count=count;
    gecko_cmd_msg->data.cmd_mesh_test_set_nettx.interval=interval;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_nettx_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_nettx;
}

/** 
*
* gecko_cmd_mesh_test_get_relay
*
*  
*    
*
**/

static inline struct gecko_msg_mesh_test_get_relay_rsp_t* gecko_cmd_mesh_test_get_relay()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_relay_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_relay;
}

/** 
*
* gecko_cmd_mesh_test_set_relay
*
* Set the relay state and the relay retransmit           state of a node locally.  Normally, these states are           controlled by the Provisioner.  This command overrides any           setting done by the Provisioner.            
*
* @param enabled   Setting indicating whether the relay functionality is enabled on the node (1) or not (0); value indicating disabled (2) cannot be set. 
* @param count   Number of relay transmissions beyond the initial one. Range: 0-7.
* @param interval   Relay reransmit interval steps. The interval between transmissions is 10*(1+steps) milliseconds. Range: 0-31.    
*
**/

static inline struct gecko_msg_mesh_test_set_relay_rsp_t* gecko_cmd_mesh_test_set_relay(uint8 enabled,uint8 count,uint8 interval)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_relay.enabled=enabled;
    gecko_cmd_msg->data.cmd_mesh_test_set_relay.count=count;
    gecko_cmd_msg->data.cmd_mesh_test_set_relay.interval=interval;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_relay_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_relay;
}

/** 
*
* gecko_cmd_mesh_test_set_adv_scan_params
*
* Set non-default advertisement and scanning             parameters used in mesh communications. Note that this             command needs to be called before "node initialization" or "Provisioner initialization"             for the settings to take effect. 
*
* @param adv_interval_min   Minimum advertisement interval. Value is in units of 0.625ms. Default value is 1 (0.625ms). 
* @param adv_interval_max   Maximum advertisement interval. Value is in units of 0.625ms. Must be equal to or greater than the minimum interval. Default value is 32 (20 ms). 
* @param adv_repeat_packets   Number of times to repeat each packet on all selected advertisement channels. Range: 1-5. Default value is 1.
* @param adv_use_random_address   Bluetooth address type. Range: 0: use public address, 1: use random address. Default value: 0 (public address).
* @param adv_channel_map   Advertisement channel selection bitmask. Range: 0x1-0x7. Default value: 7 (all channels)
* @param scan_interval   Scan interval. Value is in units of 0.625ms. Range: 0x0004 to 0x4000 (time range of 2.5ms to 10.24s). Default value is 160 (100ms). 
* @param scan_window   Scan window. Value is in units of 0.625ms. Must be equal to or less than the scan interval    
*
**/

static inline struct gecko_msg_mesh_test_set_adv_scan_params_rsp_t* gecko_cmd_mesh_test_set_adv_scan_params(uint16 adv_interval_min,uint16 adv_interval_max,uint8 adv_repeat_packets,uint8 adv_use_random_address,uint8 adv_channel_map,uint16 scan_interval,uint16 scan_window)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.adv_interval_min=adv_interval_min;
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.adv_interval_max=adv_interval_max;
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.adv_repeat_packets=adv_repeat_packets;
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.adv_use_random_address=adv_use_random_address;
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.adv_channel_map=adv_channel_map;
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.scan_interval=scan_interval;
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_scan_params.scan_window=scan_window;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_adv_scan_params_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_adv_scan_params;
}

/** 
*
* gecko_cmd_mesh_test_set_ivupdate_test_mode
*
* By default IV index update has limitations to how           often the update procedure can be performed. This test command           can be called to set IV update test mode where any time limits           are ignored. 
*
* @param mode   Whether test mode is enabled (1) or disabled (0)    
*
**/

static inline struct gecko_msg_mesh_test_set_ivupdate_test_mode_rsp_t* gecko_cmd_mesh_test_set_ivupdate_test_mode(uint8 mode)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_ivupdate_test_mode.mode=mode;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_ivupdate_test_mode_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_ivupdate_test_mode;
}

/** 
*
* gecko_cmd_mesh_test_get_ivupdate_test_mode
*
* Get the current IV update test mode. See "set IV update test mode" for details.  
*    
*
**/

static inline struct gecko_msg_mesh_test_get_ivupdate_test_mode_rsp_t* gecko_cmd_mesh_test_get_ivupdate_test_mode()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_ivupdate_test_mode_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_ivupdate_test_mode;
}

/** 
*
* gecko_cmd_mesh_test_set_segment_send_delay
*
* Set delay in milliseconds between sending consecutive segments of a segmented message.             The default value is 0. Note that this             command needs to be called before "node initialization" or "Provisioner initialization"             for the settings to take effect. 
*
* @param delay   Number of milliseconds to delay each segment after the first.    
*
**/

static inline struct gecko_msg_mesh_test_set_segment_send_delay_rsp_t* gecko_cmd_mesh_test_set_segment_send_delay(uint8 delay)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_segment_send_delay.delay=delay;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_segment_send_delay_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_segment_send_delay;
}

/** 
*
* gecko_cmd_mesh_test_set_ivupdate_state
*
* Forcefully change the IV update state on the             device. Normally the state changes as a result of an IV             index update procedure progressing from one state to the             next. 
*
* @param state   Whether IV update state should be entered (1) or exited (0)    
*
**/

static inline struct gecko_msg_mesh_test_set_ivupdate_state_rsp_t* gecko_cmd_mesh_test_set_ivupdate_state(uint8 state)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_ivupdate_state.state=state;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_ivupdate_state_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_ivupdate_state;
}

/** 
*
* gecko_cmd_mesh_test_send_beacons
*
* This command can be used to send secure             network beacons for every network key on the device,             regardless of beacon configuration state or how many             beacons sent by other devices have been             observed.  
*    
*
**/

static inline struct gecko_msg_mesh_test_send_beacons_rsp_t* gecko_cmd_mesh_test_send_beacons()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_send_beacons_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_send_beacons;
}

/** 
*
* gecko_cmd_mesh_test_bind_local_model_app
*
* Bind a Model to an Appkey locally. 
*
* @param elem_index   The index of the target Element, 0 is Primary Element
* @param appkey_index   The Appkey to use for binding
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for SIG models.
* @param model_id   Model ID    
*
**/

static inline struct gecko_msg_mesh_test_bind_local_model_app_rsp_t* gecko_cmd_mesh_test_bind_local_model_app(uint16 elem_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_bind_local_model_app.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_bind_local_model_app.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_test_bind_local_model_app.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_bind_local_model_app.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_bind_local_model_app_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_bind_local_model_app;
}

/** 
*
* gecko_cmd_mesh_test_unbind_local_model_app
*
* Remove a binding between a Model and an Appkey locally. 
*
* @param elem_index   The index of the target Element, 0 is Primary Element
* @param appkey_index   The Appkey to use for binding
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for SIG models.
* @param model_id   Model ID    
*
**/

static inline struct gecko_msg_mesh_test_unbind_local_model_app_rsp_t* gecko_cmd_mesh_test_unbind_local_model_app(uint16 elem_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_unbind_local_model_app.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_unbind_local_model_app.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_test_unbind_local_model_app.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_unbind_local_model_app.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_unbind_local_model_app_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_unbind_local_model_app;
}

/** 
*
* gecko_cmd_mesh_test_add_local_model_sub
*
* Add an address to a local model's subscription list. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID
* @param sub_address   The address to add to the subscription list    
*
**/

static inline struct gecko_msg_mesh_test_add_local_model_sub_rsp_t* gecko_cmd_mesh_test_add_local_model_sub(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_add_local_model_sub_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_add_local_model_sub;
}

/** 
*
* gecko_cmd_mesh_test_del_local_model_sub
*
* Remove an address from a local Model's subscription list. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID
* @param sub_address   The address to remove from the subscription list    
*
**/

static inline struct gecko_msg_mesh_test_del_local_model_sub_rsp_t* gecko_cmd_mesh_test_del_local_model_sub(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_del_local_model_sub_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_del_local_model_sub;
}

/** 
*
* gecko_cmd_mesh_test_add_local_model_sub_va
*
* Add a virtual address to a local model's subscription list. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID
* @param sub_address   The Label UUID to add to the subscription list. The array must be exactly 16 bytes long.    
*
**/

static inline struct gecko_msg_mesh_test_add_local_model_sub_va_rsp_t* gecko_cmd_mesh_test_add_local_model_sub_va(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint8 sub_address_len, const uint8* sub_address_data)
{
    if ((uint16_t)sub_address_len > BGLIB_MSG_MAX_PAYLOAD - 7)
    {
        gecko_rsp_msg->data.rsp_mesh_test_add_local_model_sub_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_test_add_local_model_sub_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub_va.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub_va.sub_address.len=sub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_test_add_local_model_sub_va.sub_address.data,sub_address_data,sub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_add_local_model_sub_va_id+((7+sub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_add_local_model_sub_va;
}

/** 
*
* gecko_cmd_mesh_test_del_local_model_sub_va
*
* Remove a virtual address from a local model's subscription list. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID
* @param sub_address   The Label UUID to remove from the subscription list. The array must be exactly 16 bytes long.    
*
**/

static inline struct gecko_msg_mesh_test_del_local_model_sub_va_rsp_t* gecko_cmd_mesh_test_del_local_model_sub_va(uint16 elem_index,uint16 vendor_id,uint16 model_id,uint8 sub_address_len, const uint8* sub_address_data)
{
    if ((uint16_t)sub_address_len > BGLIB_MSG_MAX_PAYLOAD - 7)
    {
        gecko_rsp_msg->data.rsp_mesh_test_del_local_model_sub_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_test_del_local_model_sub_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub_va.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub_va.sub_address.len=sub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_test_del_local_model_sub_va.sub_address.data,sub_address_data,sub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_del_local_model_sub_va_id+((7+sub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_del_local_model_sub_va;
}

/** 
*
* gecko_cmd_mesh_test_get_local_model_sub
*
* Get all entries in a local model's subscription list. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID    
*
**/

static inline struct gecko_msg_mesh_test_get_local_model_sub_rsp_t* gecko_cmd_mesh_test_get_local_model_sub(uint16 elem_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_get_local_model_sub.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_get_local_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_get_local_model_sub.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_local_model_sub_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_local_model_sub;
}

/** 
*
* gecko_cmd_mesh_test_set_local_model_pub
*
* Set a local model's publication address, key, and parameters. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param appkey_index   The application key index to use for the application messages published
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID
* @param pub_address   The address to publish to
* @param ttl   Time-to-Live value for published messages
* @param period   Publication period encoded as step count and step resolution. The encoding is as follows:                   
*  - Bits 0..5: Step count
*  - Bits 6..7: Step resolution:                     
*  - 00: 100 milliseconds
*  - 01: 1 second
*  - 10: 10 seconds
*  - 11: 10 minutes
* 
* @param retrans   Retransmission count and interval;                   controls how many
* times the model re-publishes the                   same message after
* the initial publish transmission,                   and the cadence of
* retransmissions.
* Retransmission count is encoded in the three
* low bits of the value, ranging from 0 to 7. Default
* value is 0 (no retransmissions).
* Retransmission interval is encoded in the
* five high bits of the value, ranging from 0 to 31,
* in 50-millisecond units. Value of 0 corresponds to
* 50 ms, while value of 31 corresponds to 1600 ms.
* 
* @param credentials   Friendship credentials flag    
*
**/

static inline struct gecko_msg_mesh_test_set_local_model_pub_rsp_t* gecko_cmd_mesh_test_set_local_model_pub(uint16 elem_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id,uint16 pub_address,uint8 ttl,uint8 period,uint8 retrans,uint8 credentials)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.pub_address=pub_address;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.period=period;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.retrans=retrans;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub.credentials=credentials;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_local_model_pub_id+((14)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_local_model_pub;
}

/** 
*
* gecko_cmd_mesh_test_set_local_model_pub_va
*
* Set a model's publication virtual address, key, and parameters.  
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param appkey_index   The application key index to use for the published messages. 
* @param vendor_id   Vendor ID of model being configured. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID of the model being configured.
* @param ttl   Publication time-to-live value
* @param period   Publication period encoded as step count and step resolution. The encoding is as follows:                   
*  - Bits 0..5: Step count
*  - Bits 6..7: Step resolution:                     
*  - 00: 100 milliseconds
*  - 01: 1 second
*  - 10: 10 seconds
*  - 11: 10 minutes
* 
* @param retrans   Refer to the documentation of "local model publication set command" for details. Retransmission count; controls how many times the model re-publishes                   the same message after the initial publish transmission. Range: 0..7. Default value                   is 0 (no retransmissions).
* @param credentials   Friendship credentials flag
* @param pub_address   The Label UUID to publish to. The byte array must be exactly 16 bytes long.    
*
**/

static inline struct gecko_msg_mesh_test_set_local_model_pub_va_rsp_t* gecko_cmd_mesh_test_set_local_model_pub_va(uint16 elem_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id,uint8 ttl,uint8 period,uint8 retrans,uint8 credentials,uint8 pub_address_len, const uint8* pub_address_data)
{
    if ((uint16_t)pub_address_len > BGLIB_MSG_MAX_PAYLOAD - 13)
    {
        gecko_rsp_msg->data.rsp_mesh_test_set_local_model_pub_va.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_test_set_local_model_pub_va;
    }

    
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.period=period;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.retrans=retrans;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.credentials=credentials;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.pub_address.len=pub_address_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_test_set_local_model_pub_va.pub_address.data,pub_address_data,pub_address_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_local_model_pub_va_id+((13+pub_address_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_local_model_pub_va;
}

/** 
*
* gecko_cmd_mesh_test_get_local_model_pub
*
* Get a local model's publication address, key, and parameters. 
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param vendor_id   Vendor ID for vendor specific models. Use 0xffff for Bluetooth SIG models.
* @param model_id   Model ID    
*
**/

static inline struct gecko_msg_mesh_test_get_local_model_pub_rsp_t* gecko_cmd_mesh_test_get_local_model_pub(uint16 elem_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_get_local_model_pub.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_get_local_model_pub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_test_get_local_model_pub.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_local_model_pub_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_local_model_pub;
}

/** 
*
* gecko_cmd_mesh_test_set_local_heartbeat_subscription
*
* Set local node heartbeat subscription parameters.             Normally heartbeat subscription is controlled by the Provisioner.              
*
* @param subscription_source   Source address for heartbeat                     messages. Must be either a unicast address or the                     unassigned address, in which case heartbeat                     messages are not processed. 
* @param subscription_destination   Destination address for heartbeat                       messages. The address must be either the unicast                       address of the primary element of the node, a                       group address, or the unassigned address. If it                       is the unassigned address, heartbeat messages                       are not processed. 
* @param period_log   Heartbeat subscription period setting. Valid values are as follows:{br}                     
*  - 0x00: Heartbeat messages are not received
*  - 0x01 .. 0x11: Node shall receive heartbeat messages for 2^(n-1) seconds
*  - 0x12 .. 0xff: Prohibited
*     
*
**/

static inline struct gecko_msg_mesh_test_set_local_heartbeat_subscription_rsp_t* gecko_cmd_mesh_test_set_local_heartbeat_subscription(uint16 subscription_source,uint16 subscription_destination,uint8 period_log)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_subscription.subscription_source=subscription_source;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_subscription.subscription_destination=subscription_destination;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_subscription.period_log=period_log;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_local_heartbeat_subscription_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_local_heartbeat_subscription;
}

/** 
*
* gecko_cmd_mesh_test_get_local_heartbeat_subscription
*
* Get local node heartbeat subscription             state 
*    
*
**/

static inline struct gecko_msg_mesh_test_get_local_heartbeat_subscription_rsp_t* gecko_cmd_mesh_test_get_local_heartbeat_subscription()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_local_heartbeat_subscription_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_local_heartbeat_subscription;
}

/** 
*
* gecko_cmd_mesh_test_get_local_heartbeat_publication
*
* Get heartbeat publication state of a             local node. 
*    
*
**/

static inline struct gecko_msg_mesh_test_get_local_heartbeat_publication_rsp_t* gecko_cmd_mesh_test_get_local_heartbeat_publication()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_local_heartbeat_publication_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_local_heartbeat_publication;
}

/** 
*
* gecko_cmd_mesh_test_set_local_heartbeat_publication
*
* Set heartbeat publication state of a             local node. 
*
* @param publication_address   Heartbeat publication address. The                     address cannot be a virtual address. Note that it                     can be the unassigned address, in which case the                     heartbeat publishing is disabled. 
* @param count_log   Heartbeat publication count setting. Valid values are as follows:{br}                     
*  - 0x00: Heartbeat messages are not sent
*  - 0x01 .. 0x11: Node shall send 2^(n-1) heartbeat messages
*  - 0x12 .. 0xfe: Prohibited
*  - 0xff: Hearbeat messages are sent indefinitely
* 
* @param period_log   Heartbeat publication period setting. Valid values are as follows:{br}                     
*  - 0x00: Heartbeat messages are not sent
*  - 0x01 .. 0x11: Node shall send a heartbeat message every 2^(n-1) seconds
*  - 0x12 .. 0xff: Prohibited
* 
* @param ttl   Time-to-live parameter for heartbeat messages
* @param features   Heartbeat trigger setting. For bits set in the bitmask,                     reconfiguration of the node feature associated with the bit will                     result in the node emitting a heartbeat message. Valid values are as follows:                     
*  - Bit 0: Relay feature
*  - Bit 1: Proxy feature
*  - Bit 2: Friend feature
*  - Bit 3: Low power feature
* Remaining bits are reserved for future use.                     
* @param publication_netkey_index   Index of the network key used to encrypt heartbeat messages.    
*
**/

static inline struct gecko_msg_mesh_test_set_local_heartbeat_publication_rsp_t* gecko_cmd_mesh_test_set_local_heartbeat_publication(uint16 publication_address,uint8 count_log,uint8 period_log,uint8 ttl,uint16 features,uint16 publication_netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_publication.publication_address=publication_address;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_publication.count_log=count_log;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_publication.period_log=period_log;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_publication.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_publication.features=features;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_heartbeat_publication.publication_netkey_index=publication_netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_local_heartbeat_publication_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_local_heartbeat_publication;
}

/** 
*
* gecko_cmd_mesh_test_set_local_config
*
* Set a state to a value in the local configuration server model; this should be used for testing and debugging purposes only. 
*
* @param id   The State to modify
* @param netkey_index   Network key index; ignored for node-wide states
* @param value   The new value    
*
**/

static inline struct gecko_msg_mesh_test_set_local_config_rsp_t* gecko_cmd_mesh_test_set_local_config(uint16 id,uint16 netkey_index,uint8 value_len, const uint8* value_data)
{
    if ((uint16_t)value_len > BGLIB_MSG_MAX_PAYLOAD - 5)
    {
        gecko_rsp_msg->data.rsp_mesh_test_set_local_config.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_test_set_local_config;
    }

    
    gecko_cmd_msg->data.cmd_mesh_test_set_local_config.id=id;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_config.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_test_set_local_config.value.len=value_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_test_set_local_config.value.data,value_data,value_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_local_config_id+((5+value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_local_config;
}

/** 
*
* gecko_cmd_mesh_test_get_local_config
*
* Get the value of a state in the configuration server model; this should be used for testing and debugging purposes only. 
*
* @param id   The state to read
* @param netkey_index   Network key index; ignored for node-wide states    
*
**/

static inline struct gecko_msg_mesh_test_get_local_config_rsp_t* gecko_cmd_mesh_test_get_local_config(uint16 id,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_get_local_config.id=id;
    gecko_cmd_msg->data.cmd_mesh_test_get_local_config.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_local_config_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_local_config;
}

/** 
*
* gecko_cmd_mesh_test_add_local_key
*
* Add a network or application key locally. 
*
* @param key_type   0 for network key, 1 for application key.
* @param key   Key data
* @param key_index   Index for the added key (must be unused)
* @param netkey_index   Network key index to which the                     application key is bound; ignored for network                     keys    
*
**/

static inline struct gecko_msg_mesh_test_add_local_key_rsp_t* gecko_cmd_mesh_test_add_local_key(uint8 key_type,aes_key_128 key,uint16 key_index,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_add_local_key.key_type=key_type;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_key.key=key;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_key.key_index=key_index;
    gecko_cmd_msg->data.cmd_mesh_test_add_local_key.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_add_local_key_id+((21)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_add_local_key;
}

/** 
*
* gecko_cmd_mesh_test_del_local_key
*
* Delete a network or application key locally. 
*
* @param key_type   0 for network key, 1 for application key.
* @param key_index   Index of the key to delete    
*
**/

static inline struct gecko_msg_mesh_test_del_local_key_rsp_t* gecko_cmd_mesh_test_del_local_key(uint8 key_type,uint16 key_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_del_local_key.key_type=key_type;
    gecko_cmd_msg->data.cmd_mesh_test_del_local_key.key_index=key_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_del_local_key_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_del_local_key;
}

/** 
*
* gecko_cmd_mesh_test_update_local_key
*
* Update network or application key value locally.
* Copies the existing network key value to the old value             and
* replaces the current value with the given key data.
* Note that the normal way to update keys on Provisioner             as
* well as on nodes is to run the key refresh procedure.             This
* command is for debugging only.
*  
*
* @param key_type   0 for network key, 1 for application key.
* @param key   Key data
* @param key_index   Index for the key to update    
*
**/

static inline struct gecko_msg_mesh_test_update_local_key_rsp_t* gecko_cmd_mesh_test_update_local_key(uint8 key_type,aes_key_128 key,uint16 key_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_update_local_key.key_type=key_type;
    gecko_cmd_msg->data.cmd_mesh_test_update_local_key.key=key;
    gecko_cmd_msg->data.cmd_mesh_test_update_local_key.key_index=key_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_update_local_key_id+((19)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_update_local_key;
}

/** 
*
* gecko_cmd_mesh_test_set_sar_config
*
* Changes the transport layer segmentation and           reassembly configuration values. This command must be issued           before initializing the Mesh stack or the changes will not           take effect. 
*
* @param incomplete_timer_ms   Maximum timeout before a transaction               expires, regardless of other parameters. Value is in               milliseconds. Default = 10000 (10               seconds).
* @param pending_ack_base_ms   Base time to wait at the receiver before               sending a transport layer acknowledgement. Value is in               milliseconds. Default = 150.
* @param pending_ack_mul_ms   TTL multiplier to add to the base               acknowledgement timer. Value is in milliseconds. Default               = 50.
* @param wait_for_ack_base_ms   Base time to wait for an acknowledgement at               the sender before retransmission. Value is in               milliseconds. Default = 200.
* @param wait_for_ack_mul_ms   TTL multiplier to add to the base               retransmission timer. Value is in milliseconds. Default               = 50.
* @param max_send_rounds   Number of attempts to send fragments of a               segmented message, including the initial Tx. Default =               3.    
*
**/

static inline struct gecko_msg_mesh_test_set_sar_config_rsp_t* gecko_cmd_mesh_test_set_sar_config(uint32 incomplete_timer_ms,uint32 pending_ack_base_ms,uint32 pending_ack_mul_ms,uint32 wait_for_ack_base_ms,uint32 wait_for_ack_mul_ms,uint8 max_send_rounds)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_sar_config.incomplete_timer_ms=incomplete_timer_ms;
    gecko_cmd_msg->data.cmd_mesh_test_set_sar_config.pending_ack_base_ms=pending_ack_base_ms;
    gecko_cmd_msg->data.cmd_mesh_test_set_sar_config.pending_ack_mul_ms=pending_ack_mul_ms;
    gecko_cmd_msg->data.cmd_mesh_test_set_sar_config.wait_for_ack_base_ms=wait_for_ack_base_ms;
    gecko_cmd_msg->data.cmd_mesh_test_set_sar_config.wait_for_ack_mul_ms=wait_for_ack_mul_ms;
    gecko_cmd_msg->data.cmd_mesh_test_set_sar_config.max_send_rounds=max_send_rounds;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_sar_config_id+((21)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_sar_config;
}

/** 
*
* gecko_cmd_mesh_test_get_element_seqnum
*
* Get current sequence number of an element              
*
* @param elem_index   The index of the target element, 0 is the primary element    
*
**/

static inline struct gecko_msg_mesh_test_get_element_seqnum_rsp_t* gecko_cmd_mesh_test_get_element_seqnum(uint16 elem_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_get_element_seqnum.elem_index=elem_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_element_seqnum_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_element_seqnum;
}

/** 
*
* gecko_cmd_mesh_test_set_adv_bearer_state
*
* Disable/Enable adveritsement bearer for sending              
*
* @param state   0: disable advertisement, 1: enable advertisement    
*
**/

static inline struct gecko_msg_mesh_test_set_adv_bearer_state_rsp_t* gecko_cmd_mesh_test_set_adv_bearer_state(uint8 state)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_adv_bearer_state.state=state;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_adv_bearer_state_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_adv_bearer_state;
}

/** 
*
* gecko_cmd_mesh_test_get_key_count
*
* Get total number of keys in node.              
*
* @param type   0 for network key, 1 for application key.    
*
**/

static inline struct gecko_msg_mesh_test_get_key_count_rsp_t* gecko_cmd_mesh_test_get_key_count(uint8 type)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_get_key_count.type=type;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_key_count_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_key_count;
}

/** 
*
* gecko_cmd_mesh_test_get_key
*
* Get key by position. Only current key data exits in normal mode. Old key data can be queried only during key refresh.              
*
* @param type   0 for network key, 1 for application key.
* @param index   Key position, ranging from zero to key count minus one.
* @param current   1: Current key, 0: Old  key.    
*
**/

static inline struct gecko_msg_mesh_test_get_key_rsp_t* gecko_cmd_mesh_test_get_key(uint8 type,uint32 index,uint8 current)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_get_key.type=type;
    gecko_cmd_msg->data.cmd_mesh_test_get_key.index=index;
    gecko_cmd_msg->data.cmd_mesh_test_get_key.current=current;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_get_key_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_get_key;
}

/** 
*
* gecko_cmd_mesh_test_prov_get_device_key
*
* Get device key by the address of the nodes primary element              
*
* @param address   Address of the node    
*
**/

static inline struct gecko_msg_mesh_test_prov_get_device_key_rsp_t* gecko_cmd_mesh_test_prov_get_device_key(uint16 address)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_prov_get_device_key.address=address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_prov_get_device_key_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_prov_get_device_key;
}

/** 
*
* gecko_cmd_mesh_test_prov_prepare_key_refresh
*
* Prepare key refresh by feeding the new network key and all needed application             keys.  Function can be called multiple times to include more application keys. Network key must             be the same in all calls. If network key is changed the network key from 1st command is used.{br}             Sending appkey data with length zero will forget all initialization data unless this done in             the first prepare command i.e. we want to update only the network key.             Also starting the keyrefresh procedure will forget all the preparation data.              
*
* @param net_key   New net key
* @param app_keys   list of new application keys, 16-bytes each    
*
**/

static inline struct gecko_msg_mesh_test_prov_prepare_key_refresh_rsp_t* gecko_cmd_mesh_test_prov_prepare_key_refresh(aes_key_128 net_key,uint8 app_keys_len, const uint8* app_keys_data)
{
    if ((uint16_t)app_keys_len > BGLIB_MSG_MAX_PAYLOAD - 17)
    {
        gecko_rsp_msg->data.rsp_mesh_test_prov_prepare_key_refresh.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_test_prov_prepare_key_refresh;
    }

    
    gecko_cmd_msg->data.cmd_mesh_test_prov_prepare_key_refresh.net_key=net_key;
    gecko_cmd_msg->data.cmd_mesh_test_prov_prepare_key_refresh.app_keys.len=app_keys_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_test_prov_prepare_key_refresh.app_keys.data,app_keys_data,app_keys_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_prov_prepare_key_refresh_id+((17+app_keys_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_prov_prepare_key_refresh;
}

/** 
*
* gecko_cmd_mesh_test_cancel_segmented_tx
*
* Cancel sending a segmented message              
*
* @param src_addr   Source address for the segmented message
* @param dst_addr   Destination address for the segmented message    
*
**/

static inline struct gecko_msg_mesh_test_cancel_segmented_tx_rsp_t* gecko_cmd_mesh_test_cancel_segmented_tx(uint16 src_addr,uint16 dst_addr)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_cancel_segmented_tx.src_addr=src_addr;
    gecko_cmd_msg->data.cmd_mesh_test_cancel_segmented_tx.dst_addr=dst_addr;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_cancel_segmented_tx_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_cancel_segmented_tx;
}

/** 
*
* gecko_cmd_mesh_test_set_iv_index
*
* Set IV index value of the node.  
*
* @param iv_index   IV Index value to use    
*
**/

static inline struct gecko_msg_mesh_test_set_iv_index_rsp_t* gecko_cmd_mesh_test_set_iv_index(uint32 iv_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_iv_index.iv_index=iv_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_iv_index_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_iv_index;
}

/** 
*
* gecko_cmd_mesh_test_set_element_seqnum
*
* Set current sequence number of an element              
*
* @param elem_index   The index of the target element, 0 is the primary element
* @param seqnum   Sequence number to set on the target element.                     
*
**/

static inline struct gecko_msg_mesh_test_set_element_seqnum_rsp_t* gecko_cmd_mesh_test_set_element_seqnum(uint16 elem_index,uint32 seqnum)
{
    
    gecko_cmd_msg->data.cmd_mesh_test_set_element_seqnum.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_test_set_element_seqnum.seqnum=seqnum;
    gecko_cmd_msg->header=((gecko_cmd_mesh_test_set_element_seqnum_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_test_set_element_seqnum;
}

/** 
*
* gecko_cmd_mesh_lpn_init
*
* Initialize the Low Power node (LPN) mode. The node needs to be           provisioned before calling this command. Once the LPN mode is           initialized, the node cannot operate in the network without a Friend           node. In order to establish a friendship with a nearby Friend node,           the "establish           friendship" command should be used. This call has to be made before           calling the other commands in this class.          
*    
*
**/

static inline struct gecko_msg_mesh_lpn_init_rsp_t* gecko_cmd_mesh_lpn_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_init;
}

/** 
*
* gecko_cmd_mesh_lpn_deinit
*
* Deinitialize the LPN functionality. After calling this command, a           possible friendship with a Friend node is terminated and the node can           operate in the network independently. After calling this command, no           other command in this class should be called before the Low Power mode           is "initialized" again.          
*    
*
**/

static inline struct gecko_msg_mesh_lpn_deinit_rsp_t* gecko_cmd_mesh_lpn_deinit()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_deinit_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_deinit;
}

/** 
*
* gecko_cmd_mesh_lpn_configure
*
* Deprecated. Replacement is "mesh_lpn_config" command.  Configure the parameters for friendship         establishment 
*
* @param queue_length   Minimum queue length the friend must support.             This value should be chosen based on the expected message             frequency and LPN sleep period, as messages that do not fit             into the friend queue are dropped.             Note that the given value is rounded up to the nearest             power of 2.             Range: 2..128
* @param poll_timeout   Poll timeout in milliseconds. Poll timeout is the             longest time LPN will sleep in between querying its friend for             queued messages. Long poll timeout allows the LPN to sleep for             longer periods, at the expense of increased latency for receiving             messages.             Note that the given value is rounded up to the nearest 100ms             Range: 1s to 95h 59 min 59s 900 ms    
*
**/
BGLIB_DEPRECATED_API 
static inline struct gecko_msg_mesh_lpn_configure_rsp_t* gecko_cmd_mesh_lpn_configure(uint8 queue_length,uint32 poll_timeout)
{
    
    gecko_cmd_msg->data.cmd_mesh_lpn_configure.queue_length=queue_length;
    gecko_cmd_msg->data.cmd_mesh_lpn_configure.poll_timeout=poll_timeout;
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_configure_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_configure;
}

/** 
*
* gecko_cmd_mesh_lpn_establish_friendship
*
* Establish a friendship. Once a frienship has been established         the node can start saving power.  
*
* @param netkey_index   Network key index used in friendship request    
*
**/

static inline struct gecko_msg_mesh_lpn_establish_friendship_rsp_t* gecko_cmd_mesh_lpn_establish_friendship(uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_lpn_establish_friendship.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_establish_friendship_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_establish_friendship;
}

/** 
*
* gecko_cmd_mesh_lpn_poll
*
* Poll the Friend node for stored messages and security updates. This           command may be used if the application is expecting to receive           messages at a specific time. However, it is not required for correct           operation, as the procedure will be performed autonomously before the           poll timeout expires.          
*    
*
**/

static inline struct gecko_msg_mesh_lpn_poll_rsp_t* gecko_cmd_mesh_lpn_poll()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_poll_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_poll;
}

/** 
*
* gecko_cmd_mesh_lpn_terminate_friendship
*
* Terminate an already established friendship. "Friendship terminated"           event will be emitted when the friendship termination has been           completed.          
*    
*
**/

static inline struct gecko_msg_mesh_lpn_terminate_friendship_rsp_t* gecko_cmd_mesh_lpn_terminate_friendship()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_terminate_friendship_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_terminate_friendship;
}

/** 
*
* gecko_cmd_mesh_lpn_config
*
* Configure the paramaters for friendship establishment and LPN behavior.          
*
* @param setting_id   Identifies the LPN setting to be updated
* @param value   New value for the given setting    
*
**/

static inline struct gecko_msg_mesh_lpn_config_rsp_t* gecko_cmd_mesh_lpn_config(uint8 setting_id,uint32 value)
{
    
    gecko_cmd_msg->data.cmd_mesh_lpn_config.setting_id=setting_id;
    gecko_cmd_msg->data.cmd_mesh_lpn_config.value=value;
    gecko_cmd_msg->header=((gecko_cmd_mesh_lpn_config_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_lpn_config;
}

/** 
*
* gecko_cmd_mesh_friend_init
*
* Initialize the Friend mode. The node needs to be           provisioned before calling this command. Once the Friend mode is           initialized, it is ready to accept friend requests from Low Power Nodes.            This call has to be made before           calling the other commands in this class.          
*    
*
**/

static inline struct gecko_msg_mesh_friend_init_rsp_t* gecko_cmd_mesh_friend_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_friend_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_friend_init;
}

/** 
*
* gecko_cmd_mesh_friend_deinit
*
* Deinitialize the Friend functionality. After calling this command, a           possible friendship with a Low Power node is terminated and all friendsips           are terminated. After calling this command, no           other command in this class should be called before the Friend mode           is "initialized" again.          
*    
*
**/

static inline struct gecko_msg_mesh_friend_deinit_rsp_t* gecko_cmd_mesh_friend_deinit()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_friend_deinit_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_friend_deinit;
}

/** 
*
* gecko_cmd_mesh_config_client_cancel_request
*
* Cancel an ongoing request, releasing resources         allocated at the configuration client. Note that this call         does no undo any setting a node may have made if it         had received the request already.          
*
* @param handle   Request handle    
*
**/

static inline struct gecko_msg_mesh_config_client_cancel_request_rsp_t* gecko_cmd_mesh_config_client_cancel_request(uint32 handle)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_cancel_request.handle=handle;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_cancel_request_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_cancel_request;
}

/** 
*
* gecko_cmd_mesh_config_client_get_request_status
*
* Get the status of a pending request 
*
* @param handle   Request handle    
*
**/

static inline struct gecko_msg_mesh_config_client_get_request_status_rsp_t* gecko_cmd_mesh_config_client_get_request_status(uint32 handle)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_request_status.handle=handle;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_request_status_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_request_status;
}

/** 
*
* gecko_cmd_mesh_config_client_get_default_timeout
*
* Get the default timeout for configuration         client requests. If there is no response when the timeout         expires a configuration request is considered to have         failed, and an event with an error result will be generated.         Note that if the Mesh stack notices the request is destined         to an LPN by receiving an on-behalf-of acknowledgement         from a Friend node the timeout in use will be changed         to the LPN default timeout.          
*    
*
**/

static inline struct gecko_msg_mesh_config_client_get_default_timeout_rsp_t* gecko_cmd_mesh_config_client_get_default_timeout()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_default_timeout_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_default_timeout;
}

/** 
*
* gecko_cmd_mesh_config_client_set_default_timeout
*
* Set the default timeout for configuration         client requests. 
*
* @param timeout_ms   Timeout in milliseconds. Default timeout             is 5s (5000 ms)
* @param lpn_timeout_ms   Timeout in milliseconds when             communicating with an LPN node. Default LPN timeout is             120s (120000 ms)    
*
**/

static inline struct gecko_msg_mesh_config_client_set_default_timeout_rsp_t* gecko_cmd_mesh_config_client_set_default_timeout(uint32 timeout_ms,uint32 lpn_timeout_ms)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_default_timeout.timeout_ms=timeout_ms;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_default_timeout.lpn_timeout_ms=lpn_timeout_ms;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_default_timeout_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_default_timeout;
}

/** 
*
* gecko_cmd_mesh_config_client_add_netkey
*
* Add a network key to a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param netkey_index   Index of the network key to add             
*
* Events generated
*
* gecko_evt_mesh_config_client_netkey_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_add_netkey_rsp_t* gecko_cmd_mesh_config_client_add_netkey(uint16 enc_netkey_index,uint16 server_address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_add_netkey.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_netkey.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_netkey.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_add_netkey_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_add_netkey;
}

/** 
*
* gecko_cmd_mesh_config_client_remove_netkey
*
* Remove a network key from a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param netkey_index   Index of the network key to remove             
*
* Events generated
*
* gecko_evt_mesh_config_client_netkey_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_remove_netkey_rsp_t* gecko_cmd_mesh_config_client_remove_netkey(uint16 enc_netkey_index,uint16 server_address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_netkey.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_netkey.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_netkey.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_remove_netkey_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_remove_netkey;
}

/** 
*
* gecko_cmd_mesh_config_client_list_netkeys
*
* List the network keys on a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_netkey_list - 
* gecko_evt_mesh_config_client_netkey_list_end -     
*
**/

static inline struct gecko_msg_mesh_config_client_list_netkeys_rsp_t* gecko_cmd_mesh_config_client_list_netkeys(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_list_netkeys.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_netkeys.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_list_netkeys_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_list_netkeys;
}

/** 
*
* gecko_cmd_mesh_config_client_add_appkey
*
* Add an application key to a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param appkey_index   Index of the application key to add
* @param netkey_index   Index of the network key to bind the             application key to on the node; note that this may be             different from the binding on other nodes or on the             configuration client if desired.
*
* Events generated
*
* gecko_evt_mesh_config_client_appkey_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_add_appkey_rsp_t* gecko_cmd_mesh_config_client_add_appkey(uint16 enc_netkey_index,uint16 server_address,uint16 appkey_index,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_add_appkey.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_appkey.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_appkey.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_appkey.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_add_appkey_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_add_appkey;
}

/** 
*
* gecko_cmd_mesh_config_client_remove_appkey
*
* Remove an application key from a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param appkey_index   Index of the application key to remove
* @param netkey_index   Index of the network key bound to the             application key to on the node; note that this may be             different from the binding on other nodes or on the             configuration client.
*
* Events generated
*
* gecko_evt_mesh_config_client_appkey_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_remove_appkey_rsp_t* gecko_cmd_mesh_config_client_remove_appkey(uint16 enc_netkey_index,uint16 server_address,uint16 appkey_index,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_appkey.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_appkey.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_appkey.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_appkey.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_remove_appkey_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_remove_appkey;
}

/** 
*
* gecko_cmd_mesh_config_client_list_appkeys
*
* List the application keys on a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param netkey_index   Network key index for the key used as the query parameter;               the result contains the indices of the application keys               bound to this network key on the node.             
*
* Events generated
*
* gecko_evt_mesh_config_client_appkey_list - 
* gecko_evt_mesh_config_client_appkey_list_end -     
*
**/

static inline struct gecko_msg_mesh_config_client_list_appkeys_rsp_t* gecko_cmd_mesh_config_client_list_appkeys(uint16 enc_netkey_index,uint16 server_address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_list_appkeys.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_appkeys.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_appkeys.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_list_appkeys_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_list_appkeys;
}

/** 
*
* gecko_cmd_mesh_config_client_bind_model
*
* Bind an application key to a model 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param appkey_index   Index of the application key to bind to the             model
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
*
* Events generated
*
* gecko_evt_mesh_config_client_binding_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_bind_model_rsp_t* gecko_cmd_mesh_config_client_bind_model(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_bind_model.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_bind_model.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_bind_model.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_bind_model.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_bind_model.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_bind_model.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_bind_model_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_bind_model;
}

/** 
*
* gecko_cmd_mesh_config_client_unbind_model
*
* Unbind an application key from a model 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param appkey_index   Index of the application key to unbind from the             model
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
*
* Events generated
*
* gecko_evt_mesh_config_client_binding_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_unbind_model_rsp_t* gecko_cmd_mesh_config_client_unbind_model(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 appkey_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_unbind_model.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_unbind_model.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_unbind_model.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_unbind_model.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_unbind_model.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_unbind_model.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_unbind_model_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_unbind_model;
}

/** 
*
* gecko_cmd_mesh_config_client_list_bindings
*
* List application key bindings of a model 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             queried resides on the node. 
* @param vendor_id   Vendor ID for the model to query. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to query.
*
* Events generated
*
* gecko_evt_mesh_config_client_bindings_list - 
* gecko_evt_mesh_config_client_bindings_list_end -     
*
**/

static inline struct gecko_msg_mesh_config_client_list_bindings_rsp_t* gecko_cmd_mesh_config_client_list_bindings(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_list_bindings.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_bindings.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_bindings.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_bindings.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_bindings.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_list_bindings_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_list_bindings;
}

/** 
*
* gecko_cmd_mesh_config_client_get_model_pub
*
* Get model publication state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             queried resides on the node. 
* @param vendor_id   Vendor ID for the model to query. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to query.
*
* Events generated
*
* gecko_evt_mesh_config_client_model_pub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_model_pub_rsp_t* gecko_cmd_mesh_config_client_get_model_pub(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_model_pub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_model_pub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_model_pub.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_model_pub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_model_pub.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_model_pub_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_model_pub;
}

/** 
*
* gecko_cmd_mesh_config_client_set_model_pub
*
* Set model publication state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param address   The address to publish to. Can be a unicast address, a virtual address, or a group address; can also be the unassigned address to stop the model from publishing. 
* @param appkey_index   The application key index to use for the published messages. 
* @param credentials   Friendship credential flag. If zero,             publication is done using normal credentials; if             one, it is done with friendship credentials, meaning             only the friend can decrypt the published message             and relay it forward using the normal credentials.             The default value is 0.
* @param ttl   Publication time-to-live value
* @param period_ms   Publication period in milliseconds. Note that             the resolution of the publicaton period is limited by the             specification to 100ms up to a period of 6.3s, 1s up to             a period of 63s, 10s up to a period of 630s, and 10             minutes above that. Maximum period allowed is 630 minutes.             
* @param retransmit_count   Publication retransmission count.             Valid values range from 0 to 7.
* @param retransmit_interval_ms   Publication retransmission interval in             millisecond units. The range of value is 50 to 1600 ms,             and the resolution of the value is 50 milliseconds.             
*
* Events generated
*
* gecko_evt_mesh_config_client_model_pub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_model_pub_rsp_t* gecko_cmd_mesh_config_client_set_model_pub(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uint16 address,uint16 appkey_index,uint8 credentials,uint8 ttl,uint32 period_ms,uint8 retransmit_count,uint16 retransmit_interval_ms)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.address=address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.credentials=credentials;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.period_ms=period_ms;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.retransmit_count=retransmit_count;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub.retransmit_interval_ms=retransmit_interval_ms;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_model_pub_id+((22)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_model_pub;
}

/** 
*
* gecko_cmd_mesh_config_client_set_model_pub_va
*
* Set model publication state, with a full         virtual publication address 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param address   The Label UUID (full virtual address) to publish to.             
* @param appkey_index   The application key index to use for the published messages. 
* @param credentials   Friendship credential flag. If zero,             publication is done using normal credentials; if             one, it is done with friendship credentials, meaning             only the friend can decrypt the published message             and relay it forward using the normal credentials.             The default value is 0.
* @param ttl   Publication time-to-live value
* @param period_ms   Publication period in milliseconds. Note that             the resolution of the publicaton period is limited by the             specification to 100ms up to a period of 6.3s, 1s up to             a period of 63s, 10s up to a period of 630s, and 10             minutes above that. Maximum period allowed is 630 minutes.             
* @param retransmit_count   Publication retransmission count.             Valid values range from 0 to 7.
* @param retransmit_interval_ms   Publication retransmission interval in             millisecond units. The range of value is 50 to 1600 ms,             and the resolution of the value is 50 milliseconds.             
*
* Events generated
*
* gecko_evt_mesh_config_client_model_pub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_model_pub_va_rsp_t* gecko_cmd_mesh_config_client_set_model_pub_va(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uuid_128 address,uint16 appkey_index,uint8 credentials,uint8 ttl,uint32 period_ms,uint8 retransmit_count,uint16 retransmit_interval_ms)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.address=address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.credentials=credentials;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.period_ms=period_ms;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.retransmit_count=retransmit_count;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_pub_va.retransmit_interval_ms=retransmit_interval_ms;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_model_pub_va_id+((36)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_model_pub_va;
}

/** 
*
* gecko_cmd_mesh_config_client_add_model_sub
*
* Add an address to model subscription list 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param sub_address   The address to add to the subscription list. Note that the address has to be a group address. 
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_add_model_sub_rsp_t* gecko_cmd_mesh_config_client_add_model_sub(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_add_model_sub_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_add_model_sub;
}

/** 
*
* gecko_cmd_mesh_config_client_add_model_sub_va
*
* Add a Label UUID (full virtual address) to model         subscription list 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param sub_address   The full virtual address to add to the subscription list. 
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_add_model_sub_va_rsp_t* gecko_cmd_mesh_config_client_add_model_sub_va(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uuid_128 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub_va.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub_va.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub_va.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_add_model_sub_va.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_add_model_sub_va_id+((25)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_add_model_sub_va;
}

/** 
*
* gecko_cmd_mesh_config_client_remove_model_sub
*
* Remove an address from model subscription list 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param sub_address   The address to remove from the subscription list.
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_remove_model_sub_rsp_t* gecko_cmd_mesh_config_client_remove_model_sub(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_remove_model_sub_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_remove_model_sub;
}

/** 
*
* gecko_cmd_mesh_config_client_remove_model_sub_va
*
* Remove a Label UUID (full virtual address) from model         subscription list 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param sub_address   The full virtual address to remove from the subscription list.
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_remove_model_sub_va_rsp_t* gecko_cmd_mesh_config_client_remove_model_sub_va(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uuid_128 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub_va.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub_va.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub_va.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_remove_model_sub_va.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_remove_model_sub_va_id+((25)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_remove_model_sub_va;
}

/** 
*
* gecko_cmd_mesh_config_client_set_model_sub
*
* Set (overwrite) model subscription address list to a single address 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param sub_address   The address to set as the subscription list.
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_model_sub_rsp_t* gecko_cmd_mesh_config_client_set_model_sub(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uint16 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_model_sub_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_model_sub;
}

/** 
*
* gecko_cmd_mesh_config_client_set_model_sub_va
*
* Set (overwrite) model subscription address list to a single virtual address 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
* @param sub_address   The full virtual address to set as the subscription list.
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_model_sub_va_rsp_t* gecko_cmd_mesh_config_client_set_model_sub_va(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id,uuid_128 sub_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub_va.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub_va.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub_va.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub_va.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub_va.model_id=model_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_model_sub_va.sub_address=sub_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_model_sub_va_id+((25)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_model_sub_va;
}

/** 
*
* gecko_cmd_mesh_config_client_clear_model_sub
*
* Clear (empty) the model subscription address list 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             configured resides on the node. 
* @param vendor_id   Vendor ID for the model to configure. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to configure.
*
* Events generated
*
* gecko_evt_mesh_config_client_model_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_clear_model_sub_rsp_t* gecko_cmd_mesh_config_client_clear_model_sub(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_clear_model_sub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_clear_model_sub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_clear_model_sub.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_clear_model_sub.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_clear_model_sub.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_clear_model_sub_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_clear_model_sub;
}

/** 
*
* gecko_cmd_mesh_config_client_list_subs
*
* Get the subscription address list of a model 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param element_index   Index of the element where the model to be             queried resides on the node. 
* @param vendor_id   Vendor ID for the model to query. Use 0xFFFF for             Bluetooth SIG models.
* @param model_id   Model ID for the model to query.
*
* Events generated
*
* gecko_evt_mesh_config_client_subs_list - 
* gecko_evt_mesh_config_client_subs_list_end -     
*
**/

static inline struct gecko_msg_mesh_config_client_list_subs_rsp_t* gecko_cmd_mesh_config_client_list_subs(uint16 enc_netkey_index,uint16 server_address,uint8 element_index,uint16 vendor_id,uint16 model_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_list_subs.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_subs.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_subs.element_index=element_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_subs.vendor_id=vendor_id;
    gecko_cmd_msg->data.cmd_mesh_config_client_list_subs.model_id=model_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_list_subs_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_list_subs;
}

/** 
*
* gecko_cmd_mesh_config_client_get_heartbeat_pub
*
* Get the heartbeat publication state of a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_heartbeat_pub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_heartbeat_pub_rsp_t* gecko_cmd_mesh_config_client_get_heartbeat_pub(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_heartbeat_pub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_heartbeat_pub.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_heartbeat_pub_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_heartbeat_pub;
}

/** 
*
* gecko_cmd_mesh_config_client_set_heartbeat_pub
*
* Set the heartbeat publication state of a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param destination_address   Heartbeat publication destination address. The address             cannot be a virtual address. Note that it can be the             unassigned address, in which case the heartbeat publishing             is disabled.
* @param netkey_index   Index of the network key used to encrypt             heartbeat messages.
* @param count_log   Heartbeat publication count logarithm-of-2 setting. Valid values are as follows:{br}             
*  - 0x00: Heartbeat messages are not sent
*  - 0x01 .. 0x11: Node shall send 2^(n-1) heartbeat messages
*  - 0x12 .. 0xfe: Prohibited
*  - 0xff: Hearbeat messages are sent indefinitely
* 
* @param period_log   Heartbeat publication period logarithm-of-2 setting. Valid values are as follows:{br}             
*  - 0x00: Heartbeat messages are not sent
*  - 0x01 .. 0x11: Node shall send a heartbeat message every 2^(n-1) seconds
*  - 0x12 .. 0xff: Prohibited
* 
* @param ttl   Time-to-live value for heartbeat messages
* @param features   Heartbeat trigger setting. For bits set in the bitmask,             reconfiguration of the node feature associated with the bit will             result in the node emitting a heartbeat message. Valid values are as follows:             
*  - Bit 0: Relay feature
*  - Bit 1: Proxy feature
*  - Bit 2: Friend feature
*  - Bit 3: Low power feature
* Remaining bits are reserved for future use.             
*
* Events generated
*
* gecko_evt_mesh_config_client_heartbeat_pub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_heartbeat_pub_rsp_t* gecko_cmd_mesh_config_client_set_heartbeat_pub(uint16 enc_netkey_index,uint16 server_address,uint16 destination_address,uint16 netkey_index,uint8 count_log,uint8 period_log,uint8 ttl,uint16 features)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.destination_address=destination_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.count_log=count_log;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.period_log=period_log;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.ttl=ttl;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_pub.features=features;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_heartbeat_pub_id+((13)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_heartbeat_pub;
}

/** 
*
* gecko_cmd_mesh_config_client_get_heartbeat_sub
*
* Get the heartbeat subscription state of a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_heartbeat_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_heartbeat_sub_rsp_t* gecko_cmd_mesh_config_client_get_heartbeat_sub(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_heartbeat_sub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_heartbeat_sub.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_heartbeat_sub_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_heartbeat_sub;
}

/** 
*
* gecko_cmd_mesh_config_client_set_heartbeat_sub
*
* Set the heartbeat subscription state of a node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param source_address   Source address for heartbeat messages. Must             be either a unicast address or the unassigned address, in             which case heartbeat messages are not processed. 
* @param destination_address   Destination address for heartbeat messages. The             address must be either the unicast address of the primary element             of the node, a group address, or the unassigned address. If it             is the unassigned address, heartbeat messages are not processed. 
* @param period_log   Heartbeat subscription period logarithm-of-2 setting. Valid values are as follows:{br}             
*  - 0x00: Heartbeat messages are not received
*  - 0x01 .. 0x11: Node shall receive heartbeat messages for 2^(n-1) seconds
*  - 0x12 .. 0xff: Prohibited
* 
*
* Events generated
*
* gecko_evt_mesh_config_client_heartbeat_sub_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_heartbeat_sub_rsp_t* gecko_cmd_mesh_config_client_set_heartbeat_sub(uint16 enc_netkey_index,uint16 server_address,uint16 source_address,uint16 destination_address,uint8 period_log)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_sub.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_sub.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_sub.source_address=source_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_sub.destination_address=destination_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_heartbeat_sub.period_log=period_log;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_heartbeat_sub_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_heartbeat_sub;
}

/** 
*
* gecko_cmd_mesh_config_client_get_beacon
*
* Get node secure network beacon state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_beacon_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_beacon_rsp_t* gecko_cmd_mesh_config_client_get_beacon(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_beacon.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_beacon.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_beacon_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_beacon;
}

/** 
*
* gecko_cmd_mesh_config_client_set_beacon
*
* Set node secure network beacon state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param value   Secure network beacon value to set. Valid values are:             
*  - 0: Node is not broadcasting secure network beacons
*  - 1: Node is broadcasting secure network beacons
* 
*
* Events generated
*
* gecko_evt_mesh_config_client_beacon_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_beacon_rsp_t* gecko_cmd_mesh_config_client_set_beacon(uint16 enc_netkey_index,uint16 server_address,uint8 value)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_beacon.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_beacon.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_beacon.value=value;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_beacon_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_beacon;
}

/** 
*
* gecko_cmd_mesh_config_client_get_default_ttl
*
* Get node default TTL state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_default_ttl_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_default_ttl_rsp_t* gecko_cmd_mesh_config_client_get_default_ttl(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_default_ttl.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_default_ttl.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_default_ttl_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_default_ttl;
}

/** 
*
* gecko_cmd_mesh_config_client_set_default_ttl
*
* Set node default TTL state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param value   Default TTL value. Valid value range is from             2 to 127 for relayed PDUs, and 0 to indicate nonrelayed             PDUs
*
* Events generated
*
* gecko_evt_mesh_config_client_default_ttl_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_default_ttl_rsp_t* gecko_cmd_mesh_config_client_set_default_ttl(uint16 enc_netkey_index,uint16 server_address,uint8 value)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_default_ttl.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_default_ttl.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_default_ttl.value=value;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_default_ttl_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_default_ttl;
}

/** 
*
* gecko_cmd_mesh_config_client_get_gatt_proxy
*
* Get node GATT proxy state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_gatt_proxy_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_gatt_proxy_rsp_t* gecko_cmd_mesh_config_client_get_gatt_proxy(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_gatt_proxy.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_gatt_proxy.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_gatt_proxy_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_gatt_proxy;
}

/** 
*
* gecko_cmd_mesh_config_client_set_gatt_proxy
*
* Set node GATT proxy state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param value   GATT proxy value to set. Valid values are:             
*  - 0: Proxy feature is disabled
*  - 1: Proxy feature is enabled
* 
*
* Events generated
*
* gecko_evt_mesh_config_client_gatt_proxy_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_gatt_proxy_rsp_t* gecko_cmd_mesh_config_client_set_gatt_proxy(uint16 enc_netkey_index,uint16 server_address,uint8 value)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_gatt_proxy.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_gatt_proxy.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_gatt_proxy.value=value;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_gatt_proxy_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_gatt_proxy;
}

/** 
*
* gecko_cmd_mesh_config_client_get_relay
*
* Get node relay state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_relay_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_relay_rsp_t* gecko_cmd_mesh_config_client_get_relay(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_relay.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_relay.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_relay_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_relay;
}

/** 
*
* gecko_cmd_mesh_config_client_set_relay
*
* Set node relay state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param value   Relay value to set. Valid values are:             
*  - 0: Relay feature is disabled
*  - 1: Relay feature is enabled
* 
* @param retransmit_count   Relay retransmit count. Valid values range from 0 to 7;             default value is 0 (no retransmissions).
* @param retransmit_interval_ms   Relay retransmit interval in             milliseconds. Valid values range from 10 ms to 320             ms, with a resolution of 10 ms. Value is ignored             if retransmission count is set to zero.
*
* Events generated
*
* gecko_evt_mesh_config_client_relay_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_relay_rsp_t* gecko_cmd_mesh_config_client_set_relay(uint16 enc_netkey_index,uint16 server_address,uint8 value,uint8 retransmit_count,uint16 retransmit_interval_ms)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_relay.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_relay.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_relay.value=value;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_relay.retransmit_count=retransmit_count;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_relay.retransmit_interval_ms=retransmit_interval_ms;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_relay_id+((8)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_relay;
}

/** 
*
* gecko_cmd_mesh_config_client_get_network_transmit
*
* Get node network transmit state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_network_transmit_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_network_transmit_rsp_t* gecko_cmd_mesh_config_client_get_network_transmit(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_network_transmit.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_network_transmit.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_network_transmit_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_network_transmit;
}

/** 
*
* gecko_cmd_mesh_config_client_set_network_transmit
*
* Set node network transmit state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param transmit_count   Network transmit count. Valid values range             from 1 to 8; default value is 1 (single transmission; no             retransmissions).
* @param transmit_interval_ms   Network transmit interval in             milliseconds. Valid values range from 10 ms to 320             ms, with a resolution of 10 ms. Value is ignored             if transmission count is set to one.
*
* Events generated
*
* gecko_evt_mesh_config_client_network_transmit_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_network_transmit_rsp_t* gecko_cmd_mesh_config_client_set_network_transmit(uint16 enc_netkey_index,uint16 server_address,uint8 transmit_count,uint16 transmit_interval_ms)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_network_transmit.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_network_transmit.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_network_transmit.transmit_count=transmit_count;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_network_transmit.transmit_interval_ms=transmit_interval_ms;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_network_transmit_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_network_transmit;
}

/** 
*
* gecko_cmd_mesh_config_client_get_identity
*
* Get node identity state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param netkey_index   Network key index for which the state is queried
*
* Events generated
*
* gecko_evt_mesh_config_client_identity_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_identity_rsp_t* gecko_cmd_mesh_config_client_get_identity(uint16 enc_netkey_index,uint16 server_address,uint16 netkey_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_identity.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_identity.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_identity.netkey_index=netkey_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_identity_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_identity;
}

/** 
*
* gecko_cmd_mesh_config_client_set_identity
*
* Set node identity state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param netkey_index   Network key index for which the state is configured
* @param value   Identity value to set. Valid values are:             
*  - 0: Node identity advertising is disabled
*  - 1: Node identity advertising is enabled
* 
*
* Events generated
*
* gecko_evt_mesh_config_client_identity_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_identity_rsp_t* gecko_cmd_mesh_config_client_set_identity(uint16 enc_netkey_index,uint16 server_address,uint16 netkey_index,uint8 value)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_identity.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_identity.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_identity.netkey_index=netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_identity.value=value;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_identity_id+((7)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_identity;
}

/** 
*
* gecko_cmd_mesh_config_client_get_friend
*
* Get node friend state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_friend_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_friend_rsp_t* gecko_cmd_mesh_config_client_get_friend(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_friend.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_friend.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_friend_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_friend;
}

/** 
*
* gecko_cmd_mesh_config_client_set_friend
*
* Set node friend state 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param value   Friend value to set. Valid values are:             
*  - 0: Friend feature is not enabled
*  - 1: Friend feature is enabled
* 
*
* Events generated
*
* gecko_evt_mesh_config_client_friend_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_set_friend_rsp_t* gecko_cmd_mesh_config_client_set_friend(uint16 enc_netkey_index,uint16 server_address,uint8 value)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_set_friend.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_friend.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_set_friend.value=value;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_set_friend_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_set_friend;
}

/** 
*
* gecko_cmd_mesh_config_client_get_lpn_polltimeout
*
* Get LPN poll timeout from a Friend node 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param lpn_address   LPN address             
*
* Events generated
*
* gecko_evt_mesh_config_client_lpn_polltimeout_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_lpn_polltimeout_rsp_t* gecko_cmd_mesh_config_client_get_lpn_polltimeout(uint16 enc_netkey_index,uint16 server_address,uint16 lpn_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_lpn_polltimeout.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_lpn_polltimeout.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_lpn_polltimeout.lpn_address=lpn_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_lpn_polltimeout_id+((6)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_lpn_polltimeout;
}

/** 
*
* gecko_cmd_mesh_config_client_get_dcd
*
* Get composition data of a device 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
* @param page   Composition data page to query
*
* Events generated
*
* gecko_evt_mesh_config_client_dcd_data - 
* gecko_evt_mesh_config_client_dcd_data_end -     
*
**/

static inline struct gecko_msg_mesh_config_client_get_dcd_rsp_t* gecko_cmd_mesh_config_client_get_dcd(uint16 enc_netkey_index,uint16 server_address,uint8 page)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_get_dcd.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_dcd.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_config_client_get_dcd.page=page;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_get_dcd_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_get_dcd;
}

/** 
*
* gecko_cmd_mesh_config_client_reset_node
*
* Request a node to unprovision itself. To be used when         a node is removed from network 
*
* @param enc_netkey_index   Network key used to encrypt the request             on the network layer
* @param server_address   Destination node primary element address             
*
* Events generated
*
* gecko_evt_mesh_config_client_reset_status -     
*
**/

static inline struct gecko_msg_mesh_config_client_reset_node_rsp_t* gecko_cmd_mesh_config_client_reset_node(uint16 enc_netkey_index,uint16 server_address)
{
    
    gecko_cmd_msg->data.cmd_mesh_config_client_reset_node.enc_netkey_index=enc_netkey_index;
    gecko_cmd_msg->data.cmd_mesh_config_client_reset_node.server_address=server_address;
    gecko_cmd_msg->header=((gecko_cmd_mesh_config_client_reset_node_id+((4)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_config_client_reset_node;
}

/** 
*
* gecko_cmd_l2cap_coc_send_connection_request
*
* This command is used for sending LE credit based connection request.               
*
* @param connection   Handle of the LE connection to be used for opening connection-oriented channel.
* @param le_psm   LE Protocol/Service Multiplexer - LE_PSM
* @param mtu   The maximum size of payload data that the application on the device
* sending the request is capable of accepting, i.e. the MTU corresponds
* to the maximum SDU size.
* Range:  23 to 65533.
* Application needs to handle segmentation and reassembly from PDU to
* SDU.
* 
* @param mps   The maximum size of payload data that the L2CAP layer on the device
* sending the request is capable of accepting, i.e. the MPS corresponds
* to the maximum PDU payload size.
* Range:  23 to 250.
* That is the maximum size of data that the application is able to send
* using "l2cap_coc_send_data" command or receive by "l2cap_coc_data"
* event.
* 
* @param initial_credit   The initial credit value indicates the number of PDUs that the peer device can send.
*
* Events generated
*
* gecko_evt_l2cap_coc_connection_response - Triggered when a LE credit based connection connection response has been received in response to this command.
* gecko_evt_l2cap_coc_channel_disconnected - Triggered when a LE credit based connection connection response has not been received within the 30 seconds timeout in response to this command.    
*
**/

static inline struct gecko_msg_l2cap_coc_send_connection_request_rsp_t* gecko_cmd_l2cap_coc_send_connection_request(uint8 connection,uint16 le_psm,uint16 mtu,uint16 mps,uint16 initial_credit)
{
    
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_request.connection=connection;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_request.le_psm=le_psm;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_request.mtu=mtu;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_request.mps=mps;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_request.initial_credit=initial_credit;
    gecko_cmd_msg->header=((gecko_cmd_l2cap_coc_send_connection_request_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_l2cap_coc_send_connection_request;
}

/** 
*
* gecko_cmd_l2cap_coc_send_connection_response
*
* This command is used for sending LE credit based connection response.               
*
* @param connection   Handle of the LE connection to be used for opening connection-oriented channel.
* @param cid   The CID represents the destination channel endpoint of the device sending the response which is same as source CID field of corresponding request message.
* @param mtu   The maximum size of payload data that the application on the device
* sending the response is capable of accepting, i.e. the MTU corresponds
* to the maximum SDU size.
* Range:  23 to 65533.
* Application needs to handle segmentation and reassembly from PDU to
* SDU.
* 
* @param mps   The maximum size of payload data that the L2CAP layer on the device
* sending the response is capable of accepting, i.e. the MPS corresponds
* to the maximum PDU payload size.
* Range:  23 to 250.
* That is the maximum size of data that the application is able to send
* using "l2cap_coc_send_data" command or receive by "l2cap_coc_data"
* event.
* 
* @param initial_credit   The initial credit value indicates the number of PDUs that the peer device can send.
* @param result   The result field indicates the outcome of the connection request.    
*
**/

static inline struct gecko_msg_l2cap_coc_send_connection_response_rsp_t* gecko_cmd_l2cap_coc_send_connection_response(uint8 connection,uint16 cid,uint16 mtu,uint16 mps,uint16 initial_credit,uint16 result)
{
    
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_response.connection=connection;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_response.cid=cid;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_response.mtu=mtu;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_response.mps=mps;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_response.initial_credit=initial_credit;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_connection_response.result=result;
    gecko_cmd_msg->header=((gecko_cmd_l2cap_coc_send_connection_response_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_l2cap_coc_send_connection_response;
}

/** 
*
* gecko_cmd_l2cap_coc_send_le_flow_control_credit
*
* This command is used for sending LE flow control credit indicating that the channel endpoint on local device is capable of receiving more data.               
*
* @param connection   Handle of the LE connection to be used for sending flow control credit.
* @param cid   The CID represents the destination channel endpoint of the device sending the flow control credit.
* @param credits   The credit value indicates the additional number of PDUs that the peer device can send.    
*
**/

static inline struct gecko_msg_l2cap_coc_send_le_flow_control_credit_rsp_t* gecko_cmd_l2cap_coc_send_le_flow_control_credit(uint8 connection,uint16 cid,uint16 credits)
{
    
    gecko_cmd_msg->data.cmd_l2cap_coc_send_le_flow_control_credit.connection=connection;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_le_flow_control_credit.cid=cid;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_le_flow_control_credit.credits=credits;
    gecko_cmd_msg->header=((gecko_cmd_l2cap_coc_send_le_flow_control_credit_id+((5)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_l2cap_coc_send_le_flow_control_credit;
}

/** 
*
* gecko_cmd_l2cap_coc_send_disconnection_request
*
* This command is used for sending L2CAP connection-oriented channel disconnection request.              
*
* @param connection   Handle of the LE connection to be used for terminating connection-oriented channel.
* @param cid   The CID represents the destination channel endpoint of the device sending the disconnection request.
*
* Events generated
*
* gecko_evt_l2cap_coc_channel_disconnected - Triggered when a L2CAP channel is disconnected in response to this command.    
*
**/

static inline struct gecko_msg_l2cap_coc_send_disconnection_request_rsp_t* gecko_cmd_l2cap_coc_send_disconnection_request(uint8 connection,uint16 cid)
{
    
    gecko_cmd_msg->data.cmd_l2cap_coc_send_disconnection_request.connection=connection;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_disconnection_request.cid=cid;
    gecko_cmd_msg->header=((gecko_cmd_l2cap_coc_send_disconnection_request_id+((3)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_l2cap_coc_send_disconnection_request;
}

/** 
*
* gecko_cmd_l2cap_coc_send_data
*
* This command is used for sending data to a L2CAP LE connection-oriented channel.               
*
* @param connection   Handle of the LE connection to be used for sending data.
* @param cid   The CID represents the destination channel endpoint of the device sending the data.
* @param data   The data to be sent. The data length must be within the range of destination channel endpoint's MPS value.    
*
**/

static inline struct gecko_msg_l2cap_coc_send_data_rsp_t* gecko_cmd_l2cap_coc_send_data(uint8 connection,uint16 cid,uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_l2cap_coc_send_data.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_l2cap_coc_send_data;
    }

    
    gecko_cmd_msg->data.cmd_l2cap_coc_send_data.connection=connection;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_data.cid=cid;
    gecko_cmd_msg->data.cmd_l2cap_coc_send_data.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_l2cap_coc_send_data.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_l2cap_coc_send_data_id+((4+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_l2cap_coc_send_data;
}

/** 
*
* gecko_cmd_cte_transmitter_enable_cte_response
*
* This command can be used to enable CTE responses on a connection. 
*
* @param connection   Connection handle
* @param cte_types   CTE types. Bitmask of followings,                 
*  - Bit 0: AoA CTE response
*  - Bit 1: AoD CTE response with 1us slots
*  - Bit 2: AoD CTE response with 2us slots
* 
* @param switching_pattern   Antenna switching pattern    
*
**/

static inline struct gecko_msg_cte_transmitter_enable_cte_response_rsp_t* gecko_cmd_cte_transmitter_enable_cte_response(uint8 connection,uint8 cte_types,uint8 switching_pattern_len, const uint8* switching_pattern_data)
{
    if ((uint16_t)switching_pattern_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_cte_transmitter_enable_cte_response.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_cte_transmitter_enable_cte_response;
    }

    
    gecko_cmd_msg->data.cmd_cte_transmitter_enable_cte_response.connection=connection;
    gecko_cmd_msg->data.cmd_cte_transmitter_enable_cte_response.cte_types=cte_types;
    gecko_cmd_msg->data.cmd_cte_transmitter_enable_cte_response.switching_pattern.len=switching_pattern_len;
    memcpy(gecko_cmd_msg->data.cmd_cte_transmitter_enable_cte_response.switching_pattern.data,switching_pattern_data,switching_pattern_len);
    gecko_cmd_msg->header=((gecko_cmd_cte_transmitter_enable_cte_response_id+((3+switching_pattern_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_transmitter_enable_cte_response;
}

/** 
*
* gecko_cmd_cte_transmitter_disable_cte_response
*
* This command can be used to disable CTE responses on a connection. 
*
* @param connection   Connection handle    
*
**/

static inline struct gecko_msg_cte_transmitter_disable_cte_response_rsp_t* gecko_cmd_cte_transmitter_disable_cte_response(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_cte_transmitter_disable_cte_response.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_cte_transmitter_disable_cte_response_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_transmitter_disable_cte_response;
}

/** 
*
* gecko_cmd_cte_transmitter_start_connectionless_cte
*
* This command can be used to start connectionless CTE transmit. 
*
* @param adv   Periodic advertising handle
* @param cte_length   CTE length in 8 us units
* @param cte_type   Requested CTE type                     
*  - 0: AoA CTE
*  - 1: AoD CTE with 1 us slots
*  - 2: AoD CTE with 2 us slots
* 
* @param cte_count   The number of CTEs to transmit in each periodic advertising interval
* @param switching_pattern   Antenna switching pattern    
*
**/

static inline struct gecko_msg_cte_transmitter_start_connectionless_cte_rsp_t* gecko_cmd_cte_transmitter_start_connectionless_cte(uint8 adv,uint8 cte_length,uint8 cte_type,uint8 cte_count,uint8 switching_pattern_len, const uint8* switching_pattern_data)
{
    if ((uint16_t)switching_pattern_len > BGLIB_MSG_MAX_PAYLOAD - 5)
    {
        gecko_rsp_msg->data.rsp_cte_transmitter_start_connectionless_cte.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_cte_transmitter_start_connectionless_cte;
    }

    
    gecko_cmd_msg->data.cmd_cte_transmitter_start_connectionless_cte.adv=adv;
    gecko_cmd_msg->data.cmd_cte_transmitter_start_connectionless_cte.cte_length=cte_length;
    gecko_cmd_msg->data.cmd_cte_transmitter_start_connectionless_cte.cte_type=cte_type;
    gecko_cmd_msg->data.cmd_cte_transmitter_start_connectionless_cte.cte_count=cte_count;
    gecko_cmd_msg->data.cmd_cte_transmitter_start_connectionless_cte.switching_pattern.len=switching_pattern_len;
    memcpy(gecko_cmd_msg->data.cmd_cte_transmitter_start_connectionless_cte.switching_pattern.data,switching_pattern_data,switching_pattern_len);
    gecko_cmd_msg->header=((gecko_cmd_cte_transmitter_start_connectionless_cte_id+((5+switching_pattern_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_transmitter_start_connectionless_cte;
}

/** 
*
* gecko_cmd_cte_transmitter_stop_connectionless_cte
*
* This command can be used to stop connectionless CTE transmit. 
*
* @param adv   Periodic advertising handle    
*
**/

static inline struct gecko_msg_cte_transmitter_stop_connectionless_cte_rsp_t* gecko_cmd_cte_transmitter_stop_connectionless_cte(uint8 adv)
{
    
    gecko_cmd_msg->data.cmd_cte_transmitter_stop_connectionless_cte.adv=adv;
    gecko_cmd_msg->header=((gecko_cmd_cte_transmitter_stop_connectionless_cte_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_transmitter_stop_connectionless_cte;
}

/** 
*
* gecko_cmd_cte_transmitter_set_dtm_parameters
*
* This command can be used to set CTE related parameters of LE transmitter test. 
*
* @param cte_length   Expected length of the Constant Tone Extension in 8 us units
* @param cte_type   CTE type                     
*  - 0: AoA CTE
*  - 1: AoD CTE with 1 us slots
*  - 2: AoD CTE with 2 us slots
* 
* @param switching_pattern   Antenna switching pattern    
*
**/

static inline struct gecko_msg_cte_transmitter_set_dtm_parameters_rsp_t* gecko_cmd_cte_transmitter_set_dtm_parameters(uint8 cte_length,uint8 cte_type,uint8 switching_pattern_len, const uint8* switching_pattern_data)
{
    if ((uint16_t)switching_pattern_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_cte_transmitter_set_dtm_parameters.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_cte_transmitter_set_dtm_parameters;
    }

    
    gecko_cmd_msg->data.cmd_cte_transmitter_set_dtm_parameters.cte_length=cte_length;
    gecko_cmd_msg->data.cmd_cte_transmitter_set_dtm_parameters.cte_type=cte_type;
    gecko_cmd_msg->data.cmd_cte_transmitter_set_dtm_parameters.switching_pattern.len=switching_pattern_len;
    memcpy(gecko_cmd_msg->data.cmd_cte_transmitter_set_dtm_parameters.switching_pattern.data,switching_pattern_data,switching_pattern_len);
    gecko_cmd_msg->header=((gecko_cmd_cte_transmitter_set_dtm_parameters_id+((3+switching_pattern_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_transmitter_set_dtm_parameters;
}

/** 
*
* gecko_cmd_cte_transmitter_clear_dtm_parameters
*
* This command can be used to clear CTE related parameters that were previously set for LE transmitter test.  Default values will be restored for these parameters. 
*    
*
**/

static inline struct gecko_msg_cte_transmitter_clear_dtm_parameters_rsp_t* gecko_cmd_cte_transmitter_clear_dtm_parameters()
{
    
    gecko_cmd_msg->header=((gecko_cmd_cte_transmitter_clear_dtm_parameters_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_transmitter_clear_dtm_parameters;
}

/** 
*
* gecko_cmd_cte_receiver_configure
*
* This commmand can be used to configure the CTE sampling mode. 
*
* @param flags   Values: 
*  - 0: Disable raw sample mode
*  - 1: Enable raw sample mode
*     
*
**/

static inline struct gecko_msg_cte_receiver_configure_rsp_t* gecko_cmd_cte_receiver_configure(uint8 flags)
{
    
    gecko_cmd_msg->data.cmd_cte_receiver_configure.flags=flags;
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_configure_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_configure;
}

/** 
*
* gecko_cmd_cte_receiver_start_iq_sampling
*
* This command can be used to start IQ samplings on a connection. 
*
* @param connection   Connection handle
* @param interval   Measurement interval                 
*  - 0: No interval. The request is initiated only once.
*  - Other values N: Initiate the request every N-th connection events
* 
* @param cte_length   Minimum length of the CTE being requested in 8 us units
* @param cte_type   Requested CTE type                     
*  - 0: AoA CTE
*  - 1: AoD CTE with 1 us slots
*  - 2: AoD CTE with 2 us slots
* 
* @param slot_durations   Slot durations                     
*  - 1: Switching and sampling slots are 1 us each
*  - 2: Switching and sampling slots are 2 us each
* 
* @param switching_pattern   Antenna switching pattern
*
* Events generated
*
* gecko_evt_cte_receiver_iq_report - Triggered when IQ samples have been received.    
*
**/

static inline struct gecko_msg_cte_receiver_start_iq_sampling_rsp_t* gecko_cmd_cte_receiver_start_iq_sampling(uint8 connection,uint16 interval,uint8 cte_length,uint8 cte_type,uint8 slot_durations,uint8 switching_pattern_len, const uint8* switching_pattern_data)
{
    if ((uint16_t)switching_pattern_len > BGLIB_MSG_MAX_PAYLOAD - 7)
    {
        gecko_rsp_msg->data.rsp_cte_receiver_start_iq_sampling.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_cte_receiver_start_iq_sampling;
    }

    
    gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.connection=connection;
    gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.interval=interval;
    gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.cte_length=cte_length;
    gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.cte_type=cte_type;
    gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.slot_durations=slot_durations;
    gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.switching_pattern.len=switching_pattern_len;
    memcpy(gecko_cmd_msg->data.cmd_cte_receiver_start_iq_sampling.switching_pattern.data,switching_pattern_data,switching_pattern_len);
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_start_iq_sampling_id+((7+switching_pattern_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_start_iq_sampling;
}

/** 
*
* gecko_cmd_cte_receiver_stop_iq_sampling
*
* This command can be used to stop the IQ sampling on a connection. 
*
* @param connection   Connection handle    
*
**/

static inline struct gecko_msg_cte_receiver_stop_iq_sampling_rsp_t* gecko_cmd_cte_receiver_stop_iq_sampling(uint8 connection)
{
    
    gecko_cmd_msg->data.cmd_cte_receiver_stop_iq_sampling.connection=connection;
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_stop_iq_sampling_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_stop_iq_sampling;
}

/** 
*
* gecko_cmd_cte_receiver_start_connectionless_iq_sampling
*
* This command can be used to start IQ sampling on a periodic advertising synchronization. 
*
* @param sync   Periodic advertising synchronization handle
* @param slot_durations   Slot durations                     
*  - 1: Switching and sampling slots are 1 us each
*  - 2: Switching and sampling slots are 2 us each
* 
* @param cte_count    - 0: Sample and report all available CTEs
*  - Other values: Maximum number of sampled CTEs in each periodic advertising interval
* 
* @param switching_pattern   Antenna switching pattern
*
* Events generated
*
* gecko_evt_cte_receiver_iq_report - Triggered when IQ samples have been received.    
*
**/

static inline struct gecko_msg_cte_receiver_start_connectionless_iq_sampling_rsp_t* gecko_cmd_cte_receiver_start_connectionless_iq_sampling(uint8 sync,uint8 slot_durations,uint8 cte_count,uint8 switching_pattern_len, const uint8* switching_pattern_data)
{
    if ((uint16_t)switching_pattern_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_cte_receiver_start_connectionless_iq_sampling.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_cte_receiver_start_connectionless_iq_sampling;
    }

    
    gecko_cmd_msg->data.cmd_cte_receiver_start_connectionless_iq_sampling.sync=sync;
    gecko_cmd_msg->data.cmd_cte_receiver_start_connectionless_iq_sampling.slot_durations=slot_durations;
    gecko_cmd_msg->data.cmd_cte_receiver_start_connectionless_iq_sampling.cte_count=cte_count;
    gecko_cmd_msg->data.cmd_cte_receiver_start_connectionless_iq_sampling.switching_pattern.len=switching_pattern_len;
    memcpy(gecko_cmd_msg->data.cmd_cte_receiver_start_connectionless_iq_sampling.switching_pattern.data,switching_pattern_data,switching_pattern_len);
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_start_connectionless_iq_sampling_id+((4+switching_pattern_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_start_connectionless_iq_sampling;
}

/** 
*
* gecko_cmd_cte_receiver_stop_connectionless_iq_sampling
*
* This command can be used to stop IQ sampling on a periodic advertising synchronization. 
*
* @param sync   Periodic advertising synchronization handle    
*
**/

static inline struct gecko_msg_cte_receiver_stop_connectionless_iq_sampling_rsp_t* gecko_cmd_cte_receiver_stop_connectionless_iq_sampling(uint8 sync)
{
    
    gecko_cmd_msg->data.cmd_cte_receiver_stop_connectionless_iq_sampling.sync=sync;
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_stop_connectionless_iq_sampling_id+((1)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_stop_connectionless_iq_sampling;
}

/** 
*
* gecko_cmd_cte_receiver_set_dtm_parameters
*
* This command can be used to set CTE related parameters of LE receiver test. 
*
* @param cte_length   Expected length of the Constant Tone Extension in 8 us units
* @param cte_type   Expected CTE type                     
*  - 0: Expect AoA CTE
*  - 1: Expect AoD CTE with 1 us slots
*  - 2: Expect AoD CTE with 2 us slots
* 
* @param slot_durations   Slot durations                     
*  - 1: Switching and sampling slots are 1 us each
*  - 2: Switching and sampling slots are 2 us each
* 
* @param switching_pattern   Antenna switching pattern    
*
**/

static inline struct gecko_msg_cte_receiver_set_dtm_parameters_rsp_t* gecko_cmd_cte_receiver_set_dtm_parameters(uint8 cte_length,uint8 cte_type,uint8 slot_durations,uint8 switching_pattern_len, const uint8* switching_pattern_data)
{
    if ((uint16_t)switching_pattern_len > BGLIB_MSG_MAX_PAYLOAD - 4)
    {
        gecko_rsp_msg->data.rsp_cte_receiver_set_dtm_parameters.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_cte_receiver_set_dtm_parameters;
    }

    
    gecko_cmd_msg->data.cmd_cte_receiver_set_dtm_parameters.cte_length=cte_length;
    gecko_cmd_msg->data.cmd_cte_receiver_set_dtm_parameters.cte_type=cte_type;
    gecko_cmd_msg->data.cmd_cte_receiver_set_dtm_parameters.slot_durations=slot_durations;
    gecko_cmd_msg->data.cmd_cte_receiver_set_dtm_parameters.switching_pattern.len=switching_pattern_len;
    memcpy(gecko_cmd_msg->data.cmd_cte_receiver_set_dtm_parameters.switching_pattern.data,switching_pattern_data,switching_pattern_len);
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_set_dtm_parameters_id+((4+switching_pattern_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_set_dtm_parameters;
}

/** 
*
* gecko_cmd_cte_receiver_clear_dtm_parameters
*
* This command can be used to clear CTE related parameters that were previously set for LE receiver test. Default values will be restored for these parameters. 
*    
*
**/

static inline struct gecko_msg_cte_receiver_clear_dtm_parameters_rsp_t* gecko_cmd_cte_receiver_clear_dtm_parameters()
{
    
    gecko_cmd_msg->header=((gecko_cmd_cte_receiver_clear_dtm_parameters_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_cte_receiver_clear_dtm_parameters;
}

/** 
*
* gecko_cmd_mesh_sensor_server_init
*
* Initializes the Sensor Server model with Sensor Descriptors present at the element.         The descriptors are cached and Descriptor Get queries are served without propagating it to the application.         All incoming client queries are checked against the cached data, only valid requests related to existing sensors are         propagated to the the application.          
*
* @param elem_index   Server model element index
* @param descriptors   Sensor Descriptor State structures submitted as a byte array
* A sensor descriptor represents the attributes describing the sensor
* data. It does not change throughout the lifetime of the element.
* {br}{br}               The following fields are required: {br}               
*  - Sensor Property ID:        16 bits
*  - Sensor Positive Tolerance: 12 bits
*  - Sensor Negative Tolerance: 12 bits
*  - Sensor Sampling Function:   8 bits
*  - Sensor Measurement Period:  8 bits
*  - Sensor Update Interval:     8 bits
* Sensor Property ID is a 2-octet value referencing a device property that describes the meaning and the format of the data reported by the sensor.               The value 0x0000 is prohibited, the valid range is 0x0001 to 0xFFFF.
*     
*
**/

static inline struct gecko_msg_mesh_sensor_server_init_rsp_t* gecko_cmd_mesh_sensor_server_init(uint16 elem_index,uint8 descriptors_len, const uint8* descriptors_data)
{
    if ((uint16_t)descriptors_len > BGLIB_MSG_MAX_PAYLOAD - 3)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_server_init.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_server_init;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_server_init.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_init.descriptors.len=descriptors_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_server_init.descriptors.data,descriptors_data,descriptors_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_server_init_id+((3+descriptors_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_server_init;
}

/** 
*
* gecko_cmd_mesh_sensor_server_deinit
*
* Deinitializes the Sensor Server functionality.{br}          Please note that heap reserved space cannot be freed or reallocated, reinitializing with greater number of sensors than before will eventually return out of memory error until the device is reset.          
*
* @param elem_index   Server model element index    
*
**/

static inline struct gecko_msg_mesh_sensor_server_deinit_rsp_t* gecko_cmd_mesh_sensor_server_deinit(uint16 elem_index)
{
    
    gecko_cmd_msg->data.cmd_mesh_sensor_server_deinit.elem_index=elem_index;
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_server_deinit_id+((2)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_server_deinit;
}

/** 
*
* gecko_cmd_mesh_sensor_server_send_descriptor_status
*
* Send a Descriptor Status message either as a reply to a Get Descriptor client request.          
*
* @param elem_index   Server model element index
* @param client_address   Destination client address.
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param descriptors   Serialized Sensor Descriptor states for all sensors within the element consisting one or more 8 bytes structures as follows:               
*  - Sensor Property ID:        16 bits
*  - Sensor Positive Tolerance: 12 bits
*  - Sensor Negative Tolerance: 12 bits
*  - Sensor Sampling Function:   8 bits
*  - Sensor Measurement Period:  8 bits
*     
*
**/

static inline struct gecko_msg_mesh_sensor_server_send_descriptor_status_rsp_t* gecko_cmd_mesh_sensor_server_send_descriptor_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint8 descriptors_len, const uint8* descriptors_data)
{
    if ((uint16_t)descriptors_len > BGLIB_MSG_MAX_PAYLOAD - 8)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_server_send_descriptor_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_descriptor_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_descriptor_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_descriptor_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_descriptor_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_descriptor_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_descriptor_status.descriptors.len=descriptors_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_server_send_descriptor_status.descriptors.data,descriptors_data,descriptors_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_server_send_descriptor_status_id+((8+descriptors_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_descriptor_status;
}

/** 
*
* gecko_cmd_mesh_sensor_server_send_status
*
* Send Sensor Status message as a reply to a Get client request or as an unsolicited message          
*
* @param elem_index   Setup Server model element index
* @param client_address   Destination client address             The address 0x0000 can be used to publish the message according to model configuration
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param sensor_data   Serialized Sensor Data consisting of one or more Sensor state for each sensor within the element.               In order to simplify processing, the byte array shall be in TLV format:               
*  - 1st Property ID: 16 bits
*  - Value Length: 8 bits
*  - Value:        variable 
*  - 2nd Property ID: 16 bits
*  - Value Length: 8 bits
*  - Value:        variable 
*  - ...
* If sensor data was requested for a Property ID that does not exist within the element, the reply shall contain the given Property ID with zero length.                 
*
**/

static inline struct gecko_msg_mesh_sensor_server_send_status_rsp_t* gecko_cmd_mesh_sensor_server_send_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint8 sensor_data_len, const uint8* sensor_data_data)
{
    if ((uint16_t)sensor_data_len > BGLIB_MSG_MAX_PAYLOAD - 8)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_server_send_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_status.sensor_data.len=sensor_data_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_server_send_status.sensor_data.data,sensor_data_data,sensor_data_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_server_send_status_id+((8+sensor_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_status;
}

/** 
*
* gecko_cmd_mesh_sensor_server_send_column_status
*
* Send Column Status message as a response to a Column Get client request or as an unsolicited message          
*
* @param elem_index   Client model element index
* @param client_address   Destination client address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param sensor_data   Byte array containing the serialized Sensor Series Column state in the following format:               
*  - Sensor Raw Value X, variable length raw value representing the left corner of a column
*  - Sensor Column Width, variable length raw value representing the width of the column
*  - Sensor Raw Value Y, variable length raw value representing the height of the column
* If the Property ID or the column ID (Raw value X) does not exist, the reply shall contain only these two fields,               omitting the optional Column Width and Raw Value Y fields.                 
*
**/

static inline struct gecko_msg_mesh_sensor_server_send_column_status_rsp_t* gecko_cmd_mesh_sensor_server_send_column_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 sensor_data_len, const uint8* sensor_data_data)
{
    if ((uint16_t)sensor_data_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_server_send_column_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_column_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.sensor_data.len=sensor_data_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_server_send_column_status.sensor_data.data,sensor_data_data,sensor_data_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_server_send_column_status_id+((10+sensor_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_column_status;
}

/** 
*
* gecko_cmd_mesh_sensor_server_send_series_status
*
* Send Series Status message as a response to a Series Get client request or as an unsolicited message          
*
* @param elem_index   Client model element index
* @param client_address   Destination client address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param sensor_data   Byte array containing the serialized sequence of Sensor Serier Column states in the following format:               
*  - 1st Sensor Raw Value X, variable length raw value representing the left corner of a column
*  - 1st Sensor Column Width, variable length raw value representing the width of the column
*  - 1st Sensor Raw Value Y, variable length raw value representing the height og the column
*  - ...
*  - Nth Sensor Raw Value X, variable length raw value representing the left corner of a column
*  - Nth Sensor Column Width, variable length raw value representing the width of the column
*  - Nth Sensor Raw Value Y, variable length raw value representing the height og the column
* If Property ID does not exist in the element, the reply shall contain only the given Property ID,               omitting the other optional fields to column identifiers and column values.                 
*
**/

static inline struct gecko_msg_mesh_sensor_server_send_series_status_rsp_t* gecko_cmd_mesh_sensor_server_send_series_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 sensor_data_len, const uint8* sensor_data_data)
{
    if ((uint16_t)sensor_data_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_server_send_series_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_series_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.sensor_data.len=sensor_data_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_server_send_series_status.sensor_data.data,sensor_data_data,sensor_data_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_server_send_series_status_id+((10+sensor_data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_server_send_series_status;
}

/** 
*
* gecko_cmd_mesh_sensor_setup_server_send_cadence_status
*
* Replies to a Get/Set Cadence client request with a Cadence Status message.           Only Cadence Set (acknowledged) must be answered by sending the status message to the client.           In addition, configuration changes shall be published according to model publishing configuration.          
*
* @param elem_index   Client model element index
* @param client_address   Destination client address               The address 0x0000 can be used to publish the message according model configuration instead of a direct reply.
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param params   Optional byte array containing the serialized Sensor Cadence state, excluding the property ID.               If not empty, the state consists of the following fields:               
*  - Fast Cadence Period Divisor, 7 bits
*  - Status Trigger type, 1 bits (0 = discrete value, 1 = percentage)
*  - Status Trigger Delta Down, variable length
*  - Status Trigger Delta Up, variable length
*  - Status Min Interval, 8 bits, representing a power of 2 milliseconds. Valid range is 0-26 
*  - Fast Cadence Low, variable length, lower bound for the fast cadence range
*  - Low Cadence Low, variable length, higher bound for the fast cadence range
*     
*
**/

static inline struct gecko_msg_mesh_sensor_setup_server_send_cadence_status_rsp_t* gecko_cmd_mesh_sensor_setup_server_send_cadence_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 params_len, const uint8* params_data)
{
    if ((uint16_t)params_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_cadence_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_cadence_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.params.len=params_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_cadence_status.params.data,params_data,params_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_setup_server_send_cadence_status_id+((10+params_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_cadence_status;
}

/** 
*
* gecko_cmd_mesh_sensor_setup_server_send_settings_status
*
* Replies to a Get Settings client request with a Settings Status message          
*
* @param elem_index   Client model element index
* @param client_address   Destination client model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param setting_ids   Array of 16-bit Setting Property IDs of the settings the given sensor has                  
*
**/

static inline struct gecko_msg_mesh_sensor_setup_server_send_settings_status_rsp_t* gecko_cmd_mesh_sensor_setup_server_send_settings_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 setting_ids_len, const uint8* setting_ids_data)
{
    if ((uint16_t)setting_ids_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_settings_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_settings_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.setting_ids.len=setting_ids_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_settings_status.setting_ids.data,setting_ids_data,setting_ids_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_setup_server_send_settings_status_id+((10+setting_ids_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_settings_status;
}

/** 
*
* gecko_cmd_mesh_sensor_setup_server_send_setting_status
*
* Replies to a Get/Set Setting client request with a Setting Status message.           Only Set Setting (acknowledged) request must be answered by sending a reply to the unicast address of the sender.           In addition, configuration changes shall be published if model publishing is set up.          
*
* @param elem_index   Client model element index
* @param client_address   Destination client model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param setting_id   Sensor Setting Property ID field identifying the device property of a setting.               Range: 0x0001 - 0xffff, 0x0000 is prohibited.             
* @param raw_value   Sensor Setting raw value.                Size and representation depends on the type defined by the Sensor Setting Property ID.                 
*
**/

static inline struct gecko_msg_mesh_sensor_setup_server_send_setting_status_rsp_t* gecko_cmd_mesh_sensor_setup_server_send_setting_status(uint16 elem_index,uint16 client_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint16 setting_id,uint8 raw_value_len, const uint8* raw_value_data)
{
    if ((uint16_t)raw_value_len > BGLIB_MSG_MAX_PAYLOAD - 12)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_setting_status.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_setting_status;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.client_address=client_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.setting_id=setting_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.raw_value.len=raw_value_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_setup_server_send_setting_status.raw_value.data,raw_value_data,raw_value_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_setup_server_send_setting_status_id+((12+raw_value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_setup_server_send_setting_status;
}

/** 
*
* gecko_cmd_mesh_sensor_client_init
*
* Initializes the Sensor Client model.         Sensor Client does not have any internal configuration, it only activates the model in the mesh stack. 
*    
*
**/

static inline struct gecko_msg_mesh_sensor_client_init_rsp_t* gecko_cmd_mesh_sensor_client_init()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_init_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_init;
}

/** 
*
* gecko_cmd_mesh_sensor_client_deinit
*
* Deinitializes the Sensor Client model.          There are no sensor specific configuratons to reset. Under normal circumstances, models are initialized at boot and never deinitialized.          
*    
*
**/

static inline struct gecko_msg_mesh_sensor_client_deinit_rsp_t* gecko_cmd_mesh_sensor_client_deinit()
{
    
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_deinit_id+((0)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_deinit;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get_descriptor
*
* Get the Sensor Descriptor state of one specific, or all sensors within an model.         Results in a Sensor Descriptor Status event 
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   ProperyID for the sensor (optional). Range: 0x0001 - 0xffff for a specific device property ID or 0x0000 to get all (the value 0x0000 is prohibited as a real ID)                 
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_descriptor_rsp_t* gecko_cmd_mesh_sensor_client_get_descriptor(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_descriptor.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_descriptor.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_descriptor.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_descriptor.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_descriptor.property_id=property_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_descriptor_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_descriptor;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get
*
* Sends a Sensor Get message to fetch the Sensor Data state of one specific sensor given by its Property ID, results in a Sensor Status event.           The Property ID 0x0000 can be used to fetch all sensor values present at a server element.          
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, or 0x0000 when not used to get values for all sensors present in the element.                 
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_rsp_t* gecko_cmd_mesh_sensor_client_get(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get.property_id=property_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get_column
*
* Get a Sensor Series Column state, results in a Sensor Column Status event 
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param column_id   Raw value identifying a columns    
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_column_rsp_t* gecko_cmd_mesh_sensor_client_get_column(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 column_id_len, const uint8* column_id_data)
{
    if ((uint16_t)column_id_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_client_get_column.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_column;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.column_id.len=column_id_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_client_get_column.column_id.data,column_id_data,column_id_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_column_id+((10+column_id_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_column;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get_series
*
* Get a Sensor Series Column state, results in a Sensor Series Status event 
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param column_ids   Raw values identifying starting and ending columns    
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_series_rsp_t* gecko_cmd_mesh_sensor_client_get_series(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 column_ids_len, const uint8* column_ids_data)
{
    if ((uint16_t)column_ids_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_client_get_series.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_series;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.column_ids.len=column_ids_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_client_get_series.column_ids.data,column_ids_data,column_ids_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_series_id+((10+column_ids_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_series;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get_cadence
*
* Sends a Sensor Get Cadence message to get the Sensor Cadence state, results in a Sensor Cadence Status message.          
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.                 
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_cadence_rsp_t* gecko_cmd_mesh_sensor_client_get_cadence(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_cadence.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_cadence.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_cadence.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_cadence.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_cadence.property_id=property_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_cadence_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_cadence;
}

/** 
*
* gecko_cmd_mesh_sensor_client_set_cadence
*
* Sends a Sensor Cadence Set message, either acknowledged or unacknowledged, depending on the message flags.           Acknowledged message results in a Cadence Status reply message and event, the server shall publish its new state in any case.          
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   Bit 1 (0x02) defines whether response is required.{br}              If set to 1, SET CADENCE message will be sent, zero will send SET CADENCE UNACKNOWLEDGED
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param params   Byte array containing serialized fields of Sensor Cadence state, excluding the property ID               
*  - Fast Cadence Period Divisor, 7 bits
*  - Status Trigger type, 1 bit (0 = discrete value, 1 = percentage)
*  - Status Trigger Delta Down, variable length
*  - Status Trigger Delta Up, variable length
*  - Status Min Interval, 8 bits, representing a power of 2 milliseconds. Valid range is 0-26 
*  - Fast Cadence Low, variable length, lower bound for the fast cadence range
*  - Low Cadence Low, variable length, higher bound for the fast cadence range
*     
*
**/

static inline struct gecko_msg_mesh_sensor_client_set_cadence_rsp_t* gecko_cmd_mesh_sensor_client_set_cadence(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint8 params_len, const uint8* params_data)
{
    if ((uint16_t)params_len > BGLIB_MSG_MAX_PAYLOAD - 10)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_client_set_cadence.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_client_set_cadence;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.params.len=params_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_client_set_cadence.params.data,params_data,params_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_set_cadence_id+((10+params_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_set_cadence;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get_settings
*
* Sends a Sensor Settings Get message to fetch the Sensor Property IDs present for the given sensor, results in a Sensor Settings Status event.          
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.                 
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_settings_rsp_t* gecko_cmd_mesh_sensor_client_get_settings(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_settings.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_settings.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_settings.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_settings.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_settings.property_id=property_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_settings_id+((9)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_settings;
}

/** 
*
* gecko_cmd_mesh_sensor_client_get_setting
*
* Sends a Sensor Get Setting message to get the value of a specific setting for the given sensor, results in a Sensor Setting Status event.          
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   No flags defined currently
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param setting_id   Sensor Setting Property ID field identifying the device property of a setting.               Range: 0x0001 - 0xffff, 0x0000 is prohibited.                 
*
**/

static inline struct gecko_msg_mesh_sensor_client_get_setting_rsp_t* gecko_cmd_mesh_sensor_client_get_setting(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint16 setting_id)
{
    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_setting.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_setting.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_setting.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_setting.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_setting.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_get_setting.setting_id=setting_id;
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_get_setting_id+((11)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_get_setting;
}

/** 
*
* gecko_cmd_mesh_sensor_client_set_setting
*
* Sends Sensor Setting Set message to update the value of a specific setting for the given sensor,           either acknowledged or unacknowledged depeding on the message flags.           Only acknowledged requests will have a direct Sensor Setting Status reply, the server shall publish its new state in any case.          
*
* @param elem_index   Client model element index
* @param server_address   Destination server model address
* @param appkey_index   The application key index to use.
* @param flags   Bit 1 (0x02) defines whether response is required.{br}             If set to 1, SET SETTING message is sent, zero will use SET SETTING UNACKNOWLEDGED.             
* @param property_id   Property ID for the sensor. Range: 0x0001 - 0x0ffff for a specific device property, the value 0x0000 is prohibited.             
* @param setting_id   Sensor Setting Property ID field identifying the device property of a setting.               Range: 0x0001 - 0xffff, 0x0000 is prohibited.             
* @param raw_value   Sensor Setting raw value.                Size and representation depends on the type defined by the Sensor Setting Property ID.                 
*
**/

static inline struct gecko_msg_mesh_sensor_client_set_setting_rsp_t* gecko_cmd_mesh_sensor_client_set_setting(uint16 elem_index,uint16 server_address,uint16 appkey_index,uint8 flags,uint16 property_id,uint16 setting_id,uint8 raw_value_len, const uint8* raw_value_data)
{
    if ((uint16_t)raw_value_len > BGLIB_MSG_MAX_PAYLOAD - 12)
    {
        gecko_rsp_msg->data.rsp_mesh_sensor_client_set_setting.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_mesh_sensor_client_set_setting;
    }

    
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.elem_index=elem_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.server_address=server_address;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.appkey_index=appkey_index;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.flags=flags;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.property_id=property_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.setting_id=setting_id;
    gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.raw_value.len=raw_value_len;
    memcpy(gecko_cmd_msg->data.cmd_mesh_sensor_client_set_setting.raw_value.data,raw_value_data,raw_value_len);
    gecko_cmd_msg->header=((gecko_cmd_mesh_sensor_client_set_setting_id+((12+raw_value_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_mesh_sensor_client_set_setting;
}

/** 
*
* gecko_cmd_user_message_to_target
*
* This command can be used by an NCP host to send a message to the target application on device. 
*
* @param data   The message    
*
**/

static inline struct gecko_msg_user_message_to_target_rsp_t* gecko_cmd_user_message_to_target(uint8 data_len, const uint8* data_data)
{
    if ((uint16_t)data_len > BGLIB_MSG_MAX_PAYLOAD - 1)
    {
        gecko_rsp_msg->data.rsp_user_message_to_target.result = bg_err_command_too_long;
        return &gecko_rsp_msg->data.rsp_user_message_to_target;
    }

    
    gecko_cmd_msg->data.cmd_user_message_to_target.data.len=data_len;
    memcpy(gecko_cmd_msg->data.cmd_user_message_to_target.data.data,data_data,data_len);
    gecko_cmd_msg->header=((gecko_cmd_user_message_to_target_id+((1+data_len)<<8)));
    
    gecko_handle_command(gecko_cmd_msg->header,&gecko_cmd_msg->data.payload);
    
    return &gecko_rsp_msg->data.rsp_user_message_to_target;
}
#ifdef __cplusplus
}
#endif

#endif
