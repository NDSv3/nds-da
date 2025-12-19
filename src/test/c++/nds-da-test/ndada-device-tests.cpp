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
    params.emplace("DeviceMapFile", NDSDA_TEST_MMAPFILE);

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(DeviceTests, SuccessfulDeviceCreation) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceAlias", NDSDA_TEST_DEVICEALIAS);
    params.emplace("DeviceMapFile", NDSDA_TEST_DMAPFILE);

    factory.createDevice("DADevice", "deviceAccessDevice", params);
    factory.destroyDevice("deviceAccessDevice");
}

TEST_F(ndsDABasicDeviceFixture, GetDeviceMapPVs) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-deviceAlias", &tmp_time, &tmp_string);
    EXPECT_NE(tmp_time.tv_sec, 0);
    EXPECT_EQ(tmp_string, NDSDA_TEST_DEVICEALIAS);

    iface->readCSValue("/deviceAccessDevice-deviceMapFile", &tmp_time, &tmp_string);
    EXPECT_NE(tmp_time.tv_sec, 0);
    EXPECT_EQ(tmp_string, NDSDA_TEST_DMAPFILE);
}

TEST_F(ndsDABasicDeviceFixture, GetMemoryMapFile) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-memoryMapFile", &tmp_time, &tmp_string);
    EXPECT_NE(tmp_time.tv_sec, 0);
    EXPECT_EQ(tmp_string, NDSDA_TEST_MMAPFILE);
}

TEST_F(ndsDABasicDeviceFixture, WriteReadPVInt32) {
    int32_t write_value = 12345678;
    int32_t read_value  = 0;
    timespec tmp_time;

    iface->writeCSValue("/deviceAccessDevice-" + NDSDA_TEST_SIGNEDREGPV + "-Set", tmp_time, write_value);
    iface->readCSValue("/deviceAccessDevice-" + NDSDA_TEST_SIGNEDREGPV + "-Get", &tmp_time, &read_value);

    EXPECT_EQ(write_value, read_value);
}

TEST_F(ndsDABasicDeviceFixture, WriteReadPVUInt32) {
    uint32_t write_value = -1;
    int32_t read_value   = 0;
    timespec tmp_time;

    iface->writeCSValue("/deviceAccessDevice-" + NDSDA_TEST_REGISTERPV + "-Set", tmp_time, std::bit_cast<int32_t>(write_value));
    iface->readCSValue("/deviceAccessDevice-" + NDSDA_TEST_REGISTERPV + "-Get", &tmp_time, &read_value);

    EXPECT_EQ(write_value, std::bit_cast<int32_t>(read_value));
}

TEST_F(ndsDABasicDeviceFixture, WriteReadOneDUInt32) {
    std::vector<uint32_t> write_values(NDSDA_TEST_ONEDELEMENTS), read_values(NDSDA_TEST_ONEDELEMENTS);
    timespec tmp_time;

    for (size_t i = 0; i < write_values.size(); ++i) write_values[i] = std::rand() % INT32_MAX;

    iface->writeCSValue("/deviceAccessDevice-" + NDSDA_TEST_ONEDPV + "-Set", tmp_time, write_values);
    iface->readCSValue("/deviceAccessDevice-" + NDSDA_TEST_ONEDPV + "-Get", &tmp_time, &read_values);
    EXPECT_EQ(write_values.size(), read_values.size());
    for (size_t i = 0; i < write_values.size(); ++i)
        EXPECT_EQ(write_values[i], read_values[i]);
}