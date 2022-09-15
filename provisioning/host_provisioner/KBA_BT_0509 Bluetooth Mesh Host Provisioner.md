# BT Mesh Host Network Manager (Provisioner)

A provisioner plays an important role in the Bluetooth Mesh network, which creates and manages the network by adding, configuring and removing devices into/from the network. This article will provide you a ncp host example which works with the ncp target to act as the provisioner. For more information about the ncp mode and ncp host and target, please go through *KBA_BT_1602: NCP Host Implementation and Example*.

Prior to this example, we have the SoC mode provisioner example introduced in *KBA_BT_0501: BT Mesh embedded provisioner example*, it has the full implementation of provisioning and configuring a device into a network and is **easier to understand and more suitable for beginners**. However, it has below limitations:

- In SoC mode, only 14 devices can be added to the same network, while in ncp mode, it supports to add up to **512 devices** for now.

- Application and network configuration are fixed at the compiling time, any changes to the network require to re-build and flash the provsioner.

- Due to the limitation of the resources on the WSTK, it has limited features.

Given the above limitations, the host network manager was developed with below ideas in mind.

- Make it as automatic as possible so that it can be used for automatic testing.

- Separate the application and configuration, so that it doesn't need to rebuild the application to apply any configuration changes to the network.

- Configuration file can be changed at any time, the application will check if needs to reload them before issuing a command.

- Robustness improvement, every process may fail because of any reason. Retry and some recovery mechanisms are needed to make the network more robust.

- A console to receive commands from the user. Easy to add any customized command to extend the application.

## Capabilities

The host network manager supports below functionalities:

- Create network and application keys

- Provision multiple devices into a network simultaneously

- Configure multiple devices simultaneously

  - Add application keys

  - Bind application keys to models

  - Set publication address to models

  - Add subscription addresses to models

  - Set default TTL value

  - Set relay/friend/proxy feature on or off
  - Set the network/relay transmission - count and interval

  - Set Secure Network Beacon on or off

- Remove devices from a network via node reset procedure

- Blacklist devices from a network via key refresh procedure

- Set the light status

  - On or off

  - Lightness

  - Color temperature

**NOTE:** Removing a node is different than blacklisting a node from the network. Removing is like to ask the node to leave the network actively - provisioner sends a command to force the node to factory reset itself, while blacklist is passively being blacklisted from the network - provisioner sends packets to the reset of the nodes to migrate to a new network key, see table 1. For more information, you can go through *KBA_BT_0510: Blacklisting Nodes From Bluetooth Mesh Network*.

| | Remove | Blacklist |
| - | - | - |
| Network key change | No | Yes |
| Before the procedure | Target needs to work normally | Doesn't matter |
| After the procedure | Factory reset | Remaining unchanged |
*Table 1. Comparison between removing and blacklisting*

## Dependencies

### Hardware & SDK

- IDE – Simplicity Studio 4

- SDK – Bluetooth Mesh SDK 1.6.0 GA or newer, the **latest version is always recommended**.

- NCP target – WSTK with Bluetooth Mesh compatible radio boards - EFR32xG12, EFR32xG13 or EFR32MG21 (x= M, B) based.

- NCP host - **POSIX compatiable** machine to run the host application, **THIS HAS ONLY BEEN TESTED ON Linux and MacOS**. Running the application on Windows or other platforms probably needs some porting effort.
