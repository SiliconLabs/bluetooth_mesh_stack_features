# Proxy Client Example #

## Description ##

The proxy protocol enables nodes to send and receive Network PDUs, mesh beacons, proxy configuration messages and Provisioning PDUs over a connection-oriented bearer.

For example, a node could support GATT but not be able to advertise the Mesh Message AD Type. This node will establish a GATT connection with another node that supports the GATT bearer and the advertising bearer, using the Proxy protocol to forward messages between these bearers.

The proxy protocol defines two roles: the Proxy Server and the Proxy Client.

The Proxy Server is a node that supports a mesh bearer using the Proxy protocol and at least one other mesh bearer. For example, the Proxy Server can forward mesh messages between the advertising bearer and the GATT bearer.

The Proxy Client supports a mesh bearer using the Proxy protocol. For example, the Proxy Client can use the GATT bearer to send mesh messages to a node that supports the advertising bearer.

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

## Instructions

This Example is based on the ```Bluetooth Mesh - SoC Light CTL``` included with the Silicon Labs GSDK

  - Create a new project based on the ```Bluetooth Mesh - SoC Light CTL``` example
  - Copy the following file into the root directory of your project, overwriting the already existing one:
    - src/app.c
  - Install the following components:
    - Bluetooth Mesh > Models > Lighting > LC Server
    - Bluetooth Mesh > Stack > Test
  - When everything is configured, build and flash the project