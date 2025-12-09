#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include "nds-da-test-fixture.hpp"

int main(int argc, char** argv) {
    nds::Factory factory(std::shared_ptr<nds::FactoryBaseImpl>(new nds::tests::TestControlSystemFactoryImpl()));
    nds::Factory::registerControlSystem(factory);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(NDS_DA_Tests, TestMissingDeviceMapException) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("MemoryMapFile", "memoryMap.txt");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(NDS_DA_Tests, TestMissingMemoryMapException) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceMapFile", "memoryMap.txt");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(NDS_DA_Tests, DeviceCreationTest) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceAlias", "deviceAlias");
    params.emplace("DeviceMapFile", "memoryMap.txt");

    factory.createDevice("DADevice", "deviceAccessDevice", params);
    factory.destroyDevice("deviceAccessDevice");
}

TEST_F(ndsDABasicDeviceFixture, GetDeviceMapPVs) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-deviceAlias", &tmp_time, &tmp_string);
    EXPECT_NE(tmp_time.tv_sec, 0);
    EXPECT_EQ(tmp_string, DEVICE_ALIAS);

    iface->readCSValue("/deviceAccessDevice-deviceMapFile", &tmp_time, &tmp_string);
    EXPECT_NE(tmp_time.tv_sec, 0);
    EXPECT_EQ(tmp_string, DEVICE_MAPFILE);
}