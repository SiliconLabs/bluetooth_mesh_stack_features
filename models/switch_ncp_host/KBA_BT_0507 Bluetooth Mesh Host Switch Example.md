<!-- Author: Juha Laaksonen -->

# Bluetooth Mesh Host Switch Example

This example duplicates the functionalities of the SDK example - soc-btmesh-switch with NCP mode, instead of controlling the light by the push buttons on the WSTK, it implements a lightweight console, so that you can use the CLI to control the lights in the network. For more information about the ncp mode and ncp host and target, please go through [KBA_BT_1602: NCP Host Implementation and Example](https://www.silabs.com/community/wireless/bluetooth/knowledge-base.entry.html/2018/01/18/ncp_host_implementat-PEsT).

## Hardware & SDK Requirements

- IDE – [Simplicity Studio 5](https://www.silabs.com/developers/simplicity-studio)
- SDK – Bluetooth Mesh SDK 2.0.0 GA or newer, the latest version is always recommended. Included with the Gecko SDK 3.1.0 or later.
- NCP target – At least 1 Bluetooth Mesh compatible boards - EFR32xG12, EFR32xG13 or EFR32xG21 (x= M, B) based, SLWRB4104A and SLWRB4103A are recommended.
- NCP host - **POSIX compatible** machine to run the host application. Running the application on Windows needs some porting afford, this has only been tested with Linux and Cygwin.

## How to Use It

The NCP mode requires both the host and target to work.

### Supported Commands

A simple console is implemented to receive commands from user. Users can add any customized command by adding an new item to _CMDs_ array in app.c.

| Command | Usage      | Description                         |
| :------ | :--------- | :---------------------------------- |
| l       | l [1/0]    | Set light on[1] or off[0]           |
| ln      | ln [0-100] | Set lightness [0% - 100%]           |
| ct      | ct [0-100] | Set Color Temperature [0% - 100%]   |
| fr      | fr [1/0]   | 1 - Factory Reset, 0 - normal reset |
| h       | h          | Print usage                         |
| exit    | exit       | Exit program                        |

### Generate NCP Target

1. Create "Bluetooth Mesh - NCP Empty" project based on the attached board and the latest Bluetooth Mesh SDK.
2. Open the _dcd_config.btmeshconf_ file, check that the "Light Lightness Client", "Generic Power OnOff Client" and "Light CTL Client" models are added to the Bluetooth SIG Models section. It's recommended to delete "Configuration Client" and "Generic OnOff Server" models from it. Modify the _sl_btmesh.h_ file in autogen-folder `#define SL_BTMESH_FEATURE_BITMASK` to `0x000b`.
3. Erase the attached board, then build and program to it. Don't forget the bootloader.

### Run the NCP Host example

The related code example can be found here: [https://github.com/SiliconLabs/bluetooth_mesh_stack_features/tree/master/models/switch_ncp_host](https://github.com/SiliconLabs/bluetooth_mesh_stack_features/tree/master/models/switch_ncp_host)

- Download the attachment and extract it.
- cd to the foler and run "make SDK_DIR=xxx", where xxx is the real directory of your BT Mesh SDK. Assuming it builds without errors.
- The executable takes 2 parameters - serial port and baud rate. Run it as "sudo ./exe/switch /dev/ttyACM2 115200"
- If the shell starts normally, type "h" to get usage example
- Find a provisioner and light node to test it.
