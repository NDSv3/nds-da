#include "deviceAccessMain.hpp"
#include "nds3/definitions.h"
#include "nds3/exceptions.h"

DADevice::DADevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
    : deviceName(deviceName),
      rootNode(deviceName),
      deviceAlias("deviceAlias"),
      deviceMapFile("deviceMapFile") {

    // Obtain device and memory map files
    try {
        deviceMapFile.setValue(params.at("DADeviceMapFile"));
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DADeviceMapFile" << std::endl;
        throw nds::NdsError("Missing DADeviceMapFile parameter");
    }

    try {
        deviceAlias.setValue(params.at("DADeviceAlias"));
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DAMemoryMapFile" << std::endl;
        throw nds::NdsError("Missing DAMemoryMapFile parameter");
    }

    rootNode.addChild(deviceAlias);
    rootNode.addChild(deviceMapFile);

    rootNode.initialize(this, factory);
}

DADevice::~DADevice() {}

NDS_DEFINE_DRIVER(DADevice, DADevice)