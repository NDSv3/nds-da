#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include "nds-da-test-fixture.hpp"

TEST(DeviceTests, MissingDeviceMap_Exception) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("MemoryMapFile", "memoryMap.txt");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(DeviceTests, MissingMemoryMap_Exception) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceMapFile", "memoryMap.txt");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(DeviceTests, DeviceNotInMap_Exception) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceAlias", "NonExistingDevice");
    params.emplace("DeviceMapFile", NDSDA_TEST_DEVICE_MAPFILE);

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(DeviceTests, SuccessfulDeviceCreation) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceAlias", NDSDA_TEST_DEVICEALIAS);
    params.emplace("DeviceMapFile", NDSDA_TEST_DEVICE_MAPFILE);

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

TEST_F(ndsDABasicDeviceFixture, GetMemoryMapFile) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-memoryMapFile", &tmp_time, &tmp_string);
    EXPECT_NE(tmp_time.tv_sec, 0);
    EXPECT_EQ(tmp_string, MEMORY_MAPFILE);
}