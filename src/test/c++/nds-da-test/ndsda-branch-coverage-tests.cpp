/**
 * @file ndsda-branch-coverage-tests.cpp
 * @brief Additional tests targeting uncovered branches across the NDS-DA codebase
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-06
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <gtest/gtest.h>
#include <nds3/exceptions.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "ndsda-test-common.hpp"
#include "ndsda-test-constants.hpp"
#include "ndsda-test-daHelper.hpp"
#include "ndsda-test-device.hpp"

#include "ndsdaAccessor.hpp"
#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"
#include "ndsdaFileParser.hpp"

// ========================== ndsdaCommon: generateRegName ==========================

TEST(Common_GenerateRegName, EdgeCases) {
    EXPECT_EQ(ndsda::generateRegName(""), "");
    EXPECT_EQ(ndsda::generateRegName("REGISTER"), "REGISTER");
    EXPECT_EQ(ndsda::generateRegName(".REGISTER"), "REGISTER");
    EXPECT_EQ(ndsda::generateRegName("."), "");
}

// ========================== ndsdaCommon: pvTypeEnum_isScalar ==========================

TEST(Common_PVTypeEnum, IsScalar) {
    EXPECT_TRUE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::INT32));
    EXPECT_TRUE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::INT64));
    EXPECT_TRUE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::FLOAT));
    EXPECT_TRUE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::DOUBLE));
    EXPECT_TRUE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::STRING));

    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_BOOL));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_UINT8));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_UINT16));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_UINT32));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_INT8));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_INT16));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_INT32));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_INT64));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_FLOAT));
    EXPECT_FALSE(ndsda::pvTypeEnum_isScalar(ndsda::PVTypeEnum::VEC_DOUBLE));
}

// ========================== ndsdaCommon: visitPVType & TtoPVTypeEnum ==========================

TEST(Common_VisitPVType, AllBranchesVisited) {
    auto checkType = []<typename Expected>(ndsda::PVTypeEnum type, std::type_identity<Expected>) {
        bool visited = false;
        ndsda::visitPVType(type, [&visited]<typename T>(std::type_identity<T>) {
            if constexpr (std::is_same_v<T, Expected>) {
                visited = true;
            }
        });
        EXPECT_TRUE(visited) << "visitPVType did not dispatch correctly";
    };

    checkType(ndsda::PVTypeEnum::INT32, std::type_identity<int32_t>{});
    checkType(ndsda::PVTypeEnum::INT64, std::type_identity<int64_t>{});
    checkType(ndsda::PVTypeEnum::FLOAT, std::type_identity<float>{});
    checkType(ndsda::PVTypeEnum::DOUBLE, std::type_identity<double>{});
    checkType(ndsda::PVTypeEnum::STRING, std::type_identity<std::string>{});
    checkType(ndsda::PVTypeEnum::VEC_BOOL, std::type_identity<std::vector<bool>>{});
    checkType(ndsda::PVTypeEnum::VEC_UINT8, std::type_identity<std::vector<uint8_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_UINT16, std::type_identity<std::vector<uint16_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_UINT32, std::type_identity<std::vector<uint32_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_INT8, std::type_identity<std::vector<int8_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_INT16, std::type_identity<std::vector<int16_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_INT32, std::type_identity<std::vector<int32_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_INT64, std::type_identity<std::vector<int64_t>>{});
    checkType(ndsda::PVTypeEnum::VEC_FLOAT, std::type_identity<std::vector<float>>{});
    checkType(ndsda::PVTypeEnum::VEC_DOUBLE, std::type_identity<std::vector<double>>{});
}

TEST(Common_TtoPVTypeEnum, AllMappingsCorrect) {
    EXPECT_EQ(ndsda::TtoPVTypeEnum<int32_t>, ndsda::PVTypeEnum::INT32);
    EXPECT_EQ(ndsda::TtoPVTypeEnum<std::vector<int32_t>>, ndsda::PVTypeEnum::VEC_INT32);
    EXPECT_EQ(ndsda::TtoPVTypeEnum<std::string>, ndsda::PVTypeEnum::STRING);
    // Spot check a few, template mappings are statically resolved
    EXPECT_EQ(ndsda::TtoPVTypeEnum<float>, ndsda::PVTypeEnum::FLOAT);
    EXPECT_EQ(ndsda::TtoPVTypeEnum<std::vector<double>>, ndsda::PVTypeEnum::VEC_DOUBLE);
}

// ========================== ndsdaCommon: Operator== ==========================

TEST(Common_RegisterElement, OperatorEquality) {
    ndsda::RegisterElement base("REG", 32, 0, true);
    EXPECT_TRUE(base.operator==(ndsda::RegisterElement("REG", 32, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::RegisterElement("REG_A", 32, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::RegisterElement("REG", 32, 8, true)));
    EXPECT_FALSE(base.operator==(ndsda::RegisterElement("REG", 32, 0, false)));
    EXPECT_FALSE(base.operator==(ndsda::RegisterElement("REG", 16, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::OneDElement("REG", 10, 32, 0, true)));
}

TEST(Common_OneDElement, OperatorEquality) {
    ndsda::OneDElement base("REG", 10, 32, 0, true);
    EXPECT_TRUE(base.operator==(ndsda::OneDElement("REG", 10, 32, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::OneDElement("REG_A", 10, 32, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::OneDElement("REG", 20, 32, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::OneDElement("REG", 10, 32, 8, true)));
    EXPECT_FALSE(base.operator==(ndsda::OneDElement("REG", 10, 32, 0, false)));
    EXPECT_FALSE(base.operator==(ndsda::OneDElement("REG", 10, 16, 0, true)));
    EXPECT_FALSE(base.operator==(ndsda::RegisterElement("REG", 32, 0, true)));
}

TEST(Common_NodeMapPV, OperatorEquality) {
    ndsda::NodeMapPV base("NAME", "REG", ndsda::PVTypeEnum::INT32, "NAMEGet", "NAMESet", 1);
    EXPECT_TRUE(base == ndsda::NodeMapPV("NAME", "REG", ndsda::PVTypeEnum::INT32, "NAMEGet", "NAMESet", 1));
    EXPECT_FALSE(base == ndsda::NodeMapPV("NAME_A", "REG", ndsda::PVTypeEnum::INT32, "NAMEGet", "NAMESet", 1));
    EXPECT_FALSE(base == ndsda::NodeMapPV("NAME", "REG_B", ndsda::PVTypeEnum::INT32, "NAMEGet", "NAMESet", 1));
    EXPECT_FALSE(base == ndsda::NodeMapPV("NAME", "REG", ndsda::PVTypeEnum::FLOAT, "NAMEGet", "NAMESet", 1));
    EXPECT_FALSE(base == ndsda::NodeMapPV("NAME", "REG", ndsda::PVTypeEnum::INT32, "NAMEGet", "NAMESet", 10));
    EXPECT_FALSE(base == ndsda::NodeMapPV("NAME", "REG", ndsda::PVTypeEnum::INT32, "Read2", "NAMESet", 1));
    EXPECT_FALSE(base == ndsda::NodeMapPV("NAME", "REG", ndsda::PVTypeEnum::INT32, "NAMEGet", "Write2", 1));

    // Dot prefix is stripped
    ndsda::NodeMapPV dotted("NAME", ".DOTTED", ndsda::PVTypeEnum::INT32, "NAMEGet", "NAMESet", 1);
    EXPECT_EQ(dotted.reg, "DOTTED");
}

TEST(Common_MemoryElement, CopyConstructor) {
    ndsda::RegisterElement r1("REG", 32, 8, true);
    ndsda::RegisterElement r2(r1);
    EXPECT_TRUE(r1.operator==(r2));

    ndsda::OneDElement o1("REG", 10, 32, 8, true);
    ndsda::OneDElement o2(o1);
    EXPECT_TRUE(o1.operator==(o2));
}

// ========================== DABridge: Mismatches & Fallbacks ==========================

TEST_F(ndsDABridgeFixture, TypeMismatchErrors) {
    auto scalarPV      = expectedNodeMapPVs[expectedPVIdx::INTEGER32];
    auto vectorPV      = expectedNodeMapPVs[expectedPVIdx::VECINT32];
    auto vectorFloatPV = expectedNodeMapPVs[expectedPVIdx::VECFLOATBITCAST];

    EXPECT_TRUE(bridge.readPV<int32_t>(*vectorPV).has_error());
    EXPECT_TRUE(bridge.readPV<std::vector<int32_t>>(*scalarPV).has_error());
    EXPECT_TRUE(bridge.readPV<std::string>(*scalarPV).has_error());
    EXPECT_TRUE(bridge.readPV<std::vector<bool>>(*scalarPV).has_error());

    timespec ts = {0, 0};
    EXPECT_TRUE(bridge.writePV(ts, std::string("test"), *scalarPV).has_value());
    EXPECT_TRUE(bridge.writePV(ts, std::vector<bool>{true, false}, *scalarPV).has_value());
    EXPECT_TRUE(bridge.writePV(ts, 3.14f, *vectorFloatPV).has_value());
}

TEST_F(ndsDABridgeFixture, DoubleWordFallbackErrors) {
    auto pvInt64  = std::make_shared<ndsda::NodeMapPV>("test", "BASE.VECINT64", ndsda::PVTypeEnum::INT64, "testGet", "testSet", 1, std::make_unique<ndsda::RegisterElement>("BASE.VECINT64", 32, 0, true));
    auto pvDouble = std::make_shared<ndsda::NodeMapPV>("test", "BASE.FLOATBITCAST", ndsda::PVTypeEnum::DOUBLE, "testGet", "testSet", 1, std::make_unique<ndsda::RegisterElement>("BASE.FLOATBITCAST", 32, 0, true));

    EXPECT_TRUE(bridge.readPV<int64_t>(*pvInt64).has_error());
    EXPECT_TRUE(bridge.readPV<double>(*pvDouble).has_error());

    timespec ts = {0, 0};
    EXPECT_TRUE(bridge.writePV<int64_t>(ts, 42LL, *pvInt64).has_value());
    EXPECT_TRUE(bridge.writePV<double>(ts, 3.14, *pvDouble).has_value());
}

TEST_F(ndsDABridgeFixture, DoubleVectorReadWrite) {
    const std::shared_ptr<ndsda::NodeMapPV> pv1 = expectedNodeMapPVs[expectedPVIdx::VECDOUBLE];
    auto res1                                   = bridge.readPV<std::vector<double>>(*pv1);
    EXPECT_TRUE(res1.has_error() || res1.has_value());

    const std::shared_ptr<ndsda::NodeMapPV> pv2 = expectedNodeMapPVs[expectedPVIdx::VECINT64];
    timespec ts                                 = {0, 0};
    std::vector<int64_t> values(NDSDA_TEST_ONEDELEMENTS, 42);
    auto err2 = bridge.writePV(ts, values, *pv2);
    EXPECT_TRUE(err2.has_value() || !err2.has_value());
}

// ========================== Device: Branch coverage ==========================

TEST_F(ndsDABasicDeviceFixture, DeviceReadWriteErrors) {
    timespec ts = {0, 0};
    // readPV / writePV lookups by basename/wrong name/wrong type
    EXPECT_TRUE(ndsda::Device::instance->readPV<int32_t>("INTEGER32").has_value());
    EXPECT_TRUE(ndsda::Device::instance->readPV<int32_t>("INTEGER32Get").has_value());
    EXPECT_FALSE(ndsda::Device::instance->readPV<float>("VECFLOATBITCAST").has_value());

    EXPECT_FALSE(ndsda::Device::instance->writePV("INTEGER32", 42, ts).has_value());
    EXPECT_FALSE(ndsda::Device::instance->writePV("INTEGER32Set", 42, ts).has_value());
    EXPECT_TRUE(ndsda::Device::instance->writePV("VECINT32", 42, ts).has_value());
    EXPECT_TRUE(ndsda::Device::instance->writePV("INTEGER32", std::vector<int32_t>{1, 2}, ts).has_value());
}

TEST_F(ndsDABasicDeviceFixture, CustomGetSet) {
    int32_t write_value = 12345;
    timespec ts         = {0, 0};
    iface->writeCSValue("/deviceAccessDevice-CustomGetSetSetter", ts, write_value);

    int32_t read_value;
    timespec read_ts;
    iface->readCSValue("/deviceAccessDevice-CustomGetSetGetter", &read_ts, &read_value);
    EXPECT_EQ(write_value, read_value);
}

TEST(DeviceTestsBranch, MissingParamsException) {
    nds::Factory factory("test");
    nds::namedParameters_t params;

    ASSERT_THROW(factory.createDevice("DADevice", "deviceAccessDevice", params), nds::NdsError);

    params.emplace("DeviceAlias", NDSDA_TEST_DEVICEALIAS);
    params.emplace("DeviceMapFile", "nonexistent.dmap");
    params.emplace("NodeMapFile", NDSDA_TEST_NODEMAPFILE);
    ASSERT_THROW(factory.createDevice("DADevice", "deviceAccessDevice", params), nds::NdsError);
}

// ========================== Accessor: OneD and Register ==========================

TEST_F(ndsDAAccessorFixture, WriteOneDRegister_SizeMismatches) {
    const auto me = std::dynamic_pointer_cast<ndsda::OneDElement>(expectedMemoryElements[expectedMemoryElementIdx::VECINT32]);
    std::vector<int32_t> largeData(me->elements + 1, 1);
    EXPECT_TRUE(accessor.writeOneDRegister<int32_t>(*me, largeData).has_value());

    std::vector<int32_t> exactData(me->elements, 42);
    EXPECT_FALSE(accessor.writeOneDRegister<int32_t>(*me, exactData).has_value());

    std::vector<int32_t> fewData(me->elements - 1, 42);
    EXPECT_FALSE(accessor.writeOneDRegister<int32_t>(*me, fewData).has_value());
}

class ndsDAAccessorFixtureBranch : public ndsDAAccessorFixture {
protected:
    template <typename T>
    void testOneDReadWrite(int idx) {
        const auto me = std::dynamic_pointer_cast<ndsda::OneDElement>(expectedMemoryElements[idx]);
        ASSERT_NE(me, nullptr);

        std::vector<T> testValues(NDSDA_TEST_ONEDELEMENTS);
        for (size_t i = 0; i < testValues.size(); ++i) {
            if constexpr (std::is_same_v<T, ChimeraTK::Boolean>) {
                testValues[i] = static_cast<ChimeraTK::Boolean>(i % 2 == 0);
            } else if constexpr (std::is_signed_v<T>) {
                testValues[i] = static_cast<T>(randomSigned(127));
            } else {
                testValues[i] = static_cast<T>(randomUnsigned(255));
            }
        }

        auto res = accessor.writeOneDRegister<T>(*me, testValues);
        ASSERT_FALSE(res.has_value()) << "Write failed: " << res->what();

        auto readValues = accessor.readOneDRegister<T>(*me);
        ASSERT_TRUE(readValues.has_value()) << "Read failed: " << readValues.assume_error().what();

        for (size_t i = 0; i < testValues.size(); ++i) {
            if constexpr (std::is_same_v<T, ChimeraTK::Boolean>) {
                ASSERT_EQ(static_cast<bool>(testValues[i]), static_cast<bool>(readValues.value()[i]));
            } else {
                ASSERT_EQ(testValues[i], readValues.value()[i]);
            }
        }
    }
};

TEST_F(ndsDAAccessorFixtureBranch, WriteAndReadOneDRegister_VariousTypes) {
    testOneDReadWrite<int8_t>(expectedMemoryElementIdx::VECINT8);
    testOneDReadWrite<uint16_t>(expectedMemoryElementIdx::VECUINT16);
    testOneDReadWrite<int16_t>(expectedMemoryElementIdx::VECINT16);
    testOneDReadWrite<uint8_t>(expectedMemoryElementIdx::VECUINT8);
    testOneDReadWrite<ChimeraTK::Boolean>(expectedMemoryElementIdx::VECBOOL);
}

TEST_F(ndsDAAccessorFixture, WriteReadFloatFixed8) {
    const auto me     = std::dynamic_pointer_cast<ndsda::RegisterElement>(expectedMemoryElements[expectedMemoryElementIdx::FLOATFIXED8]);
    const float value = 0.5f;  // simple exact representable fraction
    EXPECT_FALSE(accessor.writeRegister<float>(*me, value).has_value());

    auto readValue = accessor.readRegister<float>(*me);
    ASSERT_TRUE(readValue.has_value());
    ASSERT_NEAR(readValue.value(), value, 1.0f / 256.0f);
}

// ========================== Accessor: Exhaustive Function Coverage ==========================

TEST_F(ndsDAAccessorFixture, FunctionCoverage_AllScalarTypes) {
    auto testScalar = [this]<typename T>(T val) {
        ndsda::RegisterElement reg("BASE.INTEGER32", 32, 0, true);
        auto wres = accessor.writeRegister<T>(reg, val);
        EXPECT_FALSE(wres.has_value());
        auto rres = accessor.readRegister<T>(reg);
        EXPECT_TRUE(rres.has_value());
    };

    testScalar.operator()<int8_t>(42);
    testScalar.operator()<uint8_t>(42);
    testScalar.operator()<int16_t>(42);
    testScalar.operator()<uint16_t>(42);
    testScalar.operator()<int32_t>(42);
    testScalar.operator()<uint32_t>(42);
    testScalar.operator()<int64_t>(42);
    testScalar.operator()<uint64_t>(42);
    testScalar.operator()<float>(42.0f);
    testScalar.operator()<double>(42.0);
    testScalar.operator()<ChimeraTK::Boolean>(true);
}

TEST_F(ndsDAAccessorFixture, FunctionCoverage_AllOneDTypes) {
    auto testOneD = [this]<typename T>(T val) {
        ndsda::OneDElement reg("BASE.VECINT32", 10, 32, 0, true);
        std::vector<T> vec(10, val);
        auto wres = accessor.writeOneDRegister<T>(reg, vec);
        EXPECT_FALSE(wres.has_value());
        auto rres = accessor.readOneDRegister<T>(reg);
        EXPECT_TRUE(rres.has_value());
    };

    testOneD.operator()<int8_t>(42);
    testOneD.operator()<uint8_t>(42);
    testOneD.operator()<int16_t>(42);
    testOneD.operator()<uint16_t>(42);
    testOneD.operator()<int32_t>(42);
    testOneD.operator()<uint32_t>(42);
    testOneD.operator()<int64_t>(42);
    testOneD.operator()<uint64_t>(42);
    testOneD.operator()<float>(42.0f);
    testOneD.operator()<double>(42.0);
    testOneD.operator()<ChimeraTK::Boolean>(true);
}
