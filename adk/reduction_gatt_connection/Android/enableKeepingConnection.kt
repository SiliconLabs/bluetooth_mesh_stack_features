val provisionerConnection: ProvisionerConnection
val provisioningCallback: ProvisioningCallback

...

val provisionerConfiguration = ProvisionerConfiguration().apply {
            keepingProxyConnection = true
}
provisionerConnection.provision(provisionerConfiguration, null, provisioningCallback)
