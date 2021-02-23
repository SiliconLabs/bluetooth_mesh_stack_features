/***************************************************************************//**
 * @file
 * @brief Self provisioning code
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdio.h>
#include "native_gecko.h"
#include "selfprov.h"
#include "mesh_app_memory_config.h"

#ifdef ENABLE_LOGGING
#define log(...) printf(__VA_ARGS__)
#else
#define log(...)
#endif

typedef struct _dcd_element
{
    uint16_t location;
    uint8_t  num_sig_models;
    uint8_t  num_vendor_models;
    uint8_t  payload[1];
} dcd_element_t;

typedef struct _sig_model
{
    uint16_t model_id;
} sig_model_t;

typedef struct _vendor_model
{
    uint16_t model_id;
    uint16_t vendor_id;
} vendor_model_t;

extern const size_t __mesh_dcd_len;
extern const uint8_t *__mesh_dcd_ptr;

static uint16_t get_unicast_address(void)
{
    uint16_t addr = UNICAST_ADDRESS;

    if (addr == 0 || addr > 0x7FFF) {
        struct gecko_msg_system_get_bt_address_rsp_t * get_address_rsp;
        get_address_rsp = gecko_cmd_system_get_bt_address();
        bd_addr * btaddr = &get_address_rsp->address;
        log("  bt address %02x:%02x:%02x:%02x:%02x:%02x\r\n", btaddr->addr[5], btaddr->addr[4], btaddr->addr[3], btaddr->addr[2], btaddr->addr[1], btaddr->addr[0]);

        addr = *(uint16_t *)&btaddr->addr[0] ^
               *(uint16_t *)&btaddr->addr[2] ^
               *(uint16_t *)&btaddr->addr[4];
        addr &= (0xFFFF<<UADDR_ELEM_BITS);
        addr &= (0xFFFF>>(UADDR_RSVD_BITS+1));
    }

    return addr;
}

static dcd_element_t * dcd_get_element(uint16_t elem_index)
{
    dcd_element_t * p_elem = NULL;
    uint16_t index;
    size_t offset = 10;     //dcd header

    if (elem_index < MESH_CFG_MAX_ELEMENTS) {
        for (index = 0; index <= elem_index; ++index) {
            if (offset + 4 > __mesh_dcd_len) {  //element header
                p_elem = NULL;
                break;
            }
            p_elem = (dcd_element_t *)(__mesh_dcd_ptr + offset);
            offset += (4 + p_elem->num_sig_models * 2 + p_elem->num_vendor_models * 4);
        }
    }

    return p_elem;
}

static int16_t dcd_get_sig_models(uint16_t elem_index, sig_model_t ** pp_sig_models)
{
    dcd_element_t * p_elem;

    p_elem = dcd_get_element(elem_index);
    if (NULL == p_elem) {
        if (pp_sig_models) {
            *pp_sig_models = NULL;
        }
        return -1;
    }

    if (pp_sig_models) {
        *pp_sig_models = (sig_model_t *)p_elem->payload;
    }

    return (int16_t)p_elem->num_sig_models;
}

static int16_t dcd_get_vendor_models(uint16_t elem_index, vendor_model_t ** pp_vendor_models)
{
    dcd_element_t * p_elem;

    p_elem = dcd_get_element(elem_index);
    if (NULL == p_elem) {
        if (pp_vendor_models) {
            *pp_vendor_models = NULL;
        }
        return -1;
    }

    if (pp_vendor_models) {
        *pp_vendor_models = (vendor_model_t *)(p_elem->payload + p_elem->num_sig_models * 2);
    }

    return (int16_t)p_elem->num_vendor_models;
}

static uint8_t grp_get_server_models(const sig_model_t * p_sig_models, uint8_t num_sig_models, sig_model_t * p_server_models, uint8_t max_server_models)
{
    const uint16_t SERVER_MODELS[] = PUB_SUB_SERVER_MODELS;
    const uint8_t NUM_SERVER_MODELS = (uint8_t)(sizeof(SERVER_MODELS)/sizeof(uint16_t));
    uint8_t i, j, num_models = 0;

    if (p_sig_models && p_server_models) {
        for (i = 0; i < num_sig_models && i < max_server_models; ++i) {
            for (j = 0; j < NUM_SERVER_MODELS; ++j) {
                if (SERVER_MODELS[j] == p_sig_models[i].model_id) {
                    p_server_models[num_models++].model_id = SERVER_MODELS[j];
                    break;
                }
            }
        }
    }

    return num_models;
}

static uint8_t grp_get_client_models(const sig_model_t * p_sig_models, uint8_t num_sig_models, sig_model_t * p_client_models, uint8_t max_client_models)
{
    const uint16_t CLIENT_MODELS[] = PUB_SUB_CLIENT_MODELS;
    const uint8_t NUM_CLIENT_MODELS = (uint8_t)(sizeof(CLIENT_MODELS)/sizeof(uint16_t));
    uint8_t i, j, num_models = 0;

    if (p_sig_models && p_client_models) {
        for (i = 0; i < num_sig_models && i < max_client_models; ++i) {
            for (j = 0; j < NUM_CLIENT_MODELS; ++j) {
                if (CLIENT_MODELS[j] == p_sig_models[i].model_id) {
                    p_client_models[num_models++].model_id = CLIENT_MODELS[j];
                    break;
                }
            }
        }
    }

    return num_models;
}

void self_provisioning(void)
{
    aes_key_128 netkey = {NETWORK_KEY};
    aes_key_128 appkey = {APPLICATION_KEY};
    aes_key_128 devkey = {DEVICE_KEY};

    log(">self provisioning ..\r\n");

    struct gecko_msg_mesh_test_get_key_count_rsp_t *key_count_rsp;

    // Network key
    key_count_rsp = gecko_cmd_mesh_test_get_key_count(0);
    if (key_count_rsp->result) {
        log(">failed to get netkey count (res = %d)\r\n", key_count_rsp->result);
        return;
    }

    if (0 == key_count_rsp->count) {
        // 1. Set provisioning data
        uint16_t addr = get_unicast_address();
        struct gecko_msg_mesh_node_set_provisioning_data_rsp_t *set_prov_rsp;
        set_prov_rsp = gecko_cmd_mesh_node_set_provisioning_data(devkey,    //device key
                                                                 netkey,    //network key
                                                                 0,         //netkey index
                                                                 0,         //iv index 
                                                                 addr,      //node address
                                                                 0          //key refresh in progress
                                                                 );
        if (set_prov_rsp->result) {
            log(">failed to set provisioning data (res = %d)\r\n", set_prov_rsp->result);
            return;
        }

        log(">node self provisioned, address = %04x\r\n", addr);
    }

    // Application key
    key_count_rsp = gecko_cmd_mesh_test_get_key_count(1);
    if (key_count_rsp->result) {
        log(">failed to get appkey count (res = %d)\r\n", key_count_rsp->result);
        return;
    }

    if (0 == key_count_rsp->count) {
        // 2. Add application key
        struct gecko_msg_mesh_test_add_local_key_rsp_t *add_key_rsp;
        add_key_rsp = gecko_cmd_mesh_test_add_local_key(1,      //key type: appkey
                                                        appkey, //key data
                                                        0,      //key index
                                                        0       //netkey index
                                                        );
        if (add_key_rsp->result) {
            log(">failed to add appkey (res = %d)\r\n", add_key_rsp->result);
            return;
        }

        uint16_t elem_index;
        int16_t num_models;
        uint8_t mod_index;
        sig_model_t * p_sig_models;
        vendor_model_t * p_vendor_models;

        // 3. Bind model
        struct gecko_msg_mesh_test_bind_local_model_app_rsp_t *bind_model_rsp;
        for (elem_index = 0; elem_index < MESH_CFG_MAX_ELEMENTS; ++elem_index) {
            // SIG models
            num_models = dcd_get_sig_models(elem_index, &p_sig_models);
            for (mod_index = 0; mod_index < num_models; ++mod_index) {
                bind_model_rsp = gecko_cmd_mesh_test_bind_local_model_app(elem_index,   //element index
                                                                          0,            //appkey index
                                                                          0xFFFF,       //vendor id
                                                                          p_sig_models[mod_index].model_id  //model id
                                                                          );
                if (bind_model_rsp->result) {
                    log(">failed to bind SIG model %04x (res = %d)\r\n", p_sig_models[mod_index].model_id, bind_model_rsp->result);
                    break;
                }
                log("  bind sig model [%d] %04x\r\n", elem_index, p_sig_models[mod_index].model_id);
            }

            if (mod_index < num_models) {
                break;
            }

            // Vendor models
            num_models = dcd_get_vendor_models(elem_index, &p_vendor_models);
            for (mod_index = 0; mod_index < num_models; ++mod_index) {
                bind_model_rsp = gecko_cmd_mesh_test_bind_local_model_app(elem_index,   //element index
                                                                          0,            //appkey index
                                                                          p_vendor_models[mod_index].vendor_id, //vendor id
                                                                          p_vendor_models[mod_index].model_id   //model id
                                                                          );
                if (bind_model_rsp->result) {
                    log(">failed to bind vendor %04x model %04x (res = %d)\r\n", p_vendor_models[mod_index].vendor_id, p_vendor_models[mod_index].model_id, bind_model_rsp->result);
                    break;
                }
                log("  bind vend model [%d] %04x %04x\r\n", elem_index, p_vendor_models[mod_index].vendor_id, p_vendor_models[mod_index].model_id);
            }

            if (mod_index < num_models) {
                break;
            }
        }

        if (elem_index < MESH_CFG_MAX_ELEMENTS) {
            return;
        }

        const uint16_t svr_sub_list[] = GRP_SVR_SUB_ADDRESSES;
        const uint16_t cli_sub_list[] = GRP_CLI_SUB_ADDRESSES;
        const uint16_t num_svr_subs = sizeof(svr_sub_list)/sizeof(uint16_t);
        const uint16_t num_cli_subs = sizeof(cli_sub_list)/sizeof(uint16_t);
        uint16_t index;
        sig_model_t server_models[MAX_PUB_SUB_MODELS], client_models[MAX_PUB_SUB_MODELS];
        uint8_t num_server_models, num_client_models;

        // 4. Configure publication and subscription
        struct gecko_msg_mesh_test_set_local_model_pub_rsp_t *set_pub_rsp;
        struct gecko_msg_mesh_test_add_local_model_sub_rsp_t *add_sub_rsp;
        for (elem_index = 0; elem_index < MESH_CFG_MAX_ELEMENTS; ++elem_index) {
            num_models = dcd_get_sig_models(elem_index, &p_sig_models);
            if (num_models > 0) {
                // Server models
                num_server_models = grp_get_server_models(p_sig_models, num_models, server_models, MAX_PUB_SUB_MODELS);
                for (mod_index = 0; mod_index < num_server_models; ++mod_index) {
                    // Server publication
                    set_pub_rsp = gecko_cmd_mesh_test_set_local_model_pub(elem_index,   //element index
                                                                          0,            //appkey index
                                                                          0xFFFF,       //vendor id
                                                                          server_models[mod_index].model_id, //model id
                                                                          GRP_SVR_PUB_ADDRESS,      //publication address
                                                                          GRP_SVR_PUB_TTL,          //publication TTL
                                                                          GRP_SVR_PUB_PERIOD,       //publication period
                                                                          GRP_SVR_PUB_RETRANS,      //retransmission count and interval
                                                                          GRP_SVR_PUB_CREDENTIALS   //friendship credentials flag
                                                                          );
                    if (set_pub_rsp->result) {
                        log(">failed to set svr pub model %04x address %04x (res = %d)\r\n", server_models[mod_index].model_id, GRP_SVR_PUB_ADDRESS, set_pub_rsp->result);
                        break;
                    }
                    log("  set svr pub [%d] %04x -> %04x\r\n", elem_index, server_models[mod_index].model_id, GRP_SVR_PUB_ADDRESS);

                    // Server subscription
                    for (index = 0; index < num_svr_subs; ++index) {
                        add_sub_rsp = gecko_cmd_mesh_test_add_local_model_sub(elem_index,           //element index
                                                                              0xFFFF,               //vendor id
                                                                              server_models[mod_index].model_id, //model id
                                                                              svr_sub_list[index]   //subscription address
                                                                              );
                        if (add_sub_rsp->result) {
                            log(">failed to add svr sub model %04x address %04x (res = %d)\r\n", server_models[mod_index].model_id, svr_sub_list[index], add_sub_rsp->result);
                            break;
                        }
                        log("  add svr sub [%d] %04x <- %04x\r\n", elem_index, server_models[mod_index].model_id, svr_sub_list[index]);
                    }

                    if (index < num_svr_subs) {
                        break;
                    }
                }

                if (mod_index < num_server_models) {
                    break;
                }

                // Client models
                num_client_models = grp_get_client_models(p_sig_models, num_models, client_models, MAX_PUB_SUB_MODELS);
                for (mod_index = 0; mod_index < num_client_models; ++mod_index) {
                    // Client publication
                    set_pub_rsp = gecko_cmd_mesh_test_set_local_model_pub(elem_index,   //element index
                                                                          0,            //appkey index
                                                                          0xFFFF,       //vendor id
                                                                          client_models[mod_index].model_id, //model id
                                                                          GRP_CLI_PUB_ADDRESS,      //publication address
                                                                          GRP_CLI_PUB_TTL,          //publication TTL
                                                                          GRP_CLI_PUB_PERIOD,       //publication period
                                                                          GRP_CLI_PUB_RETRANS,      //retransmission count and interval
                                                                          GRP_CLI_PUB_CREDENTIALS   //friendship credentials flag
                                                                          );
                    if (set_pub_rsp->result) {
                        log(">failed to set cli pub model %04x address %04x (res = %d)\r\n", client_models[mod_index].model_id, GRP_CLI_PUB_ADDRESS, set_pub_rsp->result);
                        break;
                    }
                    log("  set cli pub [%d] %04x -> %04x\r\n", elem_index, client_models[mod_index].model_id, GRP_CLI_PUB_ADDRESS);

                    // Client subscription
                    for (index = 0; index < num_cli_subs; ++index) {
                        add_sub_rsp = gecko_cmd_mesh_test_add_local_model_sub(elem_index,           //element index
                                                                              0xFFFF,               //vendor id
                                                                              client_models[mod_index].model_id, //model id
                                                                              cli_sub_list[index]   //subscription address
                                                                              );
                        if (add_sub_rsp->result) {
                            log(">failed to add cli sub model %04x address %04x (res = %d)\r\n", client_models[mod_index].model_id, cli_sub_list[index], add_sub_rsp->result);
                            break;
                        }
                        log("  add cli sub [%d] %04x <- %04x\r\n", elem_index, client_models[mod_index].model_id, cli_sub_list[index]);
                    }

                    if (index < num_cli_subs) {
                        break;
                    }
                }

                if (mod_index < num_client_models) {
                    break;
                }
            }
        }

        if (elem_index < MESH_CFG_MAX_ELEMENTS) {
            return;
        }

        log(">node self configured, resetting ..\r\n");
        gecko_cmd_system_reset(0);
    }
}
