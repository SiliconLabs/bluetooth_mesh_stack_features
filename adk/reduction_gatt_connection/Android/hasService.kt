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
