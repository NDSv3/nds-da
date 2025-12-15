#ifndef NDS_DA_FILE_PARSER
#define NDS_DA_FILE_PARSER

#include <string>
#include "ndsdaMemoryElements.hpp"

namespace ndsda {

class FileParser {
public:
    FileParser(std::string deviceMapFile, std::string deviceMapAlias);
    ~FileParser();

    const std::string& getMemoryMapFile() const;

private:
    std::string deviceMapFile, deviceMapAlias, memoryMapFile;
};

}  // namespace ndsda

#endif  //! NDS_DA_FILE_PARSER
