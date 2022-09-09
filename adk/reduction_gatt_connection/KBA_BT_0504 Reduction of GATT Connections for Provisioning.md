# Bluetooth Mesh Mobile - Reduction of GATT Connections for Provisioning with Configuration - Implementation

## 1. Introduction

In the version 2.3.2 of our Bluetooth Mesh Mobile ADK we have introduced a new feature which allows the User to reduce the amount of separate GATT connections necessary for provisioning of the Node with configuration. Enabling it should not only improve the performance of the operation but also increase reliability and success rates during those procedures. That's because both the provisioning process and the initial proxy session with configuration of the Node is going to take place under a **single connection**.

The main aim of this article is to give you a simple instruction on how to implement all the necessary changes into the application which are required to reduce number of GATT connections needed.

## 2. Android

### 2.1 BluetoothConnectableDevice modifications

First of all, for the new solution to work it is necessary to make the following changes to the `BluetoothConnectableDevice` class.

**Warning:** The following modifications have been already applied in the Bluetooth Mesh by Silicon Labs demo application released with **ADK 2.3.2** and the full class implementation can be found in the attachment to this article.

```Java
var bluetoothGatt: BluetoothGatt? = null

...

override fun hasService(service: UUID?): Boolean {
    return if (bluetoothGatt?.services?.isNotEmpty() == true) {
        bluetoothGatt!!.getService(service) != null
    } else {
        scanResult.scanRecord?.serviceUuids?.contains(ParcelUuid(service))
                ?: false
    }
}
var bluetoothGatt: BluetoothGatt? = null
var refreshGattServicesCallback: RefreshGattServicesCallback? = null
lateinit var bluetoothGattCallback: BluetoothGattCallback

...

bluetoothGattCallback = object : BluetoothGattCallback() {
    override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {

        ...

        if (status == BluetoothGatt.GATT_SUCCESS) {
            refreshGattServicesCallback?.onSuccess()
        } else {
            refreshGattServicesCallback?.onFail()
        }
        refreshGattServicesCallback = null

        ...

    }
}

...

override fun refreshGattServices(refreshGattServicesCallback: RefreshGattServicesCallback) {
    if (refreshDeviceCache()) {
        this.refreshGattServicesCallback = refreshGattServicesCallback
        bluetoothGatt!!.discoverServices(bluetoothGatt)
    } else {
        refreshGattServicesCallback.onFail()
    }
}

...

fun refreshDeviceCache(): Boolean {
    var result = false
    try {
        val refreshMethod: Method = bluetoothGatt!!.javaClass.getMethod("refresh")
        result = refreshMethod.invoke(bluetoothGatt!!, *arrayOfNulls(0)) as? Boolean ?: false
    } catch (exception: Exception) {
    }
    return result
}
```

### 2.2 Provision with configuration under one connection

Having successfully incorporated the code attached above, the Developer may proceed to make changes that will make the provisioning and configuration to happen at the same GATT connection. It would require a change in the execution order which is presented below.

#### 2.2.1 Execution modification

The User of the ADK needs to make sure that he/she creates a `ProvisionerConfiguration` object, sets the `isUsingOneGattConnection` property to `true` and then adds that object as an argument for the `provision` method.

**Note:** It is also possible to add other initial configuration to that object such as setting the Node as Proxy or Relay.

```Java
val provisionerConnection: ProvisionerConnection
val provisioningCallback: ProvisioningCallback

...

val provisionerConfiguration = ProvisionerConfiguration().apply {
            isUsingOneGattConnection = true
}
provisionerConnection.provision(provisionerConfiguration, null, provisioningCallback)
```

## 3. iOS

On the iOS the situation looks very similar to the Android. The appropriate steps have been covered below

### 3.1 SBMConnectableDevice modifications

Firstly, it is necessary to make modifications to class implementing `SBMConnectableDevice` protocol by implementation for new method and calling `SBMConnectableDeviceDelegate.didModifyServices(, for:)` method of delegate object.

**Warning:** Those changes have been already applied in the Bluetooth Mesh by Silicon Labs demo application released with **ADK 2.3.2** and the full class implementation can be found in the attachment to this article.

**SBMConnectableDevice:**

```Java
let peripheral: CBPeripheral
var delegate: SBMConnectableDeviceDelegate?// delegate is set by BluetoothMesh framework, do NOT override it
var callback: SBMConnectableDeviceOperationCallback?

// (...)

//   SBMConnectableDevice
func unsubscribe(fromService service: String, characteristic: String, completion: @escaping SBMConnectableDeviceOperationCallback) {
    let cbCharacteristic = peripheral.services?.first {  $0.uuid.uuidString == service  }?.characteristics?.first { $0.uuid.uuidString ==  characteristic}
    if cbCharacteristic != nil {
        callback = completion
        peripheral.setNotifyValue(false, for: cbCharacteristic!)
    } else {
        completion(self, CBUUID(string: service).data,  CBUUID(string: characteristic).data,  false)
    }
}

// CBCentralManagerDelegate
func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
    self.delegate?.didUpdate(characteristic.value, forDevice: self,  service: characteristic.service.uuid.data,  characteristic: characteristic.uuid.data)
}

// CBCentralManagerDelegate
func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
    callback(self, characteristic.service.uuid.data, characteristic.uuid.data, error == nil && !characteristic.isNotifying)
}
```

**SBMConnectableDeviceDelegate:**

```Java
//SBMConnectableDevice property
var delegate: SBMConnectableDeviceDelegate?

//CBPeripheralDelegate
func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
    delegate?.didModifyServices(invalidatedServices.map({ $0.uuid.uuidString }), for: self)
}
```

### 3.2 Provision with configuration under one connection

Once the Developer has successfully incorporated all the above changes, he or she can proceed to modify the execution sequence which is required to enable the usage of One Gatt connection.

#### 3.2.1 Execution modification

The User of the ADK needs to make sure that he/she creates a `ProvisionerConfiguration` object, sets the `useOneGattConnection` property to `true` and then adds that object as an argument for the `provision` method.

**Note:** It is also possible to add other initial configuration to that object such as setting the Node as Proxy or Relay.

```Java
let connectableDevice: SBMConnectableDevice
let subnet: SBMSubnet
...

let provisionerConnection = SBMProvisionerConnection(for: connectableDevice, subnet: subnet)
let provisionerConfiguration = SBMProvisionerConfiguration(proxyEnabled: true, nodeIdentityEnabled: true, useOneGattConnection: true, keepOneConnection: false)
    
...
    provisionerConnection.provision(withConfiguration: provisionerConfiguration, parameters: nil,   retryCount: 3) { (connection, node, error) in
        //Handle result of provisioning
    }
```

## 4. Keeping connection after the provisioning

Apart from the feature described above, in the 2.3.2 version of the ADK we have also introduced a possibility of keeping the Proxy connection after the successful provisioning. Enabling that might also lead to improvements in performance and reliability of the application.

The following points describe the necessary actions which need to be implemented in the application for each platform.

### 4.1 Android

#### 4.1.1 Execution modification

Firstly, it is necessary to set the keepingProxyConnection to `true` in the `ProvisionerConfiguration` object and add it as an argument for the `provision` method.

**Note:** It is also possible to add other initial configuration to that object such as setting the Node as Proxy or Relay.

```Java
val provisionerConnection: ProvisionerConnection
val provisioningCallback: ProvisioningCallback

...

val provisionerConfiguration = ProvisionerConfiguration().apply {
            keepingProxyConnection = true
}
provisionerConnection.provision(provisionerConfiguration, null, provisioningCallback)
```

#### 4.1.2 Obtaining active Proxy Connection object

When the provisioning session finishes, it is going to be necessary to obtain the still active ProxyConnection to use it for other operations depending on the Developer requirements. It can be done by implementing a following solution into the provisioning callback.

It is important to highlight the importance of replacing the error callback for handling unexpected connection issues such as sudden disconnects. Please make sure you carefully analyse the following code example.

```Java
val provisionerConnection: ProvisionerConnection
val provisioningCallback: ProvisioningCallback
val proxyConnection: ProxyConnection? = null
val errorCallback: ProxyConnection.ErrorCallback
...
provisioningCallback = object: ProvisioningCallback{
    override fun success(device: ConnectableDevice?, subnet: Subnet?, node: Node?) {
        proxyConnection = provisionerConnection.getProxyConnection()
        proxyConnection!!.replaceErrorCallback(errorCallback)
    }
    override fun error(device: ConnectableDevice?, subnet: Subnet?, error: ErrorType?) {
    }
})
```

### 4.2 iOS

#### 4.2.1 Execution modification

It is important to notice that if one wishes to close proxy connection after configuration, `init` method with keepProxyConnection set to false needs to be explicitly used.

```Java
let oldProvisionerConfiguration = SBMProvisionerConfiguration(proxyEnabled: true, nodeIdentityEnabled: true)
let newProvisionerConfiguration = SBMProvisionerConfiguration(proxyEnabled: true, nodeIdentityEnabled: true, useOneGattConnection: true, keepProxyConnection: true)
//both oldProvisionerConfiguration and newProvisionerConfiguration can be used to keep proxy connection open
provisionerConnection.provision(withConfiguration: oldProvisionerConfiguration, parameters: nil, retryCount: 3) { (connection, node, error) in
    //Handle result of provisioning
}
```

#### 4.2.2 Obtaining active Proxy Connection object

When the provisioning session finishes, it is going to be necessary to obtain the still active ProxyConnection to use it for other operations depending on the Developer requirements. It can be done by implementing a following solution into the provisioning callback.

It is important to highlight the importance of replacing the error callback for handling unexpected connection issues such as sudden disconnects. Please make sure you carefully analyse the following code example.

```Java
let errorCallback: (SBMConnectableDevice, Error) -> () = { device, error in
    //handle error
}
let provisionerConfiguration = SBMProvisionerConfiguration(proxyEnabled: true, nodeIdentityEnabled: true, useOneGattConnection: true, keepProxyConnection: true)
var proxyConnetion: SBMProxyConnection? = nil
provisionerConnection.provision(withConfiguration: provisionerConfiguration, parameters: nil, retryCount: 3) { (connection, node, error) in
    proxyConnection = connection.proxyConnection
    proxyConnection.replaceErrorCallback(errorCallback)
}
```

## 5. Conclusion

To sum up, the above guidelines should help the Users to understand how to enable the new described features in their application. Certainly it can bring significant improvements to the performance of their applications and increase reliability as well.
