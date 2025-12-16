#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <ctime>
#include "nds-da-test-fixture.hpp"

TEST(FileParser, wrongFilePath_exception) {
    const std::string wrongFilePath = "non_existing_file.dmap";
    const std::string deviceAlias   = NDSDA_TEST_DEVICEALIAS;

    EXPECT_THROW({ ndsda::FileParser parser(wrongFilePath, NDSDA_TEST_DEVICEALIAS); }, std::runtime_error);
}

TEST(FileParser, wrongFileFormat_exception) {
    const std::string wrongFilePath = NDSDA_TEST_MEMORY_MAPFILE;  // Using mmap instead of dmap
    const std::string deviceAlias   = NDSDA_TEST_DEVICEALIAS;

    EXPECT_THROW({ ndsda::FileParser parser(wrongFilePath, NDSDA_TEST_DEVICEALIAS); }, std::runtime_error);
}

TEST(FileParser, missingDeviceAlias_exception) {
    const std::string deviceMapFile    = NDSDA_TEST_DEVICE_MAPFILE;
    const std::string wrongDeviceAlias = "non_existing_device";

    EXPECT_THROW({ ndsda::FileParser parser(deviceMapFile, wrongDeviceAlias); }, std::runtime_error);
}

TEST_F(ndsDAFileParserFixture, TestTest) {
    ASSERT_EQ(parser->getMemoryMapFile(), ndsDAFileParserFixture::MEMORY_MAPFILE);
}