# Introduction

This article includes a code example that implements a **Bluetooth mesh provisioner** that can be run on one of our Bluetooth capable development kits.

To understand how the provisioning and configuration of mesh nodes works, it is recommended that you first try out the mesh Android app as explained in the quickstart guide [QSG148](https://www.silabs.com/documents/login/quick-start-guides/qsg148-bluetooth-mesh-demo-quick-start-guide.pdf). 

The example presented in this article can perform the same operations as the mobile app but it is more autonomous and requires only minimal input from the user. When the provisioner sees an unprovisioned beacon, it prints the UUID of this device to the debug console and then you can either:

\- push button PB1 to provision and configure the device (*

\- push button PB0 to ignore this beacon and continue scanning for other beacons (*

*) Alternatively, you can press key 'y' / 'n' on the terminal to provision or ignore the found device (starting from version v6)

The purpose of this example is to show how the Silicon Labs BT mesh stack can be used in provisioning mode. The main steps in provisioning and configuration are summarized below, along with a list of related API calls.

NOTE: This example is intended only for small scale experimental use. It is not meant to be used as a basis for any production use and the code.

**Known limitations**:

- The provisioning data is stored in the EFR32 internal flash and has limited capacity, the maximum number of nodes this example can support is around 10-15
- the max number of SIG and vendor models is limited to 16 and 4, respectively (see definition of struct tsDCD) 
- This example is missing many important features that are needed in a real production quality provisioner, such as:
  - key refresh, blacklisting of devices
  - removing nodes from the network

# Provisioning and configuration basic steps

The main steps included in provisioning a device into the network and configuring the node are summarized in the following table. This is just a compact list of what operations and API calls are involved in the process. More detailed information about provisioning and configuration in Bluetooth mesh can be found in:

- Mesh Profile specification by Bluetooth SIG, rev 1.0 (**Chapter 5 Provisioning**)
- https://blog.bluetooth.com/provisioning-a-bluetooth-mesh-network-part-1
- https://blog.bluetooth.com/provisioning-a-bluetooth-mesh-network-part-2

 

|              Step               |                         Description                          |                 Related API calls and events                 |
| :-----------------------------: | :----------------------------------------------------------: | :----------------------------------------------------------: |
| 1. Initialize provisionier mode | Initialize the Mesh stack in provisioner role. (wait for event | gecko_cmd_mesh_prov_init, gecko_evt_mesh_prov_initialized_id |
|         2. Create keys          | Application and network keys need to be created (unless they already exist) | gecko_cmd_mesh_prov_create_network, gecko_cmd_mesh_prov_create_appkey |
|           3. Scanning           |           Start scanning for unprovisioned beacons           | gecko_cmd_mesh_prov_scan_unprov_beacons, gecko_evt_mesh_prov_unprov_beacon_id |
|      4. Start provisioning      | Start provisioning, using either PB-ADV or PB-GATT bearer *  | gecko_cmd_mesh_prov_provision_device, gecko_cmd_mesh_prov_provision_gatt_device, gecko_evt_mesh_prov_device_provisioned_id |
|           5. Read DCD           | Read the device composition data (DCD) from the freshly provisioned node | gecko_cmd_mesh_config_client_get_dcd, gecko_evt_mesh_config_client_dcd_data_id, gecko_evt_mesh_config_client_dcd_data_end_id |
|         6. Send appkey          |             Push an application key to the node.             | gecko_cmd_mesh_config_client_add_appkey, gecko_evt_mesh_config_client_appkey_status_id |
|   7. Bind appkey to model(s)    |             Bind a model to an application key.              | gecko_cmd_mesh_config_client_bind_model, gecko_evt_mesh_config_client_binding_status_id |
|       8. Publish settings       |   Set a model's publication address, key, and parameters.    | gecko_cmd_mesh_config_client_set_model_pub, gecko_evt_mesh_config_client_model_pub_status_id |
|      9. Subscribe settings      |        Add an address to a model's subscription list.        | gecko_cmd_mesh_config_client_add_model_sub, gecko_evt_mesh_config_client_model_sub_status_id |

 

# Code example

The sample code is attached in a zip file. The zip file includes a **readme.txt** file that explains step-by-step how this code can be installed on top of one of the examples in the Bluetooth mesh SDK.

The Silicon Labs Bluetooth Mesh SDK performs consistency checks on the non-volatile storage areas, that hold information such as keys, at startup. These areas are not erased when flashing a new application. If the application on the EFR32 is changed from embedded provisioner to something else, like a light or switch node, or from such a node to embedded provisioner, the entire contents of flash must be erased to avoid errors during initialization. This can be accomplished with Simplicity Commander 

(commander device recover)

**Version history**:

| Revision |        Date        |                           Comments                           |
| :------: | :----------------: | :----------------------------------------------------------: |
|    v7    |      May 10th      |        Added call to gecko_bgapi_class_mesh_prov_init        |
|    v6    | February 28, 2019  | Example modified to use the new API calls (introduced in SDK 1.4.0) and all deprecated calls removed. Additionally some refactoring of the code + minor improvements. |
|    v5    |  January 22, 2019  | Modified to work with Mesh SDK 1.4.0. Max number of SIG models increased to 16. Some sanity checks added. |
|    v4    | September 14, 2018 |            Modified to work with Mesh SDK 1.3.x.             |
|    v3    |   June 11, 2018    | Option to provision using PB-GATT, Option to use fixed application and network keys |
|    v2    |    May 31, 2018    | First draft (based on code package that was earlier posted under a discussion [thread](https://www.silabs.com/community/wireless/bluetooth/forum.topic.0.15.html/appkey_deploymentfa-SJYZ) |
|          |                    |                                                              |



## Composition Data and Memory Configuration 



In order for the provisioner to configure the nodes there are a few requirements that must be met.

Firstly, the config client must be added to the device composition data (DCD) as shown

![config-client](images\config-client.PNG)



The memory configuration tab contains several settings which must be modified as well. These parameters should all be set to non-zero, the default values for each are zero.

- max provisioned devices
- max provisioned device netkeys
- max foundation client commands

For detailed descriptions of these settings please refer to section 3 of UG366.