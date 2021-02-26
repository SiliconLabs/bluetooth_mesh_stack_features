
---------------------------------------------------------------------------------
NOTE: This example is intended only for small scale experimental use. It is 
not meant to be used as a basis for any production use and the code.

Known limitations:

* The provisioning data is stored in the EFR32 internal flash and has limited capacity, 
  the maximum number of nodes this example can support is around 10-15 

* This example is missing many important features that are needed in a real production 
  quality provisioner, such as:
    - key refresh, blacklisting of devices
    - removing nodes from the network

---------------------------------------------------------------------------------

2) Create an soc-btmesh-ligh project.
	
2) Drag and drop the folling files at the root directory of your project:
  - app.c/h
  - config.c/h
  - main.c

3) open your the dcd configurator of your project:
  - add the config client model to the primary element of your project
  - set the following macros in the memory configuration:
    . Max Provisioner Devices : 4
      
This code has been tested with Bluetooth mesh SDK 1.7.3.
