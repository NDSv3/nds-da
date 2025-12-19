#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include <memory>
#include "nds-da-test-fixture.hpp"

TEST(FileParser, wrongFilePath_exception) {
    const std::string wrongFilePath = "non_existing_file.dmap";
    const std::string deviceAlias   = NDSDA_TEST_DEVICEALIAS;

    EXPECT_THROW({ ndsda::FileParser parser(wrongFilePath, NDSDA_TEST_DEVICEALIAS); }, std::runtime_error);
}

TEST(FileParser, wrongFileFormat_exception) {
    const std::string wrongFilePath = NDSDA_TEST_MMAPFILE;  // Using mmap instead of dmap
    const std::string deviceAlias   = NDSDA_TEST_DEVICEALIAS;

    EXPECT_THROW({ ndsda::FileParser parser(wrongFilePath, NDSDA_TEST_DEVICEALIAS); }, std::runtime_error);
}

TEST(FileParser, missingDeviceAlias_exception) {
    const std::string deviceMapFile    = NDSDA_TEST_DMAPFILE;
    const std::string wrongDeviceAlias = "non_existing_device";

    EXPECT_THROW({ ndsda::FileParser parser(deviceMapFile, wrongDeviceAlias); }, std::runtime_error);
}

TEST_F(ndsDAFileParserFixture, getMemoryMapFile) {
    ASSERT_EQ(parser->getMemoryMapFile(), NDSDA_TEST_MMAPFILE);
}

static const std::string elementToString(const ndsda::MemoryElement& element) {
    std::string repr = "Element Name: " + element.name + "\n";
    repr += "  Read PV: " + element.readPV + "\n";
    repr += "  Write PV: " + element.writePV + "\n";

    if (element.getType() == ndsda::MemoryElementType::SCALAR) {
        repr += "  Type: Scalar Register";
    } else if (element.getType() == ndsda::MemoryElementType::ONE_D) {
        const ndsda::OneDElement& oneDElement = static_cast<const ndsda::OneDElement&>(element);
        repr += "  Type: One-D Register\n";
        repr += "  Elements: " + std::to_string(oneDElement.elements) + "\n";
        repr += "  Element Size: " + std::to_string(oneDElement.elementSize) + " bytes";
    }

    return repr;
}
TEST_F(ndsDAFileParserFixture, elementParsing) {
    auto vec = parser->getElements();

    for (size_t i = 0; i < expectedElements.size(); ++i) {
        ASSERT_EQ(*vec[i], *expectedElements[i])
            << "Parsed element does not match expected element:\n"
            << "Parsed: "
            << elementToString(*vec[i]) << "\n"
            << "Expected: "
            << elementToString(*expectedElements[i]) << "\n";
    }
}