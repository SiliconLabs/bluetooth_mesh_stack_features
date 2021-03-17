# Silicon Labs Bluetooth Mesh Static and Input OOB provisioning

This code shows how to perform OOB provisioning using the following methods:
 - input method
 - static method

The ouput method isn't present there because it basically consist in the same thing as
the input method only the origin of OOB data differs. 

Public key sharing isn't demonstrated here for the time being.

In both cases, the provisioner code (provisioner.c) is derived from the [embedded provisioner](https://github.com/SiliconLabs/bluetooth_mesh_stack_features_staging/tree/master/provisioning/embedded_provisioner).
For more details, please refer to it.

Follow the steps described in the embedded provisioner example and replace the app.c content by the provisioner.c

For the node to be provisioned, create an soc\_btmesh\_light project and replace the app.c content by app\_light\_node.c