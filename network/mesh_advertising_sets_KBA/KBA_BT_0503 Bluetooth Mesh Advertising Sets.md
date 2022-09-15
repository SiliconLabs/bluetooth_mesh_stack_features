# Bluetooth Mesh advertising sets

## Introduction

Bluetooth Low Energy defines advertising data belonging together as an advertising set. The stack host may use then the link layer to interleave advertising events with certain advertising period, AD type and payload, PHY and other advertising parameters. All those elements defines what is referred to as advertising set.

Advertising set are independent from each other. For example, a connectable advertisement and a non-connectable beacon can be broadcasted simultaneously without the need to sequentially send the connectable advertisement, then modify the content and parameters to send the non-connectable beacon subsequently. This can be easily done using two advertising sets.

This article describes advertising sets as implemented in the Bluetooth Mesh SDK v2.x.

## Advertising Sets Used by Bluetooth Mesh Stack

The application code needs to set the maximum number of advertising sets used by both the stack and application. The Bluetooth Mesh stack uses some predefined advertising sets for its operation as described below :

```
/// Bluetooth advertisement set configuration
///
// <o SL_BT_CONFIG_USER_ADVERTISERS> Max number of advertisers reserved for user <0-8>
// <i> Default: 1
// <i> Define the number of advertisers the application needs.
#define SL_BT_CONFIG_USER_ADVERTISERS (1)
#define SL_BT_CONFIG_MAX_ADVERTISERS (SL_BT_CONFIG_USER_ADVERTISERS + SL_BT_COMPONENT_ADVERTISERS)

```

Similarly to SDK v1.7.x one advertising set is reserved by default for the user (advertising set 0). Up to 8 advertising sets can be used by the user application code if needed (SL_BT_CONFIG_USER_ADVERTISERS macros)

```
// <o SL_BTMESH_CONFIG_MAX_NETKEYS> Maximum number of Network Keys allowed <0-4>
// <i> Default: 4
// <i> Define the number of Network Keys the application needs.
#define SL_BTMESH_CONFIG_MAX_NETKEYS (4)
#define SL_BTMESH_COMPONENT_ADVERTISERS (3 + SL_BTMESH_CONFIG_MAX_NETKEYS)
#define SL_BT_COMPONENT_ADVERTISERS (0 + SL_BTMESH_COMPONENT_ADVERTISERS)
```

As a result, three advertisement sets are reserved for Mesh unprovisioned data and messages and by default four advertisement sets are reserved for Mesh GATT service advertisements. In case of the Mesh GATT advertisement, one advertisement set is used per Mesh network (per network key that is).

**Advertising Set 0 - Reserved for Application**

This advertising set is reserved for regular BLE advertising. It's created when the stack initializes by default and ready to be used by the application. For example, you can call the API `sl_bt_advetiser_create_set()` and then `sl_bt_advertiser_start()` with handle 0 to start advertising with this advertising set. This is NOT used by the Bluetooth Mesh stack, but has to be counted into the maximum number of the advertising sets. By default, one advertisement set is dedicated to the user application but up 8 advertisement set could be used.

**Advertising Set 1 - Bluetooth Mesh Data**

This advertising set is used for Bluetooth Mesh data traffic, which is the traffic transmitted on the Advertising bearer (Mesh Messsage AD type) (section 3.3.1 of [Mesh Profile v1.0.1](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=457092)) and PB-ADV provisioning bearer (section 5.2.1 of [Mesh Profile v1.0.1](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=457092)) . The stack is in control of the advertising data and the advertising parameters for this set. The user application MUST NOT override the parameters or use this set for any other purpose.

**Advertising Set 2 - Bluetooth Mesh Unprovisioned Device Beacon Without URI**

This advertising set is used for sending the Unprovisioned Device Beacon (section 3.9.2 of [Mesh Profile v1.0.1](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=457092)) when devices are in unprovisioned state. The Bluetooth Mesh stack will construct the advertising payload according to the definition of the Unprovisioned Device beacon format. User application shall not modify the advertising payload to this advertising set. What the user application can control in this case is the cadence for sending the Unprovisioned Device beacon, call the API - `sl_bt_advertiser_set_timing()` with handle 2 **BEFORE** starting the Unprovisioned Device beacon.

**Advertising Set 3 - Bluetooth Mesh Unprovisioned Device Beacon With URI**

This advertising set is also used for sending the Unprovisioned Device Beacon (section 3.9.2 of [Mesh Profile v1.0.1](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=457092)) when devices are in unprovisioned state. The only difference is there are 4-byte of URI hash appended to the data payload. See below lines from the Mesh Profile.

> Along with the Unprovisioned Device beacon, the device may also advertise a separate non-connectable advertising packet with a Uniform Resource Identifier (URI) data type (as defined in [7]) that points to outof-band (OOB) information such as a public key. To allow the association of the advertised URI with the Unprovisioned Device beacon, the beacon may contain an optional 4-octet URI Hash field.

The latest Bluetooth Mesh stack doesn't support sending Unprovisioned Device beacon with URI, so this is reserved for future use.

**Advertising Set 4 to N - Bluetooth Mesh Proxy Service Advertising**

The usage of this advertising set is device state dependent.

If a device is in unprovisioned state, this advertising set will be used to advertise the Mesh Provisioning Service (section 7.1 of [Mesh Profile v1.0.1](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=457092)) if the application call `sl_btmesh_node_start_unprov_beaconing()` with PB-GATT bit set in the bearer parameter.

If a device is in provisioned state and supports Proxy feature, it shall start advertising the Mesh Proxy Service (section 7.2 of [Mesh Profile v1.0.1](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=457092)) if the given situations are met. Alternatively, user application can start the advertising manually whenever the device is expected to connect to a proxy client.

Because a provisioned device could belong to multiple subnets, it could advertise multiple Mesh Proxy Service advertisements simultaneously. The maximum network key number that the device supports decides the maximum number of subnets the device can be in, so N is calculated to be 4 + the maximum network key on the device.

## Adding Customized Advertising Sets to Bluetooth Mesh Applications

This example shows how to add your custom BLE advertising into a Bluetooth Mesh project.

**Step 1**: Increase the MAX_ADVERTISERS macro by the number of your custom advertising sets.

The .bluetooth.max_advertisers parameter in the sl_bt_configuration_t structure decides the maximum number of the advertising sets in the application. In all examples of the Bluetooth Mesh SDK, this value is feed by SL_BT_CONFIG_MAX_ADVERTISERS macro. So, modify the SL_BT_CONFIG_MAX_ADVERTISERS macro like below to add your own advertising sets.

```
/* Number of custom Advertising Sets */
#define MY_ADVERTISERS 2
#define SL_BT_CONFIG_MAX_ADVERTISERS (SL_BT_CONFIG_USER_ADVERTISERS + SL_BT_COMPONENT_ADVERTISERS + MY_ADVERTISERS)

/*
 * Advertising Set ID from the offset. E.g. to get the first custom advertising
 * set ID, put n as 1 like this - MY_ADVERTISING_SET_ID(1)
 */
#define MY_ADVERTISING_SET_ID(n)    (SL_BT_CONFIG_MAX_ADVERTISERS + n)
```

**Step 2**: Configure the advertising sets if needed.

You can use the advertising configuration related APIs (sl_bt_advertiser_xxx) to configure the advertising sets specifically by the advertising set ID (handle). For more details about the advertising parameters and data payload format, you can check [Bluetooth Advertising Data Basics](https://docs.silabs.com/bluetooth/latest/general/adv-and-scanning/bluetooth-adv-data-basics). For constructing the advertising data in a much easier way, you can use the code example - [Advertisement or Scan Response Constructor](https://docs.silabs.com/bluetooth/latest/code-examples/stack-features/adv-and-scanning/adv-or-scan-response-constructor).

**Step 3**: Start the advertising sets

This is the final step to send your own advertisement, call `sl_bt_advertiser_start()` with advertising set ID (handle) specified to start advertising the specific advertisement.

There is basically no difference compared to regular (non-mesh) BLE case. The key thing to take into account here is **configuring the number of advertising sets** and **selecting the right advertising handle** to use.

## Conclusion

This article presents briefly the concept of advertising sets and how those can be used in your application.