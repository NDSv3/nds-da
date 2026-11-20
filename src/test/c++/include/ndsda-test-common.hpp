/**
 * @file ndsda-test-common.hpp
 * @brief NDS-DA Test fixtures and common test utilities
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef NDS_DA_TEST_FIXTURE
#define NDS_DA_TEST_FIXTURE

#include <gtest/gtest.h>
#include <string>
#include "nds3/factory.h"
#include "nds3/ndsTestInterface.h"
#include "ndsdaBridge.hpp"

#include "ndsda-test-constants.hpp"

class ndsDABasicDeviceFixture : public testing::Test {
protected:
    nds::Factory factory;
    nds::namedParameters_t params;
    nds::tests::TestControlSystemInterfaceImpl* iface;

    ndsDABasicDeviceFixture() : factory("test"), iface(nullptr) {
        params.emplace("DeviceAlias", NDSDA_TEST_DEVICEALIAS);
        params.emplace("DeviceMapFile", NDSDA_TEST_DMAPFILE);
        params.emplace("NodeMapFile", NDSDA_TEST_NODEMAPFILE);
    }
    // ~ndsDABasicDeviceFixture() override {}

    void SetUp() override {
        factory.createDevice("DADevice", "deviceAccessDevice", params);
        iface = nds::tests::TestControlSystemInterfaceImpl::getInstance("deviceAccessDevice");
        ASSERT_NE(iface, nullptr);
    }
    void TearDown() override {
        factory.destroyDevice("deviceAccessDevice");
    }
};

class ndsDABridgeFixture : public testing::Test {
protected:
    ndsda::DABridge bridge{std::make_unique<ndsda::DAAccessor>(NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS)};
};

class ndsDAAccessorFixture : public testing::Test {
protected:
    ndsda::DAAccessor accessor{NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS};
};

static auto randomSigned(auto max) { return 2 * ((std::rand() % max) - max / 2); }
static auto randomUnsigned(auto max) { return std::rand() % max; }
static float randomFloat() { return 2.0f * static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 1.0f; }

#endif  //! NDS_DA_TEST_FIXTURE
