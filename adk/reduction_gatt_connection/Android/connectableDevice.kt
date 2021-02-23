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
