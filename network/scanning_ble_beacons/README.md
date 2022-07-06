# Scanning BLE beacons in Bluetooth Mesh mode #

## Summary ##

Bluetooth mesh is based on advertisements, meaning that under the hood the stack is scanning all the time. By default, the scan response events are not passed to the application but they are just consumed by the mesh stack and then silently discarded.

This example illustrated a way to scan BLE advertisements when the Bluetooth mesh stack is running. It looks through advertising data in scan response events and searches for the mesh proxy UUID 0x1828. If a proxy advertisement is found, the address of the sender and RSSI are printed to debug output.

### Implementation 

Step 1: Handle the scan response event by adding following case to the sl_bt_on_event() function:
```C
case sl_bt_evt_scanner_scan_report_id:  
  print_scan_resp(&evt->data.evt_scanner_scan_report);  
  break;
```
Step 2: Add a function that handles the BLE scan responses, here's a sample:   
```C
static void print_scan_resp(sl_bt_evt_scanner_scan_report_t *pResp)
{  
  // decoding advertising packets is done here. The list of AD types can be found
  // at: https://www.bluetooth.com/specifications/assigned-numbers/Generic-Access-Profile

  // example of adv data including proxy service data:020106030328181416281801fd65f619452875558774bc
  // (UUID 0x1828)

  // 020106-03032818-1416281801fd65f619452875558774bc
  const uint8_t proxy_UUID[16] = {0x28, 0x18};

  int i = 0;
  int ad_match_found = 0;
  int ad_len;
  int ad_type;

  while (i < (pResp->data.len - 1))
  {

    ad_len  = pResp->data.data[i];
    ad_type = pResp->data.data[i+1];

    if (ad_type == 0x03)
    {
      // type 0x03= Complete List of 16-bit Service Class UUIDs

      if(memcmp(proxy_UUID, &(pResp->data.data[i+2]),2) == 0)
      {
        ad_match_found = 1;
      }
    }

    //jump to next AD record
    i = i + ad_len + 1;
  }

  if(ad_match_found)
  {
    for(i=5;i>=0;i--)
    {
      printf("%2.2x", pResp->address.addr[i]);
    }  
    printf(", RSSI: %d\r\n", pResp->rssi);
  }
}
```

## Gecko SDK version ##

- Bluetooth SDK v4.1.0

## Hardware Required ##

- Two WSTK boards
- Two Bluetooth capable radio boards, e.g: BRD4104A

## Set up ##

1. Create a **Bluetooth Mesh - SoC Light** project in Simplicity Studio.

2. The function code is in the *app.c* file. Copy and replace this file to the project folder.

3. Compile and flash to your radio board, then provision it.

4. Open serial console and observe the print statements.

5. On another radio board, flash any Bluetooth mesh application consisting of the Mesh Proxy service with UUID 0x1828 and provision it.
