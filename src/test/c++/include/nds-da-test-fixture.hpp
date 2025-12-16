#ifndef NDS_DA_TEST_FIXTURE
#define NDS_DA_TEST_FIXTURE

#include <gtest/gtest.h>
#include <string>
#include "nds3/factory.h"
#include "nds3/ndsTestInterface.h"
#include "ndsdaFileParser.hpp"

#define NDSDA_TEST_DEVICEALIAS "test_device"
#define NDSDA_TEST_DEVICE_MAPFILE "../src/test/test-devices.dmap"
#define NDSDA_TEST_MEMORY_MAPFILE "../src/test/test-memory.mmap"

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

    ndsDAFileParserFixture() {}
    // ~ndsDAFileParserFixture() override {}

    void SetUp() override {
        parser = std::make_unique<ndsda::FileParser>(DEVICE_MAPFILE, DEVICE_ALIAS);
    }

    void TearDown() override {}
};

#endif  //! NDS_DA_TEST_FIXTURE
