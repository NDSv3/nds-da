#ifndef NDS_DA_FILE_PARSER
#define NDS_DA_FILE_PARSER

#include <filesystem>
#include <memory>
#include <string>
#include "ndsdaMemoryElements.hpp"

namespace ndsda {

class FileParser {
public:
    FileParser(std::string deviceMapFile, std::string deviceMapAlias);
    ~FileParser();

    const std::string getMemoryMapFile() const;
    std::vector<std::shared_ptr<MemoryElement>> getElements() const;

private:
    std::filesystem::path deviceMapFile, memoryMapFile;
    std::string deviceMapAlias;
    std::vector<std::shared_ptr<MemoryElement>> elements;
};

}  // namespace ndsda

#endif  //! NDS_DA_FILE_PARSER
