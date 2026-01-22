#ifndef NDS_DA_FILE_PARSER
#define NDS_DA_FILE_PARSER

#include <filesystem>
#include <string>
#include "ndsdaCommon.hpp"

namespace ndsda {

namespace FileParser {

Result<std::string, ndsdaError> getMemoryMapFile(
    const std::filesystem::path& deviceMapFile,
    const std::string& deviceMapAlias
);

Result<std::vector<std::unique_ptr<MemoryElement>>, ndsdaError> parseMemoryMapFile(
    const std::filesystem::path& memoryMapFile
);

Result<std::vector<std::shared_ptr<NodeMapPV>>, ndsdaError> parseNodeMapFile(
    const std::filesystem::path& nodeMapFile
);

}  // namespace FileParser

}  // namespace ndsda

#endif  //! NDS_DA_FILE_PARSER
