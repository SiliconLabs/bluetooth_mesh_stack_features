//SBMConnectableDevice property
var delegate: SBMConnectableDeviceDelegate?

//CBPeripheralDelegate
func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
    delegate?delegate?.didModifyServices(invalidatedServices.map({ $0.uuid.uuidString }), for: self)
}
