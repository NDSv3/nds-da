#include "ndsdaFileParser.hpp"
#include <fstream>
#include <string>
#include "nds3/exceptions.h"

using ndsda::FileParser;

FileParser::FileParser(std::string deviceMapFile, std::string deviceMapAlias)
    : deviceMapFile(deviceMapFile),
      deviceMapAlias(deviceMapAlias) {

    {  // Scope for deviceMap RAII
        std::ifstream deviceMap(this->deviceMapFile);

        if (!deviceMap.is_open()) {
            throw std::runtime_error("Could not open device map file: " + this->deviceMapFile);
        }

        for (std::string line; std::getline(deviceMap, line);) {
            if (line.rfind(this->deviceMapAlias, 0) == 0) {
                int mapLocation = line.rfind("map=") + 4;
                memoryMapFile   = line.substr(
                    line.rfind("map=") + 4,
                    line.find_first_of("&)", mapLocation) - mapLocation
                );
                break;
            }
        }
    }

    if (memoryMapFile.empty()) {
        throw nds::NdsError("Could not find memory map for alias: " + this->deviceMapAlias);
    }
}

const std::string& FileParser::getMemoryMapFile() const {
    return this->memoryMapFile;
}

FileParser::~FileParser() {}