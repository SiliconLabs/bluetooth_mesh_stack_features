   
let nodes = null;
let edges = null;
let provisionerDevice = null;
let meshCustomDataCharacteristicIn = null;

let mesh_custom_data_service = '5e5a13c3-afea-4b3a-9994-a9c6664ef661';
let mesh_custom_data_characteristic_in = '16610a37-2fad-465e-b745-cf2c415186d6';
let mesh_custom_data_characteristic_out = '5c2e435e-4e28-4176-b520-b95d4bf8d3ca';
let fadeOutIntervals = [];

const NotificationType = {
  UNPROVISIONED_DEVICE: 0,
  PROVISION_DEVICE: 1,
  NODE_PROVISIONED: 2,
  HEARTBEAT_REPORT: 3,
  REMAP_NETWORK: 4
};

window.onload = function () {
    nodes = new vis.DataSet([]);
    edges = new vis.DataSet([]);

    const container = document.getElementById("network");
    const data = {nodes, edges};
    const options = {
      nodes: {
        shape: "dot",
        size: 25,
        font: {
          size: 16,
          color: "#000000"
        }
      },
      edges: {
        width: 2,
        color: "#2196F3"
      },
      physics: {
        enabled: true
      }
    };

    const network = new vis.Network(container, data, options);

    network.on("doubleClick", function (params) {
        if (params.nodes.length > 0) {
            const node = nodes.get(params.nodes[0]);
            if(node && !node.provisioned) {
                _provisionNode(node);
            }
        }
    });
};

function showToast(message, duration = 3000) {
    const toast = document.getElementById("toast");
    toast.textContent = message;
    toast.classList.add("show");

    setTimeout(() => {
        toast.classList.remove("show");
    }, duration);
}

function _fadeNodeOut(nodeId) {
    node = nodes.get(nodeId);
    if(node) {
        opacity = node.opacity - 0.01;
        if(opacity <= 0) {
            clearInterval(fadeOutIntervals[nodeId]);
            nodes.remove([{id: nodeId}]);
            return;
        }
        nodes.update([{id: nodeId, opacity: opacity}]);
    }
}

// Read the Provisioner address from the Characteristic and add it to the Map
function _readMeshMapCharacteristicValue(characteristic) {
    return characteristic.readValue().then(value => {
        let provisionerId = '0x';
        for (let i = value.byteLength - 1; i >= 0; i--) {
            provisionerId += value.getUint8(i).toString(16).padStart(2, '0');
        }
        nodes.add({id: provisionerId, label: `Provisioner\n${provisionerId}`, opacity: 1, color: '#4CAF50', address: provisionerId});
    });
}

function _handleMeshProxyDataOutCharacteristicNotification(notification) {
    const value = notification.target.value;
    console.log('Notification received:', value);
}

function _handleMeshCustomDataCharacteristicNotification(notification) {
    const value = notification.target.value;
    //console.log('Notification received:', value);

    switch(value.getUint8(0)) {
        case NotificationType.UNPROVISIONED_DEVICE: {
            let bleAddress =
                value.getUint8(6).toString(16).padStart(2, '0') + ':' +
                value.getUint8(5).toString(16).padStart(2, '0') + ':' +
                value.getUint8(4).toString(16).padStart(2, '0') + ':' +
                value.getUint8(3).toString(16).padStart(2, '0') + ':' +
                value.getUint8(2).toString(16).padStart(2, '0') + ':' +
                value.getUint8(1).toString(16).padStart(2, '0');
            let bleUUID =
                value.getUint8(7).toString(16).padStart(2, '0') +
                value.getUint8(8).toString(16).padStart(2, '0') +
                value.getUint8(9).toString(16).padStart(2, '0') +
                value.getUint8(10).toString(16).padStart(2, '0') +
                value.getUint8(11).toString(16).padStart(2, '0') +
                value.getUint8(12).toString(16).padStart(2, '0') +
                value.getUint8(13).toString(16).padStart(2, '0') +
                value.getUint8(14).toString(16).padStart(2, '0') +
                value.getUint8(15).toString(16).padStart(2, '0') +
                value.getUint8(16).toString(16).padStart(2, '0') +
                value.getUint8(17).toString(16).padStart(2, '0') +
                value.getUint8(18).toString(16).padStart(2, '0') +
                value.getUint8(19).toString(16).padStart(2, '0') +
                value.getUint8(20).toString(16).padStart(2, '0') +
                value.getUint8(21).toString(16).padStart(2, '0') +
                value.getUint8(22).toString(16).padStart(2, '0');
            let bleRSSI = value.getInt8(23).toString();

            if(!nodes.get(bleUUID)) {
                nodes.add({id: bleUUID, label: `${bleAddress}\nRSSI: ${bleRSSI} dBm`, opacity: 1, color: '#9E9E9E', provisioned: false, data: value});
                fadeOutIntervals[bleUUID] = setInterval(_fadeNodeOut, 100, bleUUID);
            } else nodes.update([{id: bleUUID, label: `${bleAddress}\nRSSI: ${bleRSSI} dBm`, opacity: 1}]);
        } break;
        case NotificationType.NODE_PROVISIONED: {
            let meshAddress = '0x' +
                value.getUint8(2).toString(16).padStart(2, '0') +
                value.getUint8(1).toString(16).padStart(2, '0');
            let bleUUID =
                value.getUint8(3).toString(16).padStart(2, '0') +
                value.getUint8(4).toString(16).padStart(2, '0') +
                value.getUint8(5).toString(16).padStart(2, '0') +
                value.getUint8(6).toString(16).padStart(2, '0') +
                value.getUint8(7).toString(16).padStart(2, '0') +
                value.getUint8(8).toString(16).padStart(2, '0') +
                value.getUint8(9).toString(16).padStart(2, '0') +
                value.getUint8(10).toString(16).padStart(2, '0') +
                value.getUint8(11).toString(16).padStart(2, '0') +
                value.getUint8(12).toString(16).padStart(2, '0') +
                value.getUint8(13).toString(16).padStart(2, '0') +
                value.getUint8(14).toString(16).padStart(2, '0') +
                value.getUint8(15).toString(16).padStart(2, '0') +
                value.getUint8(16).toString(16).padStart(2, '0') +
                value.getUint8(17).toString(16).padStart(2, '0') +
                value.getUint8(18).toString(16).padStart(2, '0');

            if(!nodes.get(bleUUID)) {
                nodes.add({id: bleUUID, label: `Node\n${meshAddress}`, opacity: 1, color: '#2196F3', address: meshAddress, provisioned: true, data: value});
            } else {
                nodes.update([{id: bleUUID, label: `Node\n${meshAddress}`, opacity: 1, color: '#2196F3', address: meshAddress, provisioned: true}]);
            }
            if(fadeOutIntervals[bleUUID]) clearInterval(fadeOutIntervals[bleUUID]);

            showToast('Provisioning succeeded');

        } break;
        case NotificationType.HEARTBEAT_REPORT: {
            let fromAddress = '0x' +
                value.getUint8(2).toString(16).padStart(2, '0') +
                value.getUint8(1).toString(16).padStart(2, '0');
            let toAddress = '0x' +
                value.getUint8(4).toString(16).padStart(2, '0') +
                value.getUint8(3).toString(16).padStart(2, '0');
            let minHop = value.getInt8(5).toString();
            let maxHop = value.getInt8(6).toString();

            let fromNode = nodes.get({
                filter: function (item) {
                    return item.address === fromAddress;
                }
            })[0];

            let toNode = nodes.get({
                filter: function (item) {
                    return item.address === toAddress;
                }
            })[0];

            if(!edges.get(`${fromNode.id}-${toNode.id}`) || !edges.get(`${toNode.id}-${fromNode.id}`)) {
                edges.add({id: `${fromNode.id}-${toNode.id}`, from: fromNode.id, to: toNode.id, label: maxHop});
            }

        } break;

    }
}

function _provisionNode(node) {
    if(meshCustomDataCharacteristicIn) {
        showToast(`Provisioning device: ${node.label}`);
        meshCustomDataCharacteristicIn.writeValueWithoutResponse (
            new Uint8Array([
                NotificationType.PROVISION_DEVICE,
                ...new Uint8Array(node.data.buffer, node.data.byteOffset + 7, 16)])
        )
        .catch(error => {
            console.log(error);
        });
    }
}

function _remapNetwork() {
    if(meshCustomDataCharacteristicIn) {
        showToast(`Remapping the Network`);
        edges.clear();
        meshCustomDataCharacteristicIn.writeValueWithoutResponse (
            new Uint8Array([NotificationType.REMAP_NETWORK])
        )
        .catch(error => {
            console.log(error);
        });
    }
    
}

function _onProvisionerDisconnected() {
    showToast('Provisioner disconnected');
    for(let fadeOutInterval in fadeOutIntervals) clearInterval(fadeOutIntervals[fadeOutInterval]);
    fadeOutIntervals = [];
    nodes.clear();
    edges.clear();
}

function _connectProvisioner() {
    // Check if Bluetooth is available
    navigator.bluetooth.getAvailability()
    .then(available => {
        if (!available) {
            console.log('Bluetooth is not available on this device.');
            return;
        }

        let BLEoptions = {
            filters: [{namePrefix: 'provisioner'}],
            optionalServices: [mesh_custom_data_service]
        };

        navigator.bluetooth.requestDevice(BLEoptions)
        .then(device => {
            console.log('Name - ' + device.name);
            console.log('ID - ' + device.id);

            device.addEventListener('gattserverdisconnected', _onProvisionerDisconnected);
            provisionerDevice = device;

            return device.gatt.connect();
        })
        .then(server => {
            showToast('Provisioner connected');
            console.log('Getting GAP Service...');
            return server.getPrimaryService(mesh_custom_data_service);
        })
        .then(service => {
            console.log('Getting GAP Characteristics...');
            return service.getCharacteristics();
        })
        .then(characteristics => {
            characteristics.forEach(characteristic => {
                switch (characteristic.uuid) {
                    case BluetoothUUID.getCharacteristic(mesh_custom_data_characteristic_out): {
                        // Read the Mesh Map Characteristic Value and add the Provisioner to the Map
                        _readMeshMapCharacteristicValue(characteristic);
                        // Handle the heartbeat generated notifications and add/update Nodes on the Map
                        characteristic.startNotifications();
                        characteristic.addEventListener('characteristicvaluechanged', _handleMeshCustomDataCharacteristicNotification);
                    } break;
                    case BluetoothUUID.getCharacteristic(mesh_custom_data_characteristic_in): {
                        meshCustomDataCharacteristicIn = characteristic;
                    } break;
                    default: console.log('Unknown Characteristic - ' + characteristic.uuid);
                }
            });
            

        })
        .catch(error => {
            console.log(error);
        });
    })
    .catch(error => {
        console.log(error);
    });
}

function _disconnectProvisioner() {
    if(provisionerDevice && provisionerDevice.gatt.connected) {
        meshCustomDataCharacteristicIn = null;
        provisionerDevice.gatt.disconnect();
        provisionerDevice = null;
    }
}