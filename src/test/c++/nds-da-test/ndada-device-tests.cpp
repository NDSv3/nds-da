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

#define NDSDA_TESTF_TESTBASE(TYPE, PV_IDX, RANDOM_GEN)                                                        \
    TYPE read_value;                                                                                          \
    timespec tmp_time;                                                                                        \
    iface->writeCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[PV_IDX]->writePV, tmp_time, write_value); \
    iface->readCSValue("/deviceAccessDevice-" + expectedNodeMapPVs[PV_IDX]->readPV, &tmp_time, &read_value);

#define NDSDA_TESTF_SCALARPVTEST(TYPE, PV_IDX, RANDOM_GEN)   \
    TEST_F(ndsDABasicDeviceFixture, WriteReadScalar##TYPE) { \
        TYPE write_value = (RANDOM_GEN);                     \
        NDSDA_TESTF_TESTBASE(TYPE, PV_IDX, RANDOM_GEN)       \
        EXPECT_EQ(write_value, read_value);                  \
    }
#define NDSDA_TESTF_SCALARFLOATNEARPVTEST(PV_IDX, RANDOM_GEN, FIXEDBITS)                \
    TEST_F(ndsDABasicDeviceFixture, WriteReadFloatFixed##FIXEDBITS) {                   \
        float write_value = (RANDOM_GEN);                                               \
        NDSDA_TESTF_TESTBASE(float, PV_IDX, RANDOM_GEN)                                 \
        EXPECT_NEAR(write_value, read_value, 1.0 / static_cast<float>(1 << FIXEDBITS)); \
    }

#define NDSDA_TESTF_VECTORPVTEST(TYPE, PV_IDX, RANDOM_GEN)          \
    TEST_F(ndsDABasicDeviceFixture, WriteReadVector##TYPE) {        \
        std::vector<TYPE> write_value;                              \
        for (size_t i = 0; i < write_value.size(); i++) {           \
            write_value.push_back(RANDOM_GEN);                      \
        }                                                           \
        NDSDA_TESTF_TESTBASE(std::vector<TYPE>, PV_IDX, RANDOM_GEN) \
        for (size_t i = 0; i < write_value.size(); ++i) {           \
            ASSERT_EQ(write_value[i], read_value[i]);               \
        }                                                           \
    }
#define NDSDA_TESTF_VECTORFLOATNEARPVTEST(PV_IDX, RANDOM_GEN, FIXEDBITS)                          \
    TEST_F(ndsDABasicDeviceFixture, WriteReadVectorFloatFixed##FIXEDBITS) {                       \
        std::vector<float> write_value;                                                           \
        for (size_t i = 0; i < write_value.size(); i++) {                                         \
            write_value.push_back(RANDOM_GEN);                                                    \
        }                                                                                         \
        NDSDA_TESTF_TESTBASE(std::vector<float>, PV_IDX, RANDOM_GEN)                              \
        for (size_t i = 0; i < write_value.size(); ++i) {                                         \
            ASSERT_NEAR(write_value[i], read_value[i], 1.0 / static_cast<float>(1 << FIXEDBITS)); \
        }                                                                                         \
    }

using String = std::string;
NDSDA_TESTF_SCALARPVTEST(int32_t, expectedPVIdx::INTEGER32, randomSigned(INT32_MAX))
NDSDA_TESTF_SCALARPVTEST(float, expectedPVIdx::FLOATBITCAST, randomFloat())
NDSDA_TESTF_SCALARPVTEST(String, expectedPVIdx::STRING, "TestStr")
NDSDA_TESTF_SCALARFLOATNEARPVTEST(expectedPVIdx::FLOATFIXED8, randomFloat(), 8)
NDSDA_TESTF_SCALARFLOATNEARPVTEST(expectedPVIdx::FLOATFIXED16, randomFloat(), 16)

NDSDA_TESTF_VECTORPVTEST(bool, expectedPVIdx::VECBOOL, randomSigned(BOOL_MAX))
NDSDA_TESTF_VECTORPVTEST(uint8_t, expectedPVIdx::VECUINT8, randomUnsigned(UINT8_MAX))
NDSDA_TESTF_VECTORPVTEST(uint16_t, expectedPVIdx::VECUINT16, randomUnsigned(UINT16_MAX))
NDSDA_TESTF_VECTORPVTEST(uint32_t, expectedPVIdx::VECUINT32, randomUnsigned(UINT32_MAX))
NDSDA_TESTF_VECTORPVTEST(int8_t, expectedPVIdx::VECINT8, randomSigned(INT8_MAX))
NDSDA_TESTF_VECTORPVTEST(int16_t, expectedPVIdx::VECINT16, randomSigned(INT16_MAX))
NDSDA_TESTF_VECTORPVTEST(int32_t, expectedPVIdx::VECINT32, randomSigned(INT32_MAX))
NDSDA_TESTF_VECTORPVTEST(float, expectedPVIdx::VECFLOATBITCAST, randomFloat())
NDSDA_TESTF_VECTORFLOATNEARPVTEST(expectedPVIdx::VECFLOATFIXED8, randomFloat(), 8)