/**
 * @file ndsda-test-device.hpp
 * @brief NDS-DA Test device wrapper
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
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