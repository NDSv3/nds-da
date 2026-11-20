.. SPDX-License-Identifier: Apache-2.0
.. Copyright 2026 I2A2-UPM

Driver implementation
=====================

To implement a driver using NDS-DeviceAccess, the main task is to create a class that inherits from :cpp:class:`ndsda::ndsdaDevice`, which . The base class automatically parses all required files and generates the defined PVs for DA communication. Additional PVs and Nodes can be instantiated and added to the device as needed.

The required configuration parameters for the device are:

* ``DeviceAlias``: Alias of the device in the device map file
* ``DeviceMapFile``: Path to the device map file (.dmap)
* ``NodeMapFile``: Path to the node map file (.xml)

Additional parameters can be added as needed, these are the requirements for the base class. On initialization, the device will parse the device map to find the memory map file, then parse the memory map and node map files to create the appropriate PVs and register them in the NDS tree.

Minimal example
---------------

..  code-block:: cpp
    :caption: minimal_device.cpp

    #include "ndsdaDevice.hpp"

    class Device : ndsda::ndsdaDevice {
    public:
        Device(nds::Factory& factory, const std::string& deviceName,
            const nds::namedParameters_t& params)
        : ndsda::ndsdaDevice(factory, deviceName, params) {

            /* Add PVs or Nodes */

            rootNode.initialize(this, factory);
        }
    };