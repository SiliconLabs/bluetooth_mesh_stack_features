<!-- Author: Juha Laaksonen -->

# Mesh Nodes Removal From a Bluetooth Mesh Network

## Introduction

This article will introduce how to remove i.e. exclude (or reject list) nodes from a Bluetooth Mesh network and the related APIs when developing with Silicon Labs Bluetooth Mesh SDK. Excluding nodes is an important part of the Bluetooth network management, which prevents the "trash-can-attach" risk. It utilizes the key refresh procedure to remove the nodes whose keys are compromised.

> When a node is removed from the network, all remaining nodes would have their keys changed such that the removed node would not have knowledge of the new security credentials being used if that node was compromised after being disposed. This is known as the ”trash-can attack.”

There are 2 ways for Bluetooth Mesh network to remove one or more nodes from the network. The first one is via "Config Node Reset" command, the other way is via key refresh, both ways are supported by Silicon Labs Bluetooth Mesh SDK. Table 1 shows a comparison between these 2 ways.

|                                | Config Node Reset                               | Key Refresh                                          |
| :----------------------------- | :---------------------------------------------- | :--------------------------------------------------- |
| Network key change             | No                                              | Yes, the network key used in the subnet will change  |
| Node state                     | Unprovisioned                                   | Provisioned, but not in the previous network anymore |
| Need node(s) function normally | Yes, node(s) should be able to receive commands | No                                                   |

*Table 1. Comparison between Config Node Reset and Key Refresh*

In addition to excluding node(s) from the network, the key refresh procedure is also capatible to refresh the application key(s), which results in excluding node(s) from some specific group(s) but remaining in the network. One note for refreshing the application keys, it's impossible to refresh only the application keys without refresh the bound network key.

## How Key Refresh Works

If you are interested with the detailed information about how the key refresh procedure is designed and how it works, you can look into the chapter 3.10.4 of the [Mesh Profile Specification 1.0](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=429633).

Because the Silabs Bluetooth Mesh SDK has implemented this and packed the procedure to simple API calls, it's OK to just understand the procedure in general, it has 3 phases:

1. Receive the net key.
2. Inform the provisioner that the new key has been received.
3. Use the new key and revoke the old key.

## Related APIs In Silabs Bluetooth SDK

### Commands

- ```sl_btmesh_prov_set_key_refresh_exclusion``` - this is used to set the key refresh exclusion status of a node i.e. add a node to the reject list which is maintained by the stack
- ```sl_btmesh_prov_set_key_refresh_appkey_exclusion``` - same as the above one but for refreshing appkey
- ```sl_btmesh_prov_get_key_refresh_exclusion``` - this is used to check the key refresh exclusion status of a node i.e. check if a node is in the reject list which is maintained by the stack
- ```sl_btmesh_prov_get_key_refresh_appkey_exclusion``` - same as the above one but for refreshing appkey
- ```sl_btmesh_prov_start_key_refresh``` - command to start the key refresh procedure in the network, nodes in the reject list will get excluded

#### Test Class Commands

The test class commands are for developing and testing purpose, it's **NOT** recommended to use in the production firmware.

- ```sl_btmesh_test_update_local_key``` - this is used to update the key locally by a node
- ```sl_btmesh_test_prov_prepare_key_refresh``` - this command will set the network key and the application key(s) if any which will be used for the next key refresh procedure

### Events

- ```sl_btmesh_evt_prov_key_refresh_node_update``` - there are 3 parameters carried in this event, key (network key index), phase (which phase the node has moved into) and uuid (UUID of the node). This event indicates that the node has moved to the specific phase
- ```sl_btmesh_evt_prov_key_refresh_phase_update``` - this event indicates that the whole network has moved to the specific phase
- ```sl_btmesh_evt_prov_key_refresh_complete``` - this event indicates that the key refresh procedure has completed

### Typical Flow

Below is a pseudo code to demonstrate to exclude 3 nodes with UUID - uuid1, uuid2 and uuid3.

```pseudo-c
    #define REJECTLIST 1
    #define UUID_LEN  16
    #define NET_KEY_INDEX 0
    #define APP_KEY_NUM_TO_REFRESH 0
    sl_status_t sc;

    sc = sl_btmesh_prov_set_key_refresh_exclusion(netkey_id,
                                                  REJECTLIST,
                                                  uuid1);
    check the result value, sc.

    sc = sl_btmesh_prov_set_key_refresh_exclusion(netkey_id,
                                                  REJECTLIST,
                                                  uuid2);
    check the result value, sc.

    sc = sl_btmesh_prov_set_key_refresh_exclusion(netkey_id,
                                                  REJECTLIST,
                                                  uuid3);
    check the result value, sc.

    sc = sl_btmesh_prov_start_key_refresh(netkey_id,
                                          NET_KEY_INDEX,
                                          APP_KEY_NUM_TO_REFRESH,
                                          NULL);
    check the result value, sc.

    while(1) {
      check the events from the stack.
      switch(evt_id) {
        case sl_btmesh_evt_prov_key_refresh_node_update:
          record any information carried by the event if needed.
          break;

        case sl_btmesh_evt_prov_key_refresh_phase_update:
          record any information carried by the event if needed.
          break;

        case sl_btmesh_evt_prov_key_refresh_complete:
          check the e->data.evt_prov_key_refresh_complete.result to see if success?
          record the new network key index
          break;
       }
    }
    
    ...
```

## Conclusion

This guide has a related code example. Find it here:

The project in [Bluetooth Mesh Host Provisioner](https://github.com/SiliconLabs/bluetooth_mesh_stack_features/tree/btmesh_api_v2.x/provisioning/host_provisioner) implements the reject listing feature, you can use it to try out excluding nodes from the network. If you don't have time to go through and understand the whole project, you can just poll out the stuff in the source code included which contains all the reject listing related code.
