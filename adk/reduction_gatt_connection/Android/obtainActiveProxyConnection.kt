val provisionerConnection: ProvisionerConnection
val provisioningCallback: ProvisioningCallback
val proxyConnection: ProxyConnection? = null

...

provisioningCallback = object: ProvisioningCallback{
    override fun success(device: ConnectableDevice?, subnet: Subnet?, node: Node?) {
        proxyConnection = provisionerConnection.getProxyConnection()
    }
    override fun error(device: ConnectableDevice?, subnet: Subnet?, error: ErrorType?) {
    }
})
