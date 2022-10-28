# HSL Lighting

## Introduction

Lighting is BT mesh important application scenario. Here we disuss on lighting terminology and how our stack handle it.

### How We Describe Color
Nowadays, lighting is not only for ON/OFF. Most of light provide color setting.

There are 4 primary ways to describe a color.

<img src="doc/color_system.png">

- Hue, another word for color, degree on the color wheel from 0 to 360
  - 0 is red
  - 120 is green
  - 240 is blue

- Saturation(chroma), the intensity or purity of a hue, is a percentage value
  - 0% means a shade of gray
  - 100% is the full color

- Lightness, the relative degree of black or white mixed with a given hue, is also a percentage value
  - 0% is black
  - 50% is neither light or dark
  - 100% is white

- Temperature, the perceived warmth or coolness of a color, measured in degrees Kelvin (K)

  <img src="doc/color_temperature.png" width="488" height="526">

### HSL Control Topolog
![HSL](doc/hsl_topo.png)

The Bluetooth Mesh - SoC Light HSL example is a working example application that you can use as a template for Bluetooth Mesh HSL Light applications.

Currently the Bluetooth Mesh - SoC Switch does not support HSL Client (i.e. cannot set hue or saturation). So switch side needs some porting job.  

## Project Setup
### HSL Switch
1) Create "Bluetooth Mesh - SoC Switch", add HSL client model. The example can run on EFR32 boards which support BT mesh, the provided project file is based on xG21(BRD4181A).

  <img src="doc/switch_project.png" width="1310" height="658">

2) Check SL_BTMESH_GENERIC_HSL_CLIENT_INIT_CFG_VAL on sl_btmesh_generic_base_config.h, if not set then set it.

  <img src="doc/switch_config.png" width="672" height="705">
  
3) Create btmesh_hsl_client folder(/gecko_sdk_4.0.x/app/bluetooth/common/btmesh_hsl_client), then copy provided btmesh_hsl_client.c and btmesh_hsl_client.h into the folder.

  <img src="doc/switch_copy_files.png" width="380" height="348">

Configure the publish parameter as below, test code set 7 HSL combination data to show color changing.
  ```c
  void sl_btmesh_set_hsl(uint8_t new_hsl)
  {
    uint16_thsl_table[HSL_INDEX_MAX][3] = {
        {0,0,0},       //Off
        {0,100,50},    //Red
        {120,100,50},  //Green
        {240,100,50},  //Blue
        {39,100,50},   //Orange
        {300,76,72},   //Pink
        {248,53,58},   //Purple
    };
    target_hue = (double)hsl_table[new_hsl][0]/360*65535;
    target_saturation = (double)hsl_table[new_hsl][1]/100*65535;
    target_lightness = (double)hsl_table[new_hsl][2]/100*65535;
    //...
  }
  ```
  ```c
  static void send_hsl_request(uint8_t retrans)
  {
    //...
    req.kind = mesh_lighting_request_hsl;
    req.hsl.hue = target_hue;
    req.hsl.saturation = target_saturation;
    req.hsl.lightness = target_lightness;
    //...
    sc = mesh_lib_generic_client_publish(MESH_LIGHTING_HSL_CLIENT_MODEL_ID,
                                      BTMESH_HSL_CLIENT_MAIN,
                                      hsl_trid,
                                      &req,
                                      IMMEDIATE,     // transition
                                      delay,
                                      NO_FLAGS       // flags
                                      );
    //...
  }
  ```

4) Replace app.c file, invoke sl_btmesh_change_hsl() in app_button_press_cb()
<img src="doc/switch_replace_file.png" width="1356" height="1110">

## Testing
### Gecko SDK Version
Test on Bluetooth Mesh 2.2.x

### Hardware Required
- Wireless STK Mainboard and EFR32xG21 Radio Board.
- Thunderboard Sense 2, it has RGB LED, better for showing the color.

### HSL Switch
Import the project file SimplicityStudio/soc_btmesh_switch_HSL.sls, should be good way to setup the testing.

### HSL Light
Use default Simplicity Studio "Bluetooth Mesh - SoC HSL Light" is OK.
For checking the color change according to HSL client, the recommend HW is Thunderboard Sense 2(BRD4166A).
The light color finally is control by RGB setting, rgb_led_set().

  <img src="doc/light_hsl_rgb.png">

### Provisioner
There is Mesh Host Provisioner C Example for NCP mode in Bluetooth Mesh 2.2.x.
SimplicityStudio/SDKs/gecko_sdk/app/bluetooth/example_host/btmesh_provisioner

Can be use for provsion HSL light and switch.

### Test Log
Each key press on button 0 will send HSL client control message, then the light will change the color accordingly.
  <img src="doc/test_log.png">