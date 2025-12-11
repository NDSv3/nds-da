#include "ndsdaMain.hpp"
#include <ctime>
#include <functional>
#include <memory>
#include "nds3/definitions.h"
#include "nds3/exceptions.h"

#define BIND_DELEGATEIN(func) (std::bind(&func, this, std::placeholders::_1, std::placeholders::_2))

using ndsda::Device;

Device::Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
    : deviceName(deviceName),
      rootNode(deviceName),
      deviceAlias("deviceAlias"),
      deviceMapFile("deviceMapFile"),
      memoryMapFile("memoryMapFile", [this](timespec* ts, std::string* value) {
          clock_gettime(CLOCK_REALTIME, ts);
          *value = this->fileParser->getMemoryMapFile();
      }) {

    std::string deviceMapFileStr, deviceAliasStr;
    try {
        deviceMapFileStr = params.at("DeviceMapFile");
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DADeviceMapFile" << std::endl;
        throw nds::NdsError("Missing DeviceMapFile parameter");
    }

    try {
        deviceAliasStr = params.at("DeviceAlias");
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DAMemoryMapFile" << std::endl;
        throw nds::NdsError("Missing MemoryMapFile parameter");
    }

    fileParser = std::make_unique<FileParser>(deviceMapFileStr, deviceAliasStr);

    deviceMapFile.setValue(deviceMapFileStr);
    deviceAlias.setValue(deviceAliasStr);

    rootNode.addChild(deviceAlias);
    rootNode.addChild(deviceMapFile);
    rootNode.addChild(memoryMapFile);

    rootNode.initialize(this, factory);
}

Device::~Device() {}

NDS_DEFINE_DRIVER(DADevice, Device)
