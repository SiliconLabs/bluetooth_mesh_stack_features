val provisionerConnection: ProvisionerConnection
val provisioningCallback: ProvisioningCallback

...

val provisionerConfiguration = ProvisionerConfiguration().apply {
            isUsingOneGattConnection = true
}
provisionerConnection.provision(provisionerConfiguration, null, provisioningCallback)
