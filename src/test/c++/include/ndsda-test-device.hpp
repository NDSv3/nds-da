#ifndef NDSDA_TEST_DEVICE_HPP
#define NDSDA_TEST_DEVICE_HPP

#include "ndsdaDevice.hpp"

namespace ndsda {

class Device : ndsda::ndsdaDevice {
public:
    Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
        : ndsda::ndsdaDevice(factory, deviceName, params) {}
};

}  // namespace ndsda

#endif