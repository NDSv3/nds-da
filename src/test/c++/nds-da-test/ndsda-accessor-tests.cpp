#include <gtest/gtest.h>
#include <nds3/exceptions.h>

#include "ndsda-test-common.hpp"
#include "ndsdaAccessor.hpp"
#include "ndsdaCommon.hpp"

TEST(DAAccessor, ConstructorWithInvalidDmapFilePath) {
    EXPECT_THROW(
        ndsda::DAAccessor daAccessor("invalid/path.dmap", NDSDA_TEST_DEVICEALIAS),
        nds::NdsError
    );
}

TEST(DAAccessor, ConstructorWithInvalidDeviceName) {
    EXPECT_THROW(
        ndsda::DAAccessor daAccessor(NDSDA_TEST_DMAPFILE, "NONEXISTENT_DEVICE"),
        nds::NdsError
    );
}

TEST(DAAccessor, ConstructorWithValidParameters) {
    EXPECT_NO_THROW(
        ndsda::DAAccessor daAccessor(NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS)
    );
}

TEST_F(ndsDAAccessorFixture, WriteAndReadUint32Register) {  // TODO: Constants for memoryElement index
    uint32_t testValue          = 12345;
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[1]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto res = accessor.writeRegister<uint32_t>(*memoryElementRef, testValue);
    ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();

    auto readValue = accessor.readRegister<uint32_t>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_EQ(testValue, readValue.assume_value()) << "Read value does not match written value";
}

TEST_F(ndsDAAccessorFixture, WriteAndReadInt32Register) {
    int32_t testValue           = -9876;
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[2]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto res = accessor.writeRegister<int32_t>(*memoryElementRef, testValue);
    ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();

    auto readValue = accessor.readRegister<int32_t>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_EQ(testValue, readValue.assume_value()) << "Read value does not match written value";
}

TEST_F(ndsDAAccessorFixture, MultipleWritesRetainLatestValue) {
    std::vector<uint32_t> testValues(5);
    for (size_t i = 0; i < testValues.size(); ++i) {
        testValues[i] = std::rand() % INT32_MAX;
    }
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[1]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    for (const auto& val : testValues) {
        auto res = accessor.writeRegister<uint32_t>(*memoryElementRef, val);
        ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();
    }

    auto readValue = accessor.readRegister<uint32_t>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_EQ(testValues[testValues.size() - 1], readValue.assume_value()) << "Read value does not match expected latest value";
}

TEST_F(ndsDAAccessorFixture, WriteAndReadOneDRegisterUint32) {
    std::vector<uint32_t> testValues(NDSDA_TEST_ONEDELEMENTS);
    for (size_t i = 0; i < testValues.size(); ++i) {
        testValues[i] = std::rand() % INT32_MAX;
    }
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::OneDElement>(expectedMemoryElements[5]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto res = accessor.writeOneDRegister<uint32_t>(*memoryElementRef, testValues);
    ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();

    auto readValues = accessor.readOneDRegister<uint32_t>(*memoryElementRef);
    ASSERT_TRUE(readValues.has_value()) << "Read register returned error: " << readValues.assume_error().what();

    const auto& readValuesVec = readValues.assume_value();
    ASSERT_EQ(testValues.size(), readValuesVec.size()) << "Read vector size does not match written size";
    for (size_t i = 0; i < testValues.size(); ++i) {
        ASSERT_EQ(testValues[i], readValuesVec[i]) << "Value mismatch at index " << i;
    }
}

TEST_F(ndsDAAccessorFixture, ReadOneDRegisterExceedingElementsFails) {
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::OneDElement>(expectedMemoryElements[5]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    std::vector<uint32_t> largeData(memoryElementRef->elements + 1, 1);

    auto writeError = accessor.writeOneDRegister<uint32_t>(*memoryElementRef, largeData);
    ASSERT_TRUE(writeError.has_value()) << "Write should have failed for oversized data";
}

TEST_F(ndsDAAccessorFixture, RegistersDoNotInterfereWithEachOther) {
    const auto firstRegRef  = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[1]);
    const auto secondRegRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[2]);
    ASSERT_NE(firstRegRef, nullptr) << "Invalid first register element type";
    ASSERT_NE(secondRegRef, nullptr) << "Invalid second register element type";

    accessor.writeRegister<uint32_t>(*firstRegRef, 111);
    accessor.writeRegister<int32_t>(*secondRegRef, 222);

    auto readFirst  = accessor.readRegister<uint32_t>(*firstRegRef);
    auto readSecond = accessor.readRegister<int32_t>(*secondRegRef);

    ASSERT_TRUE(readFirst.has_value()) << "Read first register failed";
    ASSERT_TRUE(readSecond.has_value()) << "Read second register failed";
    ASSERT_EQ(111, readFirst.assume_value()) << "First register value mismatch";
    ASSERT_EQ(222, readSecond.assume_value()) << "Second register value mismatch";
}

TEST_F(ndsDAAccessorFixture, WriteReadOneBitRegister) {
    const int32_t intTestValue  = std::rand() % UINT32_MAX;
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[4]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto writeRes = accessor.writeRegister<uint32_t>(*memoryElementRef, intTestValue);
    ASSERT_FALSE(writeRes.has_value()) << "Write failed: " << writeRes->what();

    auto readValue = accessor.readRegister<uint32_t>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_EQ(readValue.assume_value(), intTestValue & 0x1) << "Read value does not match written value";
}

TEST_F(ndsDAAccessorFixture, WriteReadAll) {
    const int32_t intTestValue   = std::rand() % UINT32_MAX;
    const uint32_t uintTestValue = std::rand() % INT32_MAX;
    const std::vector<int32_t> intOneDTest(NDSDA_TEST_ONEDELEMENTS, intTestValue);
    const std::vector<uint32_t> uintOneDTest(NDSDA_TEST_ONEDELEMENTS, uintTestValue);

    // Remove first element
    auto testElements = expectedMemoryElements;  // Copy vector
    testElements.erase(testElements.begin());

    // TODO: Add bit width to register element and test it
    for (auto it = testElements.begin(); it != testElements.end();) {
        if ((*it)->name == "BASE.ONE_BIT_REG") {
            it = testElements.erase(it);
        } else {
            ++it;
        }
    }

    for (const auto& element : testElements) {
        std::optional<ndsda::ndsdaError> writeResult;
        if (element->getType() == ndsda::MemoryElementType::SCALAR) {
            const auto regPtr = std::dynamic_pointer_cast<ndsda::RegisterElement>(element);
            ASSERT_NE(regPtr, nullptr) << "Invalid memory element type for scalar register";

            if (regPtr->isSigned) {
                writeResult = accessor.writeRegister<int32_t>(*regPtr, intTestValue);
            } else {
                writeResult = accessor.writeRegister<uint32_t>(*regPtr, uintTestValue);
            }
        } else {  // ONED
            const auto oneDPtr = std::dynamic_pointer_cast<ndsda::OneDElement>(element);
            ASSERT_NE(oneDPtr, nullptr) << "Invalid memory element type for OneD register";

            if (oneDPtr->isSigned) {
                writeResult = accessor.writeOneDRegister<int32_t>(*oneDPtr, intOneDTest);
            } else {
                writeResult = accessor.writeOneDRegister<uint32_t>(*oneDPtr, uintOneDTest);
            }
        }
        ASSERT_FALSE(writeResult.has_value()) << "Write failed for element " << element->name << ": " << writeResult->what();
    }

    for (const auto& element : testElements) {
        if (element->getType() == ndsda::MemoryElementType::SCALAR) {
            const auto regPtr = std::dynamic_pointer_cast<ndsda::RegisterElement>(element);
            ASSERT_NE(regPtr, nullptr) << "Invalid memory element type for scalar register";

            if (regPtr->isSigned) {
                auto readResult = accessor.readRegister<int32_t>(*regPtr);
                ASSERT_TRUE(readResult.has_value()) << "Read register returned error: " << readResult.assume_error().what();
                EXPECT_EQ(readResult.assume_value(), intTestValue) << "Read value does not match written value for element " << element->name;
            } else {
                auto readResult = accessor.readRegister<uint32_t>(*regPtr);
                ASSERT_TRUE(readResult.has_value()) << "Read register returned error: " << readResult.assume_error().what();
                EXPECT_EQ(readResult.assume_value(), uintTestValue) << "Read value does not match written value for element " << element->name;
            }
        } else {  // ONED
            const auto oneDPtr = std::dynamic_pointer_cast<ndsda::OneDElement>(element);
            ASSERT_NE(oneDPtr, nullptr) << "Invalid memory element type for OneD register";

            if (oneDPtr->isSigned) {
                auto readResult = accessor.readOneDRegister<int32_t>(*oneDPtr);
                ASSERT_TRUE(readResult.has_value()) << "Read OneD register returned error: " << readResult.assume_error().what();
                ASSERT_EQ(readResult.assume_value(), intOneDTest) << "Read OneD values do not match written values for element " << element->name;
            } else {
                auto readResult = accessor.readOneDRegister<uint32_t>(*oneDPtr);
                ASSERT_TRUE(readResult.has_value()) << "Read OneD register returned error: " << readResult.assume_error().what();
                ASSERT_EQ(readResult.assume_value(), uintOneDTest) << "Read OneD values do not match written values for element " << element->name;
            }
        }
    }
}