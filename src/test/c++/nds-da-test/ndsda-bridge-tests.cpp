
#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <cstdint>
#include <cstdlib>
#include <ranges>
#include <vector>
#include <memory>
#include <ctime>

#include "gtest/gtest.h"
#include "ndsda-test-common.hpp"
#include "ndsda-test-constants.hpp"
#include "ndsda-test-daHelper.hpp"

#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"

TEST(DABridge, ConstructorNullptrError) {
    EXPECT_THROW(
        ndsda::DABridge bridge(nullptr),
        ndsda::ndsdaError
    );
}

TEST(DABridge, ConstructorValidParameters) {
    EXPECT_NO_THROW(
        ndsda::DABridge bridge(std::make_unique<ndsda::DAAccessor>(NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS))
    );
}

TEST_F(ndsDABridgeFixture, WriteScalarToOneDException) {
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::VECINT32];

    timespec ts = {0, 0};
    EXPECT_THROW(
        bridge.writePV(ts, 0, *pv),
        ndsda::ndsdaError
    );
}

TEST_F(ndsDABridgeFixture, WriteVectorToScalarException) {
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::INTEGER32];

    EXPECT_THROW(
        bridge.writePV({0, 0}, std::vector<int32_t>(0), *pv),
        ndsda::ndsdaError
    );
}

TEST_F(ndsDABridgeFixture, ReadI32) {
    const int32_t testValue                    = randomSigned(INT32_MAX);
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::INTEGER32];
    timespec ts;
    int32_t value;
    DATestHelper::getInstance().writeRegister(pv->reg, testValue);
    bridge.readPV(&ts, &value, *pv);
    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    ASSERT_EQ(testValue, value) << "ReadPV value does not match written value";
}
TEST_F(ndsDABridgeFixture, WriteI32) {
    const int32_t testValue                    = std::rand() % INT32_MAX;
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::INTEGER32];
    timespec ts                                = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    int32_t value = DATestHelper::getInstance().readRegister<uint32_t>(pv->reg);
    ASSERT_EQ(testValue, value) << "WritePV value does not match read value";
}

TEST_F(ndsDABridgeFixture, ReadFloatBitcast) {
    const float testValue                      = randomFloat();
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATBITCAST];

    timespec ts;
    float value;

    DATestHelper::getInstance().writeRegister(pv->reg, testValue);
    bridge.readPV(&ts, &value, *pv);
    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    ASSERT_EQ(testValue, value) << "ReadPV value does not match written value";
}
TEST_F(ndsDABridgeFixture, WriteFloatBitcast) {
    const float testValue                      = randomFloat();
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATBITCAST];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    float value = DATestHelper::getInstance().readRegister<float>(pv->reg);

    ASSERT_EQ(value, testValue) << "WritePV value does not match read value";
}

TEST_F(ndsDABridgeFixture, ReadFloatFixed8) {
    const float testValue                      = randomFloat();
    const float LSB                            = 1.0f / static_cast<float>(1 << 8);
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATFIXED8];

    timespec ts;
    float value;

    DATestHelper::getInstance().writeRegister(pv->reg, static_cast<float>(testValue));

    bridge.readPV(&ts, &value, *pv);
    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    ASSERT_NEAR(testValue, value, LSB) << "ReadPV value does not match written value";
}

TEST_F(ndsDABridgeFixture, WriteFloatFixed8) {
    const float testValue                      = randomFloat();
    const float LSB                            = 1.0f / static_cast<float>(1 << 8);
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATFIXED8];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    float value = DATestHelper::getInstance().readRegister<float>(pv->reg);

    ASSERT_NEAR(value, testValue, LSB) << "WritePV value does not match read value";
}

TEST_F(ndsDABridgeFixture, ReadFloatFixed16) {
    const float testValue = randomFloat();
    const float LSB       = 1.0f / static_cast<float>(1 << 16);

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATFIXED16];

    timespec ts;
    float value;

    DATestHelper::getInstance().writeRegister(pv->reg, static_cast<float>(testValue));

    bridge.readPV(&ts, &value, *pv);
    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    ASSERT_NEAR(testValue, value, LSB) << "ReadPV value does not match written value";
}

TEST_F(ndsDABridgeFixture, WriteFloatFixed16) {
    const float testValue = randomFloat();
    const float LSB       = 1.0f / static_cast<float>(1 << 16);

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATFIXED16];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    float value = DATestHelper::getInstance().readRegister<float>(pv->reg);

    ASSERT_NEAR(value, testValue, LSB) << "WritePV value does not match read value";
}

#define NDSDA_TESTF_ONEDTESTS(TYPE, PVIDX, RANDOM_VALUE_GENERATION)                                                    \
    TEST_F(ndsDABridgeFixture, ReadOneD##TYPE) {                                                                       \
        timespec ts;                                                                                                   \
        std::array<TYPE, NDSDA_TEST_ONEDELEMENTS> writeArray;                                                          \
        std::vector<TYPE> readArray(NDSDA_TEST_ONEDELEMENTS);                                                          \
        for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {                                                         \
            writeArray[i] = (RANDOM_VALUE_GENERATION);                                                                 \
        }                                                                                                              \
        const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[PVIDX];                                        \
        DATestHelper::getInstance().writeOneD<TYPE>(pv->reg, writeArray);                                              \
        bridge.readPV(&ts, &readArray, *pv);                                                                           \
        ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";                                                    \
        ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";                                                     \
        for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {                                                         \
            ASSERT_EQ(writeArray[i], readArray[i]) << "ReadPV OneD value does not match written value at index " << i; \
        }                                                                                                              \
    }                                                                                                                  \
    TEST_F(ndsDABridgeFixture, WriteOneD##TYPE) {                                                                      \
        timespec ts = {0, 0};                                                                                          \
        std::vector<TYPE> writeArray;                                                                                  \
        for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {                                                         \
            writeArray.push_back(RANDOM_VALUE_GENERATION);                                                             \
        }                                                                                                              \
        const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[PVIDX];                                        \
        bridge.writePV(ts, writeArray, *pv);                                                                           \
        std::array<TYPE, NDSDA_TEST_ONEDELEMENTS> readArray = DATestHelper::getInstance().readOneD<TYPE>(pv->reg);     \
        for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {                                                         \
            ASSERT_EQ(writeArray[i], readArray[i]) << "WritePV OneD value does not match read value at index " << i;   \
        }                                                                                                              \
    }

NDSDA_TESTF_ONEDTESTS(int8_t, expectedPVIdx::VECINT8, randomSigned(INT8_MAX))
NDSDA_TESTF_ONEDTESTS(int16_t, expectedPVIdx::VECINT16, randomSigned(INT16_MAX))
NDSDA_TESTF_ONEDTESTS(int32_t, expectedPVIdx::VECINT32, randomSigned(INT32_MAX))

NDSDA_TESTF_ONEDTESTS(bool, expectedPVIdx::VECBOOL, randomUnsigned(BOOL_MAX))
NDSDA_TESTF_ONEDTESTS(uint8_t, expectedPVIdx::VECUINT8, randomUnsigned(UINT8_MAX))
NDSDA_TESTF_ONEDTESTS(uint16_t, expectedPVIdx::VECUINT16, randomUnsigned(UINT16_MAX))
NDSDA_TESTF_ONEDTESTS(uint32_t, expectedPVIdx::VECUINT32, randomUnsigned(UINT32_MAX))

using floatbitcast = float;
NDSDA_TESTF_ONEDTESTS(floatbitcast, expectedPVIdx::VECFLOATBITCAST, randomFloat())

TEST_F(ndsDABridgeFixture, ReadOneDFloatFixed8) {
    timespec ts;
    std::array<float, NDSDA_TEST_ONEDELEMENTS> writeArray;
    std::vector<float> readArray(NDSDA_TEST_ONEDELEMENTS);
    float LSB = 1.0f / static_cast<float>(1 << 8);

    for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {
        writeArray[i] = randomFloat();
    }

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::VECFLOATFIXED8];
    DATestHelper::getInstance().writeOneD<float>(pv->reg, writeArray);
    bridge.readPV(&ts, &readArray, *pv);

    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {
        ASSERT_NEAR(writeArray[i], readArray[i], LSB) << "ReadPV OneD value does not match written value at index " << i;
    }
}
TEST_F(ndsDABridgeFixture, WriteOneDFloatFixed8) {
    timespec ts = {0, 0};
    std::vector<float> writeArray;
    float LSB = 1.0f / static_cast<float>(1 << 8);

    for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {
        writeArray.push_back(randomFloat());
    }

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::VECFLOATFIXED8];
    bridge.writePV(ts, writeArray, *pv);
    std::array<float, NDSDA_TEST_ONEDELEMENTS> readArray = DATestHelper::getInstance().readOneD<float>(pv->reg);

    for (size_t i = 0; i < NDSDA_TEST_ONEDELEMENTS; ++i) {
        ASSERT_NEAR(writeArray[i], readArray[i], LSB) << "WritePV OneD value does not match read value at index " << i;
    }
}

TEST_F(ndsDABridgeFixture, ReadString) {
    timespec ts                               = {0, 0};
    static constexpr std::string randomString = "asdf1234";
    std::string readStr;
    std::array<uint8_t, NDSDA_TEST_ONEDELEMENTS> stringArray{};

    static_assert(stringArray.size() >= randomString.size());
    std::copy(randomString.begin(), randomString.end(), stringArray.begin());

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::STRING];

    DATestHelper::getInstance().writeOneD(pv->reg, stringArray);
    bridge.readPV(&ts, &readStr, *pv);

    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    ASSERT_EQ(readStr, randomString);
}
TEST_F(ndsDABridgeFixture, WriteString) {
    timespec ts                           = {0, 0};
    static constexpr std::string writeStr = "xyz0987";

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::STRING];
    bridge.writePV(ts, writeStr, *pv);

    auto readArr = DATestHelper::getInstance().readOneD<uint8_t>(pv->reg) | std::views::take(writeStr.size());

    ASSERT_EQ(writeStr, std::string(readArr.begin(), readArr.end()));
}