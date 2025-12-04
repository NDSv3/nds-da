#include "deviceAccessMain.hpp"
#include "nds3/definitions.h"

DADevice::DADevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
    : deviceName(deviceName), rootNode(deviceName), testPV("testPV") {

    testPV.setValue(1e5);

    rootNode.addChild(testPV);
    rootNode.initialize(this, factory);
}

DADevice::~DADevice() {}

NDS_DEFINE_DRIVER(DADevice, DADevice)