/*
 * Copyright © 2020 Silicon Labs, http://www.silabs.com. All rights reserved.
 */

package com.siliconlabs.bluetoothmesh.App.Models

import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Handler
import android.os.Looper
import android.os.ParcelUuid
import android.util.Log
import com.siliconlab.bluetoothmesh.adk.connectable_device.*
import com.siliconlabs.bluetoothmesh.App.Logic.BluetoothScanner
import java.lang.reflect.Method
import java.util.*
import java.util.concurrent.TimeUnit

class BluetoothConnectableDevice(
        private val context: Context,
        private var scanResult: ScanResult,
        val bluetoothScanner: BluetoothScanner
) : ConnectableDevice() {

    private val TAG = BluetoothConnectableDevice::class.java.simpleName + "@" + hashCode()

    var deviceConnectionCallbacks = mutableSetOf<DeviceConnectionCallback>()
    var mainHandler = Handler(Looper.getMainLooper())
    var bluetoothGatt: BluetoothGatt? = null
    lateinit var bluetoothDevice: BluetoothDevice
    private var mtuSize = 0
    lateinit var address: String
        private set
    private var advertisementData: ByteArray? = null
    var connecting = false
    private lateinit var bluetoothGattCallback: BluetoothGattCallback
    lateinit var scanCallback: ScanCallback
    private var refreshBluetoothDeviceCallback: RefreshBluetoothDeviceCallback? = null
    private lateinit var refreshBluetoothDeviceTimeoutRunnable: Runnable
    private var refreshGattServicesCallback: RefreshGattServicesCallback? = null

    fun initRefreshBluetoothDeviceTimeoutRunnable() {
        refreshBluetoothDeviceTimeoutRunnable = Runnable {
            refreshingBluetoothDeviceTimeout()
        }
    }

    fun initScanCallback() {
        scanCallback = object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult?) {
                Log.d(TAG, result.toString())

                result?.let {
                    if (it.device.address == address) {
                        processDeviceFound(result)
                    }
                }
            }

            fun processDeviceFound(result: ScanResult) {
                stopScan()
                mainHandler.removeCallbacks(refreshBluetoothDeviceTimeoutRunnable)
                processScanResult(result)

                // workaround to 133 gatt issue
                // https://github.com/googlesamples/android-BluetoothLeGatt/issues/44
                mainHandler.postDelayed({ refreshBluetoothDeviceCallback?.success() }, 500)
            }
        }
    }

    fun discoverServices(bluetoothGatt: BluetoothGatt) {
        Log.d(TAG, "discoverServices")
        for (i in 0..2) {
            if (bluetoothGatt.discoverServices()) {
                return
            }
            Thread.sleep(50)
            Log.d(TAG, "retry discover services i: $i")
        }
        disconnect()
    }

    fun initBluetoothGattCallback() {
        bluetoothGattCallback = object : BluetoothGattCallback() {
            private var connectionAttempts = 0
            private var changeMtuAttempts = 0
            private var discoverServicesAttempts = 0

            private fun changeMtu(bluetoothGatt: BluetoothGatt) {
                Log.d(TAG, "changeMtu")
                for (i in 0..2) {
                    if (bluetoothGatt.requestMtu(512)) {
                        return
                    }
                    Thread.sleep(50)
                    Log.d(TAG, "retry request mtu i: $i")
                }
                discoverServices(bluetoothGatt)
            }

            override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                super.onConnectionStateChange(gatt, status, newState)
                mainHandler.post {
                    Log.d(TAG, "onConnectionStateChange : status: $status, newState: $newState")
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        onConnectionStateChangeSuccess(gatt, newState)
                    } else {
                        onConnectionStateChangeFail(gatt)
                    }
                }
            }

            private fun onConnectionStateChangeSuccess(gatt: BluetoothGatt, newState: Int) {
                connectionAttempts = 0

                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    changeMtu(gatt)
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED || newState == BluetoothProfile.STATE_DISCONNECTING) {
                    disconnect()
                }
            }

            private fun onConnectionStateChangeFail(gatt: BluetoothGatt) {
                connecting = false
                gatt.close()

                if (!isConnected) {
                    onConnectionAttemptFail()
                } else {
                    onConnectionFail()
                }
            }

            private fun onConnectionAttemptFail() {
                Log.d(TAG, "connect connectionAttempts=$connectionAttempts")
                connectionAttempts++
                nextAttempt()
            }

            private fun nextAttempt() {
                Log.d(TAG, "nextAttempt: connectionAttempts: $connectionAttempts")
                if (connectionAttempts <= 3) {
                    //workaround to 133 gatt issue
                    connect()
                } else {
                    onConnectionError()
                    connectionAttempts = 0
                }
            }

            private fun onConnectionFail() {
                connectionAttempts = 0

                onConnectionError()
                notifyConnectionState(false)
            }

            override fun onMtuChanged(gatt: BluetoothGatt, mtu: Int, status: Int) {
                super.onMtuChanged(gatt, mtu, status)
                mainHandler.post {
                    Log.d(TAG, "onMtuChanged : status $status, mtu: $mtu")
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        onMtuChangedSuccess(gatt, mtu)
                    } else {
                        onMtuChangedFail(gatt)
                    }
                }
            }

            private fun onMtuChangedSuccess(gatt: BluetoothGatt, mtu: Int) {
                changeMtuAttempts = 0
                mtuSize = mtu
                discoverServices(gatt)
            }

            private fun onMtuChangedFail(gatt: BluetoothGatt) {
                if (++changeMtuAttempts < 3) {
                    changeMtu(gatt)
                } else {
                    discoverServices(gatt)
                }
            }

            override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                super.onServicesDiscovered(gatt, status)
                mainHandler.post {
                    Log.d(TAG, "onServicesDiscovered: status: $status")
                    if (status == BluetoothGatt.GATT_SUCCESS) {
                        onServicesDiscoveredSuccess()
                    } else {
                        onServicesDiscoveredFail(gatt)
                    }
                }
            }

            private fun onServicesDiscoveredSuccess() {
                discoverServicesAttempts = 0
                if (connecting) {
                    //notify that device is connected only if it is connecting attempt
                    notifyDeviceConnected()
                }
                refreshGattServicesCallback?.onSuccess()
                refreshGattServicesCallback = null
            }

            private fun onServicesDiscoveredFail(gatt: BluetoothGatt) {
                if (++discoverServicesAttempts < 3) {
                    discoverServices(gatt)
                } else {
                    disconnect()
                }
            }

            override fun onCharacteristicChanged(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic) {
                Log.d(TAG, "onCharacteristicChanged : bluetoothGattCharacteristic: ${characteristic.uuid}")
                updateData(characteristic.service.uuid, characteristic.uuid, characteristic.value)
            }
        }
    }

    init {
        processScanResult(scanResult)
        initScanCallback()
        initBluetoothGattCallback()
        initRefreshBluetoothDeviceTimeoutRunnable()
    }

    private fun processScanResult(scanResult: ScanResult) {
        this.bluetoothDevice = scanResult.device
        this.advertisementData = scanResult.scanRecord!!.bytes
        this.address = bluetoothDevice.address
        this.scanResult = scanResult
    }

    fun addDeviceConnectionCallback(deviceConnectionCallback: DeviceConnectionCallback) {
        synchronized(deviceConnectionCallbacks) {
            deviceConnectionCallbacks.add(deviceConnectionCallback)
        }
    }

    fun removeDeviceConnectionCallback(deviceConnectionCallback: DeviceConnectionCallback) {
        synchronized(deviceConnectionCallbacks) {
            deviceConnectionCallbacks.remove(deviceConnectionCallback)
        }
    }

    fun notifyConnectionState(connected: Boolean) {
        synchronized(deviceConnectionCallbacks) {
            for (callback in deviceConnectionCallbacks) {
                notifyConnectionState(callback, connected)
            }
        }
    }

    private fun notifyConnectionState(callback: DeviceConnectionCallback, connected: Boolean) {
        if (connected) {
            callback.onConnectedToDevice()
        } else {
            callback.onDisconnectedFromDevice()
        }
    }

    override fun getName(): String? {
        return bluetoothDevice.name
    }

    override fun refreshBluetoothDevice(callback: RefreshBluetoothDeviceCallback) {
        if (startScan()) {
            Log.d(TAG, "refreshBluetoothDevice: starting scan succeeded")
            onScanStarted(callback)
        } else {
            Log.d(TAG, "refreshBluetoothDevice: starting scan failed")
            callback.failure()
        }
    }

    private fun onScanStarted(callback: RefreshBluetoothDeviceCallback) {
        refreshBluetoothDeviceCallback = callback
        mainHandler.removeCallbacks(refreshBluetoothDeviceTimeoutRunnable)
        mainHandler.postDelayed(refreshBluetoothDeviceTimeoutRunnable, 10000L)
    }

    private fun refreshingBluetoothDeviceTimeout() {
        Log.d(TAG, "refreshingBluetoothDeviceTimeout")

        mainHandler.removeCallbacks(refreshBluetoothDeviceTimeoutRunnable)
        stopScan()
        refreshBluetoothDeviceCallback?.failure()
        refreshBluetoothDeviceCallback = null
    }

    override fun connect() {
        Log.d(TAG, "connect mac: $address")
        connectGatt()
        setupConnectionTimeout(bluetoothGatt!!)
    }

    private fun startScan(): Boolean {
        bluetoothScanner.addScanCallback(scanCallback)
        return bluetoothScanner.startLeScan(null)
    }

    private fun stopScan() {
        bluetoothScanner.removeScanCallback(scanCallback)
        bluetoothScanner.stopLeScan()
    }

    private fun connectGatt() {
        checkMainThread()
        bluetoothGatt = bluetoothDevice.connectGatt(context, false, bluetoothGattCallback, BluetoothDevice.TRANSPORT_LE)
        bluetoothGatt!!.requestConnectionPriority(BluetoothGatt.CONNECTION_PRIORITY_HIGH)
    }

    private fun setupConnectionTimeout(bluetoothGattLast: BluetoothGatt) {
        connecting = true
        mainHandler.postDelayed({
            if (bluetoothGatt == bluetoothGattLast && connecting) {
                Log.d(TAG, "connection timeout mac: $address")
                onConnectionError()
            }
        }, TimeUnit.SECONDS.toMillis(30))
    }

    override fun disconnect() {
        Log.d(TAG, "disconnect mac: $address")
        checkMainThread()

        refreshGattServicesCallback?.onFail()
        refreshGattServicesCallback = null

        connecting = false
        mainHandler.removeCallbacks(refreshBluetoothDeviceTimeoutRunnable)
        closeGatt()
        stopScan()
    }

    private fun closeGatt() {
        refreshDeviceCache()
        bluetoothGatt?.close()
        notifyDeviceDisconnected()
    }

    private fun notifyDeviceConnected() {
        Log.d(TAG, "notifyDeviceConnected: ")
        connecting = false
        onConnected()
        notifyConnectionState(true)
    }

    private fun notifyDeviceDisconnected() {
        // workaround to 22 gatt issue (when try to connect immediately after disconnect)
        mainHandler.postDelayed({
            onDisconnected()
            notifyConnectionState(false)
        }, 500)
    }

    fun refreshDeviceCache(): Boolean {
        var result = false
        try {
            val refreshMethod: Method = bluetoothGatt!!.javaClass.getMethod("refresh")
            result = refreshMethod.invoke(bluetoothGatt, *arrayOfNulls(0)) as? Boolean ?: false
            Log.d(TAG, "refreshDeviceCache $result")
        } catch (localException: Exception) {
            Log.e(TAG, "An exception occured while refreshing device")
        }
        return result
    }

    override fun getAdvertisementData() = advertisementData

    override fun refreshGattServices(refreshGattServicesCallback: RefreshGattServicesCallback) {
        if (refreshDeviceCache()) {
            this.refreshGattServicesCallback = refreshGattServicesCallback
            discoverServices(bluetoothGatt!!)
        } else {
            refreshGattServicesCallback.onFail()
        }
    }

    override fun getMTU(): Int {
        Log.d(TAG, "getMTU $mtuSize")
        return mtuSize
    }

    override fun getServiceData(service: UUID?): ByteArray? {
        return service?.let { scanResult.scanRecord?.getServiceData(ParcelUuid(it)) }
    }

    override fun hasService(service: UUID?): Boolean {
        Log.d(TAG, "hasService $service")

        return if (bluetoothGatt?.services?.isNotEmpty() == true) {
            bluetoothGatt!!.getService(service) != null
        } else {
            scanResult.scanRecord?.serviceUuids?.contains(ParcelUuid(service))
                    ?: false
        }
    }

    override fun writeData(service: UUID?, characteristic: UUID?, data: ByteArray?, connectableDeviceWriteCallback: ConnectableDeviceWriteCallback) {
        checkMainThread()

        try {
            tryToWriteData(service, characteristic, data)
            connectableDeviceWriteCallback.onWrite(service, characteristic)
        } catch (e: Exception) {
            Log.e(TAG, "writeData error: ${e.message}")
            connectableDeviceWriteCallback.onFailed(service, characteristic)
        }
    }

    private fun tryToWriteData(service: UUID?, characteristic: UUID?, data: ByteArray?) {
        val bluetoothGattCharacteristic = getBluetoothGattCharacteristic(service, characteristic)
        setCharacteristicValueAndWriteType(bluetoothGattCharacteristic, data)
        writeCharacteristic(bluetoothGattCharacteristic)
    }

    private fun getBluetoothGattCharacteristic(service: UUID?, characteristic: UUID?): BluetoothGattCharacteristic {
        return bluetoothGatt!!.getService(service)!!.getCharacteristic(characteristic)
    }

    private fun setCharacteristicValueAndWriteType(characteristic: BluetoothGattCharacteristic, data: ByteArray?) {
        characteristic.value = data
        characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
    }

    private fun writeCharacteristic(characteristic: BluetoothGattCharacteristic) {
        if (!bluetoothGatt!!.writeCharacteristic(characteristic)) {
            throw Exception("Writing to characteristic failed")
        }
    }

    override fun subscribe(service: UUID?, characteristic: UUID?, connectableDeviceSubscriptionCallback: ConnectableDeviceSubscriptionCallback) {
        Log.d(TAG, "subscribe service=$service characteristic=$characteristic")
        checkMainThread()

        try {
            Log.d(TAG, "available services=" + bluetoothGatt!!.services?.map { it.uuid })
            tryToSubscribe(service, characteristic)
            connectableDeviceSubscriptionCallback.onSuccess(service, characteristic)
        } catch (e: Exception) {
            e.message?.let { Log.e(TAG, "subscribe error: $it") } ?: e.printStackTrace()
            connectableDeviceSubscriptionCallback.onFail(service, characteristic)
        }
    }

    override fun unsubscribe(service: UUID?, characteristic: UUID?, capableDeviceUnsubscriptionCallback: ConnectableDeviceUnsubscriptionCallback) {
        Log.d(TAG, "unsubscribe service=$service characteristic=$characteristic")
        checkMainThread()

        try {
            Log.d(TAG, "available services=" + bluetoothGatt!!.services?.map { it.uuid })
            tryToUnsubscribe(service, characteristic)
            capableDeviceUnsubscriptionCallback.onSuccess(service, characteristic)
        } catch (e: Exception) {
            e.message?.let { Log.e(TAG, "subscribe error: $it") } ?: e.printStackTrace()
            capableDeviceUnsubscriptionCallback.onFail(service, characteristic)
        }
    }

    private fun tryToSubscribe(service: UUID?, characteristic: UUID?) {
        val bluetoothGattCharacteristic =
                try {
                    getBluetoothGattCharacteristic(service, characteristic)
                } catch (e: NullPointerException) {
                    throw NullPointerException("Service not available")
                }
        setCharacteristicNotification(bluetoothGattCharacteristic, true)
        val bluetoothGattDescriptor = getBluetoothGattDescriptor(bluetoothGattCharacteristic)
                .apply { value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE }
        writeDescriptor(bluetoothGattDescriptor)
    }

    private fun tryToUnsubscribe(service: UUID?, characteristic: UUID?) {
        val bluetoothGattCharacteristic =
                try {
                    getBluetoothGattCharacteristic(service, characteristic)
                } catch (e: NullPointerException) {
                    throw NullPointerException("Service not available")
                }
        setCharacteristicNotification(bluetoothGattCharacteristic, false)
        val bluetoothGattDescriptor = getBluetoothGattDescriptor(bluetoothGattCharacteristic)
                .apply { value = BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE }
        writeDescriptor(bluetoothGattDescriptor)
    }

    private fun setCharacteristicNotification(characteristic: BluetoothGattCharacteristic, enable: Boolean) {
        if (!bluetoothGatt!!.setCharacteristicNotification(characteristic, enable)) {
            throw Exception("Set characteristic notification failed: characteristic=$characteristic enable=$enable")
        }
    }

    private fun getBluetoothGattDescriptor(characteristic: BluetoothGattCharacteristic): BluetoothGattDescriptor {
        return characteristic.descriptors.takeIf { it.size == 1 }?.first()
                ?: throw Exception("Descriptors size (${characteristic.descriptors.size}) different than expected: 1")
    }

    private fun writeDescriptor(descriptor: BluetoothGattDescriptor) {
        if (!bluetoothGatt!!.writeDescriptor(descriptor)) {
            throw Exception("Writing to descriptor failed")
        }
    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as BluetoothConnectableDevice

        return (scanResult == other.scanResult)
    }

    override fun hashCode(): Int {
        return scanResult.hashCode()
    }

    fun checkMainThread() {
        if (Looper.getMainLooper() != Looper.myLooper()) {
            throw RuntimeException("Not on the main thread.")
        }
    }

    interface DeviceConnectionCallback {
        fun onConnectedToDevice()

        fun onDisconnectedFromDevice()
    }
}
