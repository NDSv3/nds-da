#include <gtest/gtest.h>
#include <nds3/exceptions.h>
#include <bit>
#include <cmath>
#include <ios>
#include <random>
#include <ranges>

#include "ndsda-test-common.hpp"
#include "ndsda-test-constants.hpp"
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

TEST_F(ndsDAAccessorFixture, WriteAndReadUint32Register) {
    uint32_t testValue          = 12345;
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::UINTEGER32]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto res = accessor.writeRegister<uint32_t>(*memoryElementRef, testValue);
    ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();

    auto readValue = accessor.readRegister<uint32_t>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_EQ(testValue, readValue.assume_value()) << "Read value does not match written value";
}

TEST_F(ndsDAAccessorFixture, WriteAndReadInt32Register) {
    int32_t testValue           = -9876;
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::INTEGER32]);
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
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::UINTEGER32]);
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
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::OneDElement>(expectedMemoryElements[expectedMemoryElementIdx::VECUINT32]);
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
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::OneDElement>(expectedMemoryElements[expectedMemoryElementIdx::VECUINT32]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    std::vector<uint32_t> largeData(memoryElementRef->elements + 1, 1);

    auto writeError = accessor.writeOneDRegister<uint32_t>(*memoryElementRef, largeData);
    ASSERT_TRUE(writeError.has_value()) << "Write should have failed for oversized data";
}

TEST_F(ndsDAAccessorFixture, RegistersDoNotInterfereWithEachOther) {
    const auto firstRegRef  = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::UINTEGER32]);
    const auto secondRegRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::INTEGER32]);
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
    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::ONEBIT]);
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto writeRes = accessor.writeRegister<uint32_t>(*memoryElementRef, intTestValue);
    ASSERT_FALSE(writeRes.has_value()) << "Write failed: " << writeRes->what();

    auto readValue = accessor.readRegister<uint32_t>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_EQ(readValue.assume_value(), intTestValue & 0x1) << "Read value does not match written value";
}

// Generate number in range [min, -1] U [1, max]
static float generateFloatInFixedRange(int width, int fracBits) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    double max_value = (std::pow(2.0, width - 1) - 1.0) / std::pow(2.0, fracBits) - 1;
    double min_value = -1.0 * (std::pow(2.0, width - 1)) / std::pow(2.0, fracBits) + 1;

    std::uniform_real_distribution<float> dist(static_cast<float>(min_value), static_cast<float>(max_value));
    float result = dist(gen);
    return result >= 0 ? result + 1.0f : result - 1.0f;
}

TEST_F(ndsDAAccessorFixture, WriteReadFloatFixed) {
    const auto me = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::FLOATFIXED16]);
    ASSERT_NE(me, nullptr) << "Invalid memory element type for test";

    // Avoid values with abs < 1 and outside range, so they are exactly representable in fixed-point
    const float value = generateFloatInFixedRange(me->elementWidthBits, me->fracBits);

    auto writeRes = accessor.writeRegister<float>(*me, value);
    ASSERT_FALSE(writeRes.has_value()) << "Write failed: " << writeRes->what();

    auto readValue = accessor.readRegister<float>(*me);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_FLOAT_EQ(readValue.assume_value(), value) << "Read value does not match written value within tolerance";
}

TEST_F(ndsDAAccessorFixture, WriteReadFloatSmallNumber) {
    const auto me = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::FLOATFIXED16]);
    ASSERT_NE(me, nullptr) << "Invalid memory element type for test";

    // Test value between -1 and 1, which cannot be exactly represented in fixed-point
    // Allowed error = +- 1 LSB
    const float value = 2 * (std::rand() / static_cast<float>(RAND_MAX)) - 1.0f;
    const float LSB   = 1.0f / static_cast<float>(1 << me->fracBits);

    auto writeRes = accessor.writeRegister<float>(*me, value);
    ASSERT_FALSE(writeRes.has_value()) << "Write failed: " << writeRes->what();

    auto readValue = accessor.readRegister<float>(*me);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_NEAR(readValue.assume_value(), value, LSB) << "Read value does not match written value within tolerance";
}

TEST_F(ndsDAAccessorFixture, WriteAndReadFloatSaturation) {
    float testValue = INT16_MAX + 1;

    const auto memoryElementRef = dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::FLOATFIXED16]);  // 16 fractional bits
    ASSERT_NE(memoryElementRef, nullptr) << "Invalid memory element type for test";

    auto res = accessor.writeRegister<float>(*memoryElementRef, testValue);
    ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();

    auto readValue = accessor.readRegister<float>(*memoryElementRef);
    ASSERT_TRUE(readValue.has_value()) << "Read register returned error: " << readValue.assume_error().what();
    ASSERT_FLOAT_EQ(INT16_MAX, readValue.assume_value()) << "Read value does not match saturated value";
}
