/**
 * @file ndsda-test-device.hpp
 * @brief NDS-DA Test device wrapper
 *
 * @author David Andrino - I2A2 UPM
 * @copyright GNU General Public License v3.0
 * @date 2026-02
 */
#ifndef NDSDA_TEST_DEVICE_HPP
#define NDSDA_TEST_DEVICE_HPP

#include "ndsdaDevice.hpp"

namespace ndsda {

class Device : public ndsda::ndsdaDevice {
public:
    static Device* instance;

    Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
        : ndsda::ndsdaDevice(factory, deviceName, params) {
        rootNode.initialize(this, factory);
        Device::instance = this;
    }
};

}  // namespace ndsda

#endif