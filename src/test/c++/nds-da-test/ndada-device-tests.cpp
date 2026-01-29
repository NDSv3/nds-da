#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include "ndsda-test-common.hpp"
#include "ndsda-test-constants.hpp"

TEST(DeviceTests, MissingDeviceMap_Exception) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceAlias", "DeviceAlias");
    params.emplace("NodeMapFile", "nodemap.xml");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(DeviceTests, MissingDeviceAlias_Exception) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceMapFile", "deviceMap.txt");
    params.emplace("NodeMapFile", "nodemap.xml");

    ASSERT_THROW(
        factory.createDevice("DADevice", "deviceAccessDevice", params),
        nds::NdsError
    );
}

TEST(DeviceTests, MissingNodeMap_Exception) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    params.emplace("DeviceAlias", "nodemap.xml");
    params.emplace("DeviceMapFile", "deviceMap.txt");

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
    params.emplace("NodeMapFile", "nodemap.xml");

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
    params.emplace("NodeMapFile", NDSDA_TEST_NODEMAPFILE);

    factory.createDevice("DADevice", "deviceAccessDevice", params);
    factory.destroyDevice("deviceAccessDevice");
}

TEST_F(ndsDABasicDeviceFixture, GetDeviceAlias) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-deviceAlias", &tmp_time, &tmp_string);
    ASSERT_EQ(tmp_string, NDSDA_TEST_DEVICEALIAS);
    ASSERT_NE(tmp_time.tv_sec, 0);
}

TEST_F(ndsDABasicDeviceFixture, GetDeviceMapFile) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-deviceMapFile", &tmp_time, &tmp_string);
    ASSERT_EQ(tmp_string, NDSDA_TEST_DMAPFILE);
    ASSERT_NE(tmp_time.tv_sec, 0);
}

TEST_F(ndsDABasicDeviceFixture, GetMemoryMapFile) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-memoryMapFile", &tmp_time, &tmp_string);
    ASSERT_EQ(tmp_string, NDSDA_TEST_MMAPFILE);
    ASSERT_NE(tmp_time.tv_sec, 0);
}

TEST_F(ndsDABasicDeviceFixture, GetNodeMapFile) {
    std::string tmp_string;
    timespec tmp_time;

    iface->readCSValue("/deviceAccessDevice-nodeMapFile", &tmp_time, &tmp_string);
    ASSERT_EQ(tmp_string, NDSDA_TEST_NODEMAPFILE);
    ASSERT_NE(tmp_time.tv_sec, 0);
}

TEST_F(ndsDABasicDeviceFixture, WriteReadPVInt32) {
    int32_t write_value = 12345678;
    int32_t read_value  = 0;
    timespec tmp_time;

    iface->writeCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[expectedPVIdx::INTEGER32]->writePV, tmp_time, write_value);
    iface->readCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[expectedPVIdx::INTEGER32]->readPV, &tmp_time, &read_value);

    EXPECT_EQ(write_value, read_value);
}

TEST_F(ndsDABasicDeviceFixture, WriteReadPVUInt32) {
    uint32_t write_value = -1;
    int32_t read_value   = 0;
    timespec tmp_time;

    iface->writeCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[expectedPVIdx::UINTEGER32]->writePV, tmp_time, std::bit_cast<int32_t>(write_value));
    iface->readCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[expectedPVIdx::UINTEGER32]->readPV, &tmp_time, &read_value);

    EXPECT_EQ(write_value, std::bit_cast<int32_t>(read_value));
}

TEST_F(ndsDABasicDeviceFixture, WriteReadOneDUInt32) {
    std::vector<uint32_t> write_values(NDSDA_TEST_ONEDELEMENTS), read_values(NDSDA_TEST_ONEDELEMENTS);
    timespec tmp_time;

    for (size_t i = 0; i < write_values.size(); ++i) write_values[i] = std::rand() % INT32_MAX;

    iface->writeCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[expectedPVIdx::VECUINT32]->writePV, tmp_time, write_values);
    iface->readCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[expectedPVIdx::VECUINT32]->writePV, &tmp_time, &read_values);
    EXPECT_EQ(write_values.size(), read_values.size());
    for (size_t i = 0; i < write_values.size(); ++i)
        EXPECT_EQ(write_values[i], read_values[i]);
}

TEST_F(ndsDABasicDeviceFixture, WriteReadFloatBitcast) {
    float write_value = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    float read_value;
    timespec tmp_time;

    iface->writeCSValue("/deviceAccessDevice-PV3-Setter", tmp_time, write_value);
    iface->readCSValue("/deviceAccessDevice-PV3-Getter", &tmp_time, &read_value);

    EXPECT_EQ(write_value, read_value);
}