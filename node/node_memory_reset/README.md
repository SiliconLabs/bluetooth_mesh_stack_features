# Node Memory Reset i.e. Erase NVM Example

## Description ##

The NVM3 driver provides a means to write and read data objects (key/value pairs) stored in flash. Wear-leveling is applied to reduce erase and write cycles and maximize flash lifetime. The driver is resilient to power loss and reset events, ensuring that objects retrieved from the driver are always in a valid state. A single NVM3 instance can be shared among several wireless stacks and application code, making it well-suited for multiprotocol applications.

This example project is aimed to demonstrate how to erase the entire Bluetooth Mesh configuration region from the NVM by deleting the associated keys. This example is not meant to be used in production code, see ```Known limitations```! The Bluetooth Mesh PS entries are stored between keys 0x1000 and 0x2FFF.

## Simplicity SDK version ##

SiSDK v2024.6.0

---

## Important

> ⚠ You are not required to follow through with the setup part of the Instructions when using our [*External Repos*](../../README.md) feature!

This project README assumes that the reader is familiar with the usage of SiliconLabs Simplicity Studio 5 and the provided example projects within it.

---

## Requirements

  - Simplicity Studio 5 with the latest SiSDK
  - SiliconLabs WSTK with Radio Board (for example BRD4187C)

## Known limitations:

  - To delete the Bluetooth Mesh configurations, in production please use:
    - ```sl_btmesh_initiate_full_reset();```
    - ```sl_btmesh_initiate_node_reset();```

## Instructions

  - This Example is based on the ```Bluetooth Mesh - SoC Light CTL``` included with the Silicon Labs SiSDK
  - Create a new project based on the ```Bluetooth Mesh - SoC Light CTL``` example
  - Copy the following files into the root directory of your project, overwriting the already existing ones:
    - src/app.c
    - src/mesh_erase_nvm.c
    - inc/mesh_erase_nvm.h
  - Install the following components:
    - Bluetooth Mesh > Models > Lighting > LC Server
    - Bluetooth > Bluetooth Host (Stack) > Additional Features > NVM Support
  - When everything is configured, build and flash the projects
  - If everything went right, you should see the board booting up
  - Now, you may provision the node into a Network and after that, erase the NVM content with pressing ```Button 0``` and ```Reset``` at the same time. You will notice that as the result of the loss of data, the node will not be able properly reinitialize.

  ![erase_nvm](images/erase_nvm.png)
  ![init_failed](images/init_failed.png)

---