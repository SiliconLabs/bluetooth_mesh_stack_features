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
