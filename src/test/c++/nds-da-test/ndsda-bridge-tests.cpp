#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include "nds-da-test-fixture.hpp"

#include "ndsdaBridge.hpp"
#include "ndsdaMemoryElements.hpp"

TEST(DABridge, ConstructorWithInvalidDmapFilePath) {
    EXPECT_THROW(
        ndsda::DABridge bridge("invalid/path.dmap", NDSDA_TEST_DEVICEALIAS, expectedElements),
        nds::NdsError
    );
}

TEST(DABridge, ConstructorWithInvalidDeviceName) {
    EXPECT_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DMAPFILE, "NONEXISTENT_DEVICE", expectedElements),
        nds::NdsError
    );
}

TEST(DABridge, ConstructorWithEmptyElements) {
    std::vector<std::shared_ptr<ndsda::MemoryElement>> emptyElements;
    EXPECT_NO_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS, emptyElements)
    );
}

TEST(DABridge, ConstructorWithNullElement) {
    std::vector<std::shared_ptr<ndsda::MemoryElement>> elements;
    elements.push_back(nullptr);
    EXPECT_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS, elements),
        std::exception
    );
}

TEST_F(ndsDABridgeFixture, WriteAndReadUint32Register) {
    uint32_t testValue = 12345;
    bridge->writeRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_SET_SUFFIX, testValue);
    uint32_t readValue = bridge->readRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValue, readValue);
}

TEST_F(ndsDABridgeFixture, WriteAndReadInt32Register) {
    int32_t testValue = -9876;
    bridge->writeRegister<int32_t>(NDSDA_TEST_SIGNEDREGPV + ndsda::DAPV_SET_SUFFIX, testValue);
    int32_t readValue = bridge->readRegister<int32_t>(NDSDA_TEST_SIGNEDREGPV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValue, readValue);
}

// TEST_F(ndsDABridgeFixture, WriteAndReadFloatRegister) {
//     float testValue = 3.14159f;
//     bridge->writeRegister<float>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_SET_SUFFIX, testValue);
//     float readValue = bridge->readRegister<float>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_GET_SUFFIX);

//     EXPECT_FLOAT_EQ(testValue, readValue);
// }

TEST_F(ndsDABridgeFixture, MultipleWritesRetainLatestValue) {
    bridge->writeRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_SET_SUFFIX, 100);
    bridge->writeRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_SET_SUFFIX, 200);
    bridge->writeRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_SET_SUFFIX, 300);

    EXPECT_EQ(300, bridge->readRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_GET_SUFFIX));
}

TEST_F(ndsDABridgeFixture, ReadNonexistentRegisterThrows) {
    EXPECT_THROW(
        bridge->readRegister<uint32_t>("nonexistent-Get"),
        nds::NdsError
    );
}

TEST_F(ndsDABridgeFixture, WriteNonexistentRegisterThrows) {
    EXPECT_THROW(
        bridge->writeRegister<uint32_t>("nonexistent-Set", 123),
        nds::NdsError
    );
}

TEST_F(ndsDABridgeFixture, WriteAndReadOneDRegisterUint32) {
    std::vector<uint32_t> testValues(NDSDA_TEST_ONEDELEMENTS);
    for (size_t i = 0; i < testValues.size(); ++i) testValues[i] = std::rand() % INT32_MAX;
    bridge->writeOneDRegister<uint32_t>(NDSDA_TEST_ONEDPV + ndsda::DAPV_SET_SUFFIX, testValues);
    auto readValues = bridge->readOneDRegister<uint32_t>(NDSDA_TEST_ONEDPV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValues.size(), readValues.size());
    for (size_t i = 0; i < testValues.size(); ++i) {
        EXPECT_EQ(testValues[i], readValues[i]);
    }
}

// TEST_F(ndsDABridgeFixture, WriteAndReadOneDRegisterFloat) {
//     std::vector<float> testValues(NDSDA_TEST_ONEDELEMENTS);
//     for (size_t i = 0; i < testValues.size(); ++i) testValues[i] = static_cast<float>(std::rand()) / RAND_MAX * 1000;
//     bridge->writeOneDRegister<float>(NDSDA_TEST_ONEDPV + ndsda::DAPV_SET_SUFFIX, testValues);
//     auto readValues = bridge->readOneDRegister<float>(NDSDA_TEST_ONEDPV + ndsda::DAPV_GET_SUFFIX);

//     EXPECT_EQ(testValues.size(), readValues.size());
//     for (size_t i = 0; i < testValues.size(); ++i) {
//         EXPECT_FLOAT_EQ(testValues[i], readValues[i]);
//     }
// }

TEST_F(ndsDABridgeFixture, ReadOneDRegisterExceedingElementsThrows) {
    std::vector<uint32_t> largeData(300, 1);
    EXPECT_THROW(
        bridge->writeOneDRegister<uint32_t>(NDSDA_TEST_ONEDPV + ndsda::DAPV_SET_SUFFIX, largeData),
        nds::NdsError
    );
}

TEST_F(ndsDABridgeFixture, RegistersDoNotInterfereWithEachOther) {
    bridge->writeRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_SET_SUFFIX, 111);
    bridge->writeRegister<int32_t>(NDSDA_TEST_SIGNEDREGPV + ndsda::DAPV_SET_SUFFIX, 222);

    EXPECT_EQ(111, bridge->readRegister<uint32_t>(NDSDA_TEST_REGISTERPV + ndsda::DAPV_GET_SUFFIX));
    EXPECT_EQ(222, bridge->readRegister<int32_t>(NDSDA_TEST_SIGNEDREGPV + ndsda::DAPV_GET_SUFFIX));
}