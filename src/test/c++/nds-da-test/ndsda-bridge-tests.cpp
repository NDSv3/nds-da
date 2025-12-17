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

static std::vector<std::shared_ptr<ndsda::MemoryElement>> createExpectedElements() {
    return {
        std::make_shared<ndsda::OneDElement>(".BASE", 4, 32),
        std::make_shared<ndsda::RegisterElement>("BASE.FIRST_REG"),
        std::make_shared<ndsda::RegisterElement>("BASE.SECOND_REG"),
        std::make_shared<ndsda::RegisterElement>("BASE.THIRD_REG"),
        std::make_shared<ndsda::RegisterElement>("BASE.ONE_BIT_REG"),
        std::make_shared<ndsda::OneDElement>(".DMA_SAMPLES", 200, 32)
    };
}

TEST(DABridge, ConstructorWithInvalidDmapFilePath) {
    auto elements = createExpectedElements();
    EXPECT_THROW(
        ndsda::DABridge bridge("invalid/path.dmap", NDSDA_TEST_DEVICEALIAS, elements),
        nds::NdsError
    );
}

TEST(DABridge, ConstructorWithInvalidDeviceName) {
    auto elements = createExpectedElements();
    EXPECT_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DEVICE_MAPFILE, "NONEXISTENT_DEVICE", elements),
        nds::NdsError
    );
}

TEST(DABridge, ConstructorWithEmptyElements) {
    std::vector<std::shared_ptr<ndsda::MemoryElement>> emptyElements;
    EXPECT_NO_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DEVICE_MAPFILE, NDSDA_TEST_DEVICEALIAS, emptyElements)
    );
}

TEST(DABridge, ConstructorWithNullElement) {
    auto elements = createExpectedElements();
    elements.push_back(nullptr);
    EXPECT_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DEVICE_MAPFILE, NDSDA_TEST_DEVICEALIAS, elements),
        std::exception
    );
}

TEST_F(ndsDABridgeFixture, WriteAndReadUint32Register) {
    uint32_t testValue = 12345;
    bridge->writeRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, testValue);
    uint32_t readValue = bridge->readRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValue, readValue);
}

TEST_F(ndsDABridgeFixture, WriteAndReadInt32Register) {
    int32_t testValue = -9876;
    bridge->writeRegister<int32_t>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, testValue);
    int32_t readValue = bridge->readRegister<int32_t>(DAPV_REGISTER_PV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValue, readValue);
}

TEST_F(ndsDABridgeFixture, WriteAndReadFloatRegister) {
    float testValue = 3.14159f;
    bridge->writeRegister<float>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, testValue);
    float readValue = bridge->readRegister<float>(DAPV_REGISTER_PV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_FLOAT_EQ(testValue, readValue);
}

TEST_F(ndsDABridgeFixture, MultipleWritesRetainLatestValue) {
    bridge->writeRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, 100);
    bridge->writeRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, 200);
    bridge->writeRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, 300);

    EXPECT_EQ(300, bridge->readRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_GET_SUFFIX));
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
    std::vector<uint32_t> testValues(DAPV_ONED_ELEMENTS);
    for (size_t i = 0; i < testValues.size(); ++i) testValues[i] = std::rand() % INT32_MAX;
    bridge->writeOneDRegister<uint32_t>(DAPV_ONED_PV + ndsda::DAPV_SET_SUFFIX, testValues);
    auto readValues = bridge->readOneDRegister<uint32_t>(DAPV_ONED_PV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValues.size(), readValues.size());
    for (size_t i = 0; i < testValues.size(); ++i) {
        EXPECT_EQ(testValues[i], readValues[i]);
    }
}

TEST_F(ndsDABridgeFixture, WriteAndReadOneDRegisterFloat) {
    std::vector<float> testValues(DAPV_ONED_ELEMENTS);
    for (size_t i = 0; i < testValues.size(); ++i) testValues[i] = static_cast<float>(std::rand()) / RAND_MAX * 1000;
    bridge->writeOneDRegister<float>(DAPV_ONED_PV + ndsda::DAPV_SET_SUFFIX, testValues);
    auto readValues = bridge->readOneDRegister<float>(DAPV_ONED_PV + ndsda::DAPV_GET_SUFFIX);

    EXPECT_EQ(testValues.size(), readValues.size());
    for (size_t i = 0; i < testValues.size(); ++i) {
        EXPECT_FLOAT_EQ(testValues[i], readValues[i]);
    }
}

TEST_F(ndsDABridgeFixture, ReadOneDRegisterExceedingElementsThrows) {
    std::vector<uint32_t> largeData(300, 1);
    EXPECT_THROW(
        bridge->writeOneDRegister<uint32_t>(DAPV_ONED_PV + ndsda::DAPV_SET_SUFFIX, largeData),
        nds::NdsError
    );
}

TEST_F(ndsDABridgeFixture, RegistersDoNotInterfereWithEachOther) {
    bridge->writeRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_SET_SUFFIX, 111);
    bridge->writeRegister<uint32_t>(std::string("BASE.SECOND_REG") + ndsda::DAPV_SET_SUFFIX, 222);

    EXPECT_EQ(111, bridge->readRegister<uint32_t>(DAPV_REGISTER_PV + ndsda::DAPV_GET_SUFFIX));
    EXPECT_EQ(222, bridge->readRegister<uint32_t>(std::string("BASE.SECOND_REG") + ndsda::DAPV_GET_SUFFIX));
}