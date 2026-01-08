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

    if (!std::filesystem::exists(this->deviceMapFile)) {
        throw nds::NdsError("Device map file does not exist: " + deviceMapFile);
    }

    std::string memoryMapFile_str;
    {  // Scope for deviceMap RAII
        std::ifstream deviceMap(this->deviceMapFile);

        if (!deviceMap.is_open()) {
            throw nds::NdsError("Could not open device map file: " + deviceMapFile);
        }

        for (std::string line; std::getline(deviceMap, line);) {
            if (line.rfind(deviceMapAlias, 0) == 0) {
                int mapLocation   = line.rfind("map=") + 4;
                memoryMapFile_str = line.substr(
                    line.rfind("map=") + 4,
                    line.find_first_of("&)", mapLocation) - mapLocation
                );
                break;
            }
        }
    }

    if (memoryMapFile_str.empty()) {
        throw nds::NdsError("Could not find memory map for alias: " + this->deviceMapAlias);
    }

    this->memoryMapFile = this->deviceMapFile.remove_filename() / std::filesystem::path(memoryMapFile_str);

    if (!std::filesystem::exists(this->memoryMapFile)) {
        throw nds::NdsError("Memory map file does not exist: " + this->memoryMapFile.string());
    }

    this->elements = parseRegisters(this->memoryMapFile);
}

FileParser::~FileParser() {}

const std::string FileParser::getMemoryMapFile() const noexcept {
    return this->memoryMapFile;
}

const std::vector<std::shared_ptr<ndsda::MemoryElement>> FileParser::getElements() const noexcept {
    return this->elements;
}

static std::vector<std::shared_ptr<ndsda::MemoryElement>> parseRegisters(const std::string& memoryMapFile) {
    std::vector<std::shared_ptr<ndsda::MemoryElement>> registers;
    std::ifstream memMap(memoryMapFile);

    if (!memMap.is_open()) {
        throw std::runtime_error("Could not open memory map file: " + memoryMapFile);
    }

    std::string path, address, BAR, AccMode;
    size_t nElements, nBytes, width, fixed, sgnd;
    for (std::string line; std::getline(memMap, line);) {
        if (line.empty() || line[0] == '#' || line[0] == '@')
            continue;

        std::istringstream iss(line);
        if (!(iss >> path >> nElements >> address >> nBytes >> BAR >> width >> fixed >> sgnd >> AccMode)) {
            continue;  // TODO: Handle Malformed line
        }

        if (nElements == 1) {
            registers.push_back(std::make_shared<ndsda::RegisterElement>(path, fixed, bool(sgnd)));
        } else {
            registers.push_back(
                std::make_shared<ndsda::OneDElement>(path, nElements, width, fixed, bool(sgnd))
            );
        }
    }

    return registers;
}