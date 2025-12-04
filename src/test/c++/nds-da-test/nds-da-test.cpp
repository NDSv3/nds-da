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

TEST(NDS_DA_Tests, TestMissingDeviceMapException) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DAMemoryMapFile", "memoryMap.txt");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(NDS_DA_Tests, TestMissingMemoryMapException) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DADeviceMapFile", "memoryMap.txt");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(NDS_DA_Tests, DeviceCreationTest) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DAMemoryMapFile", "memoryMap.txt");
    params.emplace("DADeviceMapFile", "memoryMap.txt");

    factory.createDevice("DADevice", "deviceAccessDevice", params);
    factory.destroyDevice("deviceAccessDevice");
}