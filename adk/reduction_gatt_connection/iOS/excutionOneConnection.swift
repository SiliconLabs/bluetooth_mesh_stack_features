let connectableDevice: SBMConnectableDevice
let subnet: SBMSubnet
...

let provisionerConnection = SBMProvisionerConnection(for: connectableDevice, subnet: subnet)
let provisionerConfiguration = SBMProvisionerConfiguration(proxyEnabled: true, nodeIdentityEnabled: true, useOneGattConnection: true)
    
...
    provisionerConnection.provision(withConfiguration: provisionerConfiguration, parameters: nil,   retryCount: 3) { (connection, node, error) in
        //Handle result of provisioning
    }
