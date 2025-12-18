#ifndef NDS_DA_TEST_FIXTURE
#define NDS_DA_TEST_FIXTURE

#include <gtest/gtest.h>
#include <string>
#include "nds3/factory.h"
#include "nds3/ndsTestInterface.h"
#include "ndsdaBridge.hpp"
#include "ndsdaFileParser.hpp"

#define NDSDA_TEST_DEVICEALIAS "test_device"
#define NDSDA_TEST_DEVICE_MAPFILE "../src/test/test-devices.dmap"
#define NDSDA_TEST_MEMORY_MAPFILE "../src/test/test-memory.mmap"

// WARNING: MUST match the content of MEMORY_MAPFILE with DEVICE_ALIAS
static const std::vector<std::shared_ptr<ndsda::MemoryElement>> expectedElements = {
    std::make_shared<ndsda::OneDElement>(".BASE", 4, 32, 0, 0),
    std::make_shared<ndsda::RegisterElement>("BASE.FIRST_REG", 0, 0),
    std::make_shared<ndsda::RegisterElement>("BASE.SECOND_REG", 0, 1),
    std::make_shared<ndsda::RegisterElement>("BASE.THIRD_REG", 0, 0),
    std::make_shared<ndsda::RegisterElement>("BASE.ONE_BIT_REG", 0, 0),
    std::make_shared<ndsda::OneDElement>(".DMA_SAMPLES", 200, 32, 0, 0)
};

class ndsDABasicDeviceFixture : public testing::Test {
protected:
    const std::string DEVICE_ALIAS   = NDSDA_TEST_DEVICEALIAS;
    const std::string DEVICE_MAPFILE = NDSDA_TEST_DEVICE_MAPFILE;
    const std::string MEMORY_MAPFILE = NDSDA_TEST_MEMORY_MAPFILE;

    nds::Factory factory;
    nds::namedParameters_t params;
    nds::tests::TestControlSystemInterfaceImpl* iface;

    ndsDABasicDeviceFixture() : factory("test") {
        params.emplace("DeviceAlias", DEVICE_ALIAS);
        params.emplace("DeviceMapFile", DEVICE_MAPFILE);
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

class ndsDAFileParserFixture : public testing::Test {
protected:
    const std::string DEVICE_ALIAS   = NDSDA_TEST_DEVICEALIAS;
    const std::string DEVICE_MAPFILE = NDSDA_TEST_DEVICE_MAPFILE;
    const std::string MEMORY_MAPFILE = NDSDA_TEST_MEMORY_MAPFILE;

    std::unique_ptr<ndsda::FileParser> parser;

    void SetUp() override {
        parser = std::make_unique<ndsda::FileParser>(DEVICE_MAPFILE, DEVICE_ALIAS);
    }

    void TearDown() override {}
};

class ndsDABridgeFixture : public testing::Test {
protected:
    const std::string DEVICE_ALIAS   = NDSDA_TEST_DEVICEALIAS;
    const std::string DEVICE_MAPFILE = NDSDA_TEST_DEVICE_MAPFILE;
    const std::string MEMORY_MAPFILE = NDSDA_TEST_MEMORY_MAPFILE;

    const std::string DAPV_REGISTER_PV        = "BASE.FIRST_REG";
    const std::string DAPV_REGISTER_SIGNED_PV = "BASE.SECOND_REG";
    const std::string DAPV_ONED_PV            = ".DMA_SAMPLES";
    const size_t DAPV_ONED_ELEMENTS           = 200;

    std::unique_ptr<ndsda::DABridge> bridge;

    void SetUp() override {
        bridge = std::make_unique<ndsda::DABridge>(DEVICE_MAPFILE, DEVICE_ALIAS, expectedElements);
    }

    void TearDown() override {}
};
#endif  //! NDS_DA_TEST_FIXTURE
