/**
 * @file ndsdaFileParser.hpp
 * @brief NDS-DA File parsing utilities
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef NDS_DA_FILE_PARSER
#define NDS_DA_FILE_PARSER

#include <filesystem>
#include <string>
#include "ndsdaCommon.hpp"

namespace ndsda {

namespace FileParser {

/**
 * @brief Resolve the memory map file path from a device map file and alias
 * @param deviceMapFile Path to the device map (.dmap)
 * @param deviceMapAlias Device alias to resolve
 * @return Result<std::string> Resolved memory map file path or an error
 */
Result<std::string> getMemoryMapFile(
    const std::filesystem::path& deviceMapFile,
    const std::string& deviceMapAlias
);

/**
 * @brief Obtain the memory elements (abstractions over ChimeraTK registers) from a memory map file
 * @param memoryMapFile Path to the memory map (.mmap)
 * @return Result<std::vector<std::unique_ptr<MemoryElement>>> Parsed elements or an error
 */
Result<std::vector<std::unique_ptr<MemoryElement>>> parseMemoryMapFile(
    const std::filesystem::path& memoryMapFile
);

/**
 * @brief Parse a node map file into PV definitions
 * @param nodeMapFile Path to the node map XML
 * @return Result<std::vector<std::shared_ptr<NodeMapPV>>> Parsed PVs or an error
 */
Result<std::vector<std::shared_ptr<NodeMapPV>>> parseNodeMapFile(
    const std::filesystem::path& nodeMapFile
);

}  // namespace FileParser

}  // namespace ndsda

#endif  //! NDS_DA_FILE_PARSER
