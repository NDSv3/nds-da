
#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <memory>
#include <ctime>

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

TEST_F(ndsDABridgeFixture, ReadI32) {
    const int32_t testValue                    = std::rand() % INT32_MAX;
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

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    int32_t value = DATestHelper::getInstance().readRegister<uint32_t>(pv->reg);

    ASSERT_EQ(testValue, value) << "WritePV value does not match read value";
}

TEST_F(ndsDABridgeFixture, ReadFloatBitcast) {
    const float testValue                      = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATBITCAST];

    timespec ts;
    float value;

    DATestHelper::getInstance().writeRegister(pv->reg, std::bit_cast<int32_t>(testValue));

    bridge.readPV(&ts, &value, *pv);
    ASSERT_NE(ts.tv_nsec, 0) << "ReadPV did not set timestamp";
    ASSERT_NE(ts.tv_sec, 0) << "ReadPV did not set timestamp";
    ASSERT_EQ(testValue, value) << "ReadPV value does not match written value";
}

TEST_F(ndsDABridgeFixture, WriteFloatBitcast) {
    const float testValue                      = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATBITCAST];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    int32_t value = DATestHelper::getInstance().readRegister<int32_t>(pv->reg);

    ASSERT_EQ(std::bit_cast<float>(value), testValue) << "WritePV value does not match read value";
}

TEST_F(ndsDABridgeFixture, ReadFloatFixed8) {
    const float testValue                      = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
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
    const float testValue                      = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    const float LSB                            = 1.0f / static_cast<float>(1 << 8);
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATFIXED8];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    float value = DATestHelper::getInstance().readRegister<float>(pv->reg);

    ASSERT_NEAR(value, testValue, LSB) << "WritePV value does not match read value";
}

TEST_F(ndsDABridgeFixture, ReadFloatFixed16) {
    const float testValue = 2 * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) - 1;
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
    const float testValue = 2 * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) - 1;
    const float LSB       = 1.0f / static_cast<float>(1 << 16);

    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[expectedPVIdx::FLOATFIXED16];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    float value = DATestHelper::getInstance().readRegister<float>(pv->reg);

    ASSERT_NEAR(value, testValue, LSB) << "WritePV value does not match read value";
}