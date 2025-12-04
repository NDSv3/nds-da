#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include "nds3/ndsTestInterface.h"

int main(int argc, char** argv) {
    nds::Factory factory(std::shared_ptr<nds::FactoryBaseImpl>(new nds::tests::TestControlSystemFactoryImpl()));
    // factory.createDevice("NDS-DeviceAccess", "deviceAccessTestDevice", nds::namedParameters_t());
    nds::Factory::registerControlSystem(factory);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(NDS_DA_Tests, DeviceCreationTest) {
    nds::Factory factory("test");
    nds::namedParameters_t params;
    int value;
    timespec timestamp;

    params.emplace("param1", "value1");

    factory.createDevice("DADevice", "deviceAccessDevice", params);

    nds::tests::TestControlSystemInterfaceImpl* test_iface = nds::tests::TestControlSystemInterfaceImpl::getInstance("deviceAccessDevice");

    test_iface->readCSValue("/deviceAccessDevice-testPV", &timestamp, &value);
    EXPECT_EQ(value, 1e5);

    factory.destroyDevice("deviceAccessDevice");
}