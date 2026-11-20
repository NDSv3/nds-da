/**
 * @file nds-da-test.cpp
 * @brief NDS-DA Test main entry point
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include "ndsda-test-device.hpp"

using ndsda::Device;
ndsda::Device* ndsda::Device::instance = nullptr;
NDS_DEFINE_DRIVER(DADevice, Device)

int main(int argc, char** argv) {
    nds::Factory factory(std::shared_ptr<nds::FactoryBaseImpl>(new nds::tests::TestControlSystemFactoryImpl()));
    nds::Factory::registerControlSystem(factory);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}