#include "ndsdaMain.hpp"
#include "nds3/definitions.h"
#include "nds3/exceptions.h"

using ndsda::Device;

Device::Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
    : deviceName(deviceName),
      rootNode(deviceName),
      deviceAlias("deviceAlias"),
      deviceMapFile("deviceMapFile") {

    // Obtain device and memory map files
    try {
        deviceMapFile.setValue(params.at("DeviceMapFile"));
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DADeviceMapFile" << std::endl;
        throw nds::NdsError("Missing DeviceMapFile parameter");
    }

    try {
        deviceAlias.setValue(params.at("DeviceAlias"));
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DAMemoryMapFile" << std::endl;
        throw nds::NdsError("Missing MemoryMapFile parameter");
    }

    rootNode.addChild(deviceAlias);
    rootNode.addChild(deviceMapFile);

    rootNode.initialize(this, factory);
}

Device::~Device() {}

NDS_DEFINE_DRIVER(DADevice, Device)
