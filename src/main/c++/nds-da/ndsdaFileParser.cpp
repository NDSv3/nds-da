#include "ndsdaFileParser.hpp"
#include <fstream>
#include <iostream>
#include <string>

using ndsda::FileParser;

FileParser::FileParser(std::string deviceMapFile, std::string deviceMapAlias)
    : deviceMapFile(deviceMapFile),
      deviceMapAlias(deviceMapAlias) {

    std::ifstream deviceMap(this->deviceMapFile);

    if (!deviceMap.is_open()) {
        throw std::runtime_error("Could not open device map file: " + this->deviceMapFile);
    }

    for (std::string line; std::getline(deviceMap, line);) {
        std::cout << line << std::endl;
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

const std::string& FileParser::getMemoryMapFile() const {
    return this->memoryMapFile;
}