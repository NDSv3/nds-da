/**
 * @file nds-da-test.cpp
 * @brief NDS-DA Test main entry point
 *
 * @author David Andrino - I2A2 UPM
 * @copyright GNU General Public License v3.0
 * @date 2026-02
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