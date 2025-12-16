#ifndef NDS_DA_FILE_PARSER
#define NDS_DA_FILE_PARSER

#include <memory>
#include <string>
#include "ndsdaMemoryElements.hpp"

namespace ndsda {

class FileParser {
public:
    FileParser(std::string deviceMapFile, std::string deviceMapAlias);
    ~FileParser();

    const std::string& getMemoryMapFile() const;
    std::vector<std::shared_ptr<MemoryElement>> getElements() const;

private:
    std::string deviceMapFile, deviceMapAlias, memoryMapFile;
    std::vector<std::shared_ptr<MemoryElement>> elements;
};

}  // namespace ndsda

#endif  //! NDS_DA_FILE_PARSER
