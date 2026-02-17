#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include <memory>
#include "ndsda-test-common.hpp"
#include "ndsda-test-constants.hpp"
#include "ndsdaFileParser.hpp"
#include "ndsdaCommon.hpp"

/* ================================= UTILITY FUNCTIONS ======================================= */
static const std::string memoryElement_toString(const ndsda::MemoryElement& element);
static const std::string memoryMapPV_toString(const ndsda::NodeMapPV& nodeMapPv);

/* ================================= DEVICE FILE TESTS ======================================= */

TEST(FileParser, deviceFile_InvalidPath_exception) {
    const std::string wrongFilePath = "non_existing_file.dmap";
    const std::string deviceAlias   = NDSDA_TEST_DEVICEALIAS;

    EXPECT_TRUE(ndsda::FileParser::getMemoryMapFile(wrongFilePath, deviceAlias).has_error());
}

/* ================================= MEMORY MAP TESTS ======================================= */

TEST(FileParser, memoryMap_InvalidPath_exception) {
    const std::string wrongFilePath = "non_existing_file.mmap";

    EXPECT_TRUE(ndsda::FileParser::parseMemoryMapFile(wrongFilePath).has_error());
}

TEST(FileParser, memoryMap_wrongFileFormat_exception) {
    const std::string memoryMapFile = NDSDA_TEST_MMAPFILE;  // Using mmap instead of dmap
    const std::string deviceAlias   = NDSDA_TEST_DEVICEALIAS;

    EXPECT_TRUE(ndsda::FileParser::getMemoryMapFile(memoryMapFile, deviceAlias).has_error());
}

TEST(FileParser, memoryMap_missingDeviceAlias_exception) {
    const std::string deviceMapFile    = NDSDA_TEST_DMAPFILE;
    const std::string wrongDeviceAlias = "non_existing_device";

    EXPECT_TRUE(ndsda::FileParser::getMemoryMapFile(deviceMapFile, wrongDeviceAlias).has_error());
}

TEST(FileParser, memoryMap_validFile_correctElements) {
    auto elements = ndsda::FileParser::parseMemoryMapFile(NDSDA_TEST_MMAPFILE);
    ASSERT_TRUE(elements.has_value()) << "Failed to parse valid memory map file: " << elements.assume_error().what();

    auto& vec = elements.assume_value();
    for (size_t i = 0; i < expectedMemoryElements.size(); ++i) {
        ASSERT_EQ(*vec[i], *expectedMemoryElements[i])
            << "Parsed element does not match expected element:\n"
            << "Parsed: "
            << memoryElement_toString(*vec[i]) << "\n"
            << "Expected: "
            << memoryElement_toString(*expectedMemoryElements[i]) << "\n";
    }
}

/* ================================ NODEMAP FILE TESTS ===================================== */

TEST(FileParser, nodeMapFile_InvalidPath_exception) {
    const std::string wrongFilePath = "non_existing_file.xml";

    EXPECT_TRUE(ndsda::FileParser::parseNodeMapFile(wrongFilePath).has_error());
}

TEST(FileParser, nodeMapFile_invalidFormat_exception) {
    const std::string wrongFilePath = NDSDA_TEST_DMAPFILE;  // Using dmap instead of xml

    EXPECT_TRUE(ndsda::FileParser::parseNodeMapFile(wrongFilePath).has_error());
}

TEST(FileParser, nodeMapFile_malformedXML_exception) {
    const std::string wrongFilePath = NDSDA_TEST_INVALIDXML;

    EXPECT_TRUE(ndsda::FileParser::parseNodeMapFile(wrongFilePath).has_error());
}

TEST(FileParser, nodeMapFile_validFile_correctPVs) {
    const std::string wrongFilePath = NDSDA_TEST_NODEMAPFILE;

    auto pvs = ndsda::FileParser::parseNodeMapFile(NDSDA_TEST_NODEMAPFILE);
    ASSERT_TRUE(pvs.has_value()) << "Failed to parse valid node map file: " << pvs.assume_error().what();

    auto& pvsVector = pvs.assume_value();
    for (size_t i = 0; i < expectedNodeMapPVs.size(); ++i) {
        ASSERT_EQ(*pvsVector[i], *expectedNodeMapPVs[i])
            << "Parsed PV does not match expected PV:\n"
            << "Parsed: " << memoryMapPV_toString(*pvsVector[i]) << "\n"
            << "Expected: " << memoryMapPV_toString(*expectedNodeMapPVs[i]) << "\n";
    }
}

TEST(FileParser, nodeMapFile_PVWithInvalidReadWritePV_exception) {
    auto pvs = ndsda::FileParser::parseNodeMapFile(NDSDA_TEST_INVALIDPVXML);
    ASSERT_TRUE(pvs.has_error()) << "Parsing should have failed due to invalid PV read/write names, but it succeeded.";
}

static const std::string memoryElement_toString(const ndsda::MemoryElement& element) {
    std::string repr = element.name + "\n";

    if (element.getType() == ndsda::MemoryElementType::SCALAR) {
        repr += "  Type: Scalar Register\n";
    } else if (element.getType() == ndsda::MemoryElementType::ONE_D) {
        const ndsda::OneDElement& oneDElement = static_cast<const ndsda::OneDElement&>(element);
        repr += "  Type: One-D Register\n";
        repr += "  Elements: " + std::to_string(oneDElement.elements) + "\n";
    }
    repr += "  Element Size: " + std::to_string(element.elementWidthBits) + " bytes\n";
    repr += "  Fractional Bits: " + std::to_string(element.fracBits) + "\n";
    repr += "  Signed: " + std::string(element.isSigned ? "true" : "false") + "\n";

    return repr;
}

static const inline std::string typeEnumToString(ndsda::PVTypeEnum type) {
    using E = ndsda::PVTypeEnum;
    // clang-format off
    switch (type) {
        case E::INT32:      return "int32";
        case E::INT64:      return "int64";
        case E::FLOAT:      return "float";
        case E::DOUBLE:     return "double";
        case E::STRING:     return "string";
        case E::VEC_BOOL:   return "Vec<bool>";
        case E::VEC_UINT8:  return "Vec<uint8>";
        case E::VEC_UINT16: return "Vec<uint16>";
        case E::VEC_UINT32: return "Vec<uint32>";
        case E::VEC_INT8:   return "Vec<int8>";
        case E::VEC_INT16:  return "Vec<int16>";
        case E::VEC_INT32:  return "Vec<int32>";
        case E::VEC_INT64:  return "Vec<int64>";
        case E::VEC_FLOAT:  return "Vec<float>";
        case E::VEC_DOUBLE: return "Vec<double>";
    }
    // clang-format on
}
static const std::string memoryMapPV_toString(const ndsda::NodeMapPV& nodeMapPv) {
    std::string repr = "PV Name: " + nodeMapPv.name + "\n";
    repr += "  Address: " + nodeMapPv.reg + "\n";
    repr += "  Type: " + typeEnumToString(nodeMapPv.type) + "\n";
    repr += "  Number of Elements: " + std::to_string(nodeMapPv.nElem) + "\n";
    repr += "  Read PV: " + nodeMapPv.readPV + "\n";
    repr += "  Write PV: " + nodeMapPv.writePV + "\n";
    return repr;
}