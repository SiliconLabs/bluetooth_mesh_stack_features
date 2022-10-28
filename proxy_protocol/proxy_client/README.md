# Proxy Client #

## Description ##

The proxy protocol enables nodes to send and receive Network PDUs, mesh beacons, proxy configuration messages and Provisioning PDUs over a connection-oriented bearer.

For example, a node could support GATT but not be able to advertise the Mesh Message AD Type. This node will establish a GATT connection with another node that supports the GATT bearer and the advertising bearer, using the Proxy protocol to forward messages between these bearers.

The proxy protocol defines two roles: the Proxy Server and the Proxy Client.

The Proxy Server is a node that supports a mesh bearer using the Proxy protocol and at least one other mesh bearer. For example, the Proxy Server can forward mesh messages between the advertising bearer and the GATT bearer.

The Proxy Client supports a mesh bearer using the Proxy protocol. For example, the Proxy Client can use the GATT bearer to send mesh messages to a node that supports the advertising bearer.

## Gecko SDK version ##

GSDK v4.1.1

---

## Important

This project README assumes that the reader is familiar with the usage of SiliconLabs Simplicity Studio 5 and the provided example projects within it.

---

## Requirements

  - Simplicity Studio 5 with the latest GSDK
  - 3x SiliconLabs WSTK with Radio Boards (for example BRD4162A)

## Known limitations:

  - The provisioning data is stored in the EFR32 internal flash and has limited capacity, the maximum number of nodes this example can support is around 10-15 

  - This example is missing many important features that are needed in a real production quality provisioner, such as:
    - Key refresh, blacklisting of devices
    - Removing nodes from the network

## Instructions

  - This Example is based on the ```Bluetooth Mesh - SoC Light``` included with the Silicon Labs GSDK
  - Create a new project based on the ```Bluetooth Mesh - SoC Light``` example
  - Copy the following files into the root directory of your project, overwrite the already existing ones:
    - app.c/h
    - config.c/h
  - Install the following components:
    - Application > Utility > Button Press
    - Bluetooth Mesh > Stack Classes > Provisioner
    - Bluetooth Mesh > Stack Classes > Configuration Client

  <img src="images/install_button_press.png">

  - Add the Configuration Client Model in the Bluetooth Mesh Configurator to the Main Element (skipping this would result in SL_STATUS_BT_MESH_DOES_NOT_EXIST ((sl_status_t)0x0502))
 
  <img src="images/add_config_client_model.png">

  - Increase the field values meant to store the data for Provisioned Devices (0 by default) at Bluetooth Mesh > Bluetooth Mesh Stack as per your needs
    - Maximum number of provisioned devices allowed
    - Maximum number of Application Keys allowed for each Provisioned Device
    - Maximum number of Network Keys allowed for each Provisioned Device
    - Max Foundation Client Cmds
      - For detailed explanation of these fields see: https://www.silabs.com/documents/public/user-guides/ug472-bluetooth-mesh-v2x-node-configuration-users-guide.pdf

  <img src="images/increase_values_1.png">
  <img src="images/increase_values_2.png">

  - When everything is configured, build and flash the project
  - Flash the other board with ```Bluetooth Mesh - SoC Empty``` example project
  - Via the terminal, you can see when the Provisioner device noticed the Unprovisioned Beacon. Now, by pressing ```Button 1``` on the Provisioner device, you can provision the newly noticed device into your network. If the provisioning is successful, the Provisioner will extract the DCD informations from the new Node.

  <img src="images/dcd_infos.png">

---