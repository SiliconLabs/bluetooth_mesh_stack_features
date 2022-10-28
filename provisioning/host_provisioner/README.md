# Host Provisioner #

## Description ##

Provisioner - A node that is capable of adding a device to a mesh network.

Provisioning is the process of adding a new, unprovisioned device to a Bluetooth mesh network, such as a light bulb. The process is managed by a provisioner. A provisioner and an unprovisioned device follow a fixed procedure which is defined in the Bluetooth mesh specification. A provisioner provides the unprovisioned device with provisioning data that allows it to become a Bluetooh mesh node.

This example is aimed to demonstrate our Mesh library's capabilities in building a host provisioner solution with a RaspberryPi as an NCP host and a WSTK board as an NCP target.

---

## Gecko SDK version ##

GSDK v4.1.1

## Important

This project README assumes that the reader is familiar with the usage of SiliconLabs Simplicity Studio 5 and the provided example projects within it. This example is based on the ```Bluetooth - NCP Host``` one, already available in Simplicity Studio 5, extended with the node "exclusion list" functionality.

---

## Requirements

  - Simplicity Studio 5 with the latest GSDK
  - SiliconLabs WSTK with a Radio Board (for example BRD4162A)
  - RaspberryPi or other personal computer with POSIX compliant environment

## Known limitations:

  - EFR32xG22 boards are not supported

## Instructions

  - Open a new terminal on the computer
  - Navigate to the project's folder
  - Build the sample app by typing ```make```
  - Flash the radio board with the ```Bluetooth - NCP``` example included with the Silicon Labs GSDK
  - Connect the NCP target to the NCP host (if the NCP host differs from the computer used to flash the radio board)
  - Run the sample app with appropriate parameters (e.g. ```./exe/btmesh_host_provisioner -u /dev/ttyACM0```)
  - Type the desired functionality, which you would like to perform

  <img src="images/terminal.png">

  - You are free to run the program with command line arguments

  <img src="images/arguments.png">

---