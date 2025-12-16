#include "ndsdaFileParser.hpp"
#include <fstream>
#include <string>
#include <vector>
#include "nds3/exceptions.h"
#include "ndsdaMemoryElements.hpp"

using ndsda::FileParser;

static std::vector<std::shared_ptr<ndsda::MemoryElement>> parseRegisters(const std::string& memoryMapFile);

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

    this->elements = parseRegisters(this->memoryMapFile);
}

FileParser::~FileParser() {}

const std::string& FileParser::getMemoryMapFile() const {
    return this->memoryMapFile;
}

std::vector<std::shared_ptr<ndsda::MemoryElement>> FileParser::getElements() const {
    return this->elements;
}

static std::vector<std::shared_ptr<ndsda::MemoryElement>> parseRegisters(const std::string& memoryMapFile) {
    std::vector<std::shared_ptr<ndsda::MemoryElement>> registers;
    std::ifstream memMap(memoryMapFile);

    if (!memMap.is_open()) {
        throw std::runtime_error("Could not open memory map file: " + memoryMapFile);
    }

    std::string path, address, BAR /*, fixed, sgnd, AccMode */;
    size_t nElements, nBytes, width;
    for (std::string line; std::getline(memMap, line);) {
        if (line.empty() || line[0] == '#' || line[0] == '@')
            continue;

        std::istringstream iss(line);
        if (!(iss >> path >> nElements >> address >> nBytes >> BAR >> width /* >> fixed >> sgnd >> AccMode*/)) {
            continue;  // TODO: Handle Malformed line
        }

        if (nElements == 1) {
            registers.push_back(std::make_shared<ndsda::RegisterElement>(path));
        } else {
            registers.push_back(std::make_shared<ndsda::OneDElement>(path, nElements, width));
        }
    }

    return registers;
}