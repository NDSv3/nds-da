#ifndef NDS_DA_TEST_FIXTURE
#define NDS_DA_TEST_FIXTURE

#include <gtest/gtest.h>
#include "nds3/factory.h"
#include "nds3/ndsTestInterface.h"

class ndsDABasicDeviceFixture : public testing::Test {
protected:
    const std::string DEVICE_ALIAS   = "deviceAlias";
    const std::string DEVICE_MAPFILE = "deviceMap.txt";

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

#endif  //! NDS_DA_TEST_FIXTURE
