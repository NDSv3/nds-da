
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
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[0];

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
    const std::shared_ptr<ndsda::NodeMapPV> pv = expectedNodeMapPVs[0];

    timespec ts = {0, 0};
    bridge.writePV(ts, testValue, *pv);
    int32_t value = DATestHelper::getInstance().readRegister<uint32_t>(pv->reg);

    ASSERT_EQ(testValue, value) << "WritePV value does not match read value";
}