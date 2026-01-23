#include "ndsdaFileParser.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/outcome/try.hpp>

#include "ndsdaCommon.hpp"

using ndsda::Result;

static Result<std::shared_ptr<ndsda::NodeMapPV>, ndsda::ndsdaError> parsePV(const boost::property_tree::ptree& pvTree);

Result<std::string, ndsda::ndsdaError> ndsda::FileParser::getMemoryMapFile(
    const std::filesystem::path& deviceMapFile,
    const std::string& deviceMapAlias
) {
    if (!std::filesystem::exists(deviceMapFile)) {
        std::cerr << "Device map file does not exist: " << deviceMapFile << std::endl;
        return ndsda::ndsdaError("Device map file does not exist: " + deviceMapFile.string());
    }

    std::string memoryMapFile_str;
    {  // Scope for deviceMap RAII
        std::ifstream deviceMap(deviceMapFile);

        if (!deviceMap.is_open()) {
            return ndsda::ndsdaError("Could not open device map file: " + deviceMapFile.string());
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
        return ndsda::ndsdaError("Could not find memory map for alias: " + deviceMapAlias);
    }

    std::filesystem::path memoryMapFile = std::filesystem::path(deviceMapFile).remove_filename() / std::filesystem::path(memoryMapFile_str);

    if (!std::filesystem::exists(memoryMapFile)) {
        return ndsda::ndsdaError("Memory map file does not exist: " + memoryMapFile.string());
    }

    return memoryMapFile.string();
}

Result<std::vector<std::unique_ptr<ndsda::MemoryElement>>, ndsda::ndsdaError> ndsda::FileParser::parseMemoryMapFile(const std::filesystem::path& memoryMapFile) {
    std::vector<std::unique_ptr<ndsda::MemoryElement>> registers;
    std::ifstream memMap(memoryMapFile);

    if (!memMap.is_open()) {
        return ndsda::ndsdaError("Could not open memory map file: " + memoryMapFile.string());
    }

    std::string path, address, BAR, AccMode;
    size_t nElements, nBytes, width, fixed, sgnd;
    for (std::string line; std::getline(memMap, line);) {
        if (line.empty() || line[0] == '#' || line[0] == '@')
            continue;

        std::istringstream iss(line);
        if (!(iss >> path >> nElements >> address >> nBytes >> BAR >> width >> fixed >> sgnd >> AccMode)) {
            return ndsda::ndsdaError("Malformed line in memory map file: " + line);
        }

        if (nElements == 1) {
            registers.push_back(std::make_unique<ndsda::RegisterElement>(path, fixed, bool(sgnd)));
        } else {
            registers.push_back(
                std::make_unique<ndsda::OneDElement>(path, nElements, width, fixed, bool(sgnd))
            );
        }
    }

    return registers;
}

Result<std::vector<std::shared_ptr<ndsda::NodeMapPV>>, ndsda::ndsdaError> ndsda::FileParser::parseNodeMapFile(
    const std::filesystem::path& nodeMapFile
) {
    boost::property_tree::ptree pt;

    if (!std::filesystem::exists(nodeMapFile)) {
        return ndsda::ndsdaError("Node map file does not exist: " + nodeMapFile.string());
    }

    try {
        boost::property_tree::read_xml(nodeMapFile, pt);
    } catch (const boost::property_tree::xml_parser_error& e) {
        return ndsda::ndsdaError("Failed to parse node map file: " + std::string(e.what()));
    }

    auto nodeMap = pt.get_child_optional("ndsda:NodeMap");
    if (!nodeMap) return ndsda::ndsdaError("Invalid node map file: Missing NodeMap root element.");

    auto pvMap = nodeMap->get_child_optional("ndsda:PVMap");
    if (!pvMap) return ndsda::ndsdaError("Invalid node map file: Missing PVMap element.");

    std::vector<std::shared_ptr<ndsda::NodeMapPV>> nodeMapPVs;
    for (auto& pv : pvMap.get()) {
        if (pv.first != "ndsda:PV")
            return ndsda::ndsdaError("Invalid node map file: Unexpected element '" + pv.first + "' in PVMap");

        BOOST_OUTCOME_TRY(auto res, parsePV(pv.second));
        nodeMapPVs.push_back(res);
    }

    return nodeMapPVs;
}

// clang-format off
using PTE = ndsda::PVTypeEnum;
static const std::map<std::string, PTE> PV_TYPE_STRMAP = {
    {"int32",          PTE::INT32},
    {"int64",          PTE::INT64},
    {"float",          PTE::FLOAT},
    {"double",         PTE::DOUBLE},
    {"vector(int8)",   PTE::VEC_INT8},
    {"vector(uint8)",  PTE::VEC_UINT8},
    {"vector(int16)",  PTE::VEC_INT16},
    {"vector(uint16)", PTE::VEC_UINT16},
    {"vector(int32)",  PTE::VEC_INT32},
    {"vector(uint32)", PTE::VEC_UINT32},
    {"vector(int64)",  PTE::VEC_INT64},
    {"vector(bool)",   PTE::VEC_BOOL},
    {"vector(float)",  PTE::VEC_FLOAT},
    {"vector(double)", PTE::VEC_DOUBLE},
    {"string",         PTE::STRING},
};
// clang-format on

static Result<std::shared_ptr<ndsda::NodeMapPV>, ndsda::ndsdaError> parsePV(const boost::property_tree::ptree& pvTree) {
    auto xmlattrs = pvTree.get_child_optional("<xmlattr>");
    if (!xmlattrs) return ndsda::ndsdaError("Invalid PV element: Missing XML attributes.");

    // Required attributes
    auto nameOpt = xmlattrs->get_optional<std::string>("name");
    if (!nameOpt) return ndsda::ndsdaError("Invalid PV element: Missing 'name' attribute.");
    auto registerOpt = xmlattrs->get_optional<std::string>("register");
    if (!registerOpt) return ndsda::ndsdaError("Invalid PV element: Missing 'register' attribute.");
    auto typeOpt = xmlattrs->get_optional<std::string>("type");
    if (!typeOpt) return ndsda::ndsdaError("Invalid PV element: Missing 'type' attribute.");

    // Optional attributes
    bool roundFloats    = xmlattrs->get_optional<std::string>("floatMethod").value_or("bitcast") == "approximate";
    int nElem           = xmlattrs->get_optional<int>("nElem").value_or(1);
    std::string readPV  = pvTree.get_optional<std::string>("ndsda:ReadPV").value_or(nameOpt.get() + ndsda::DAPV_GET_SUFFIX);
    std::string writePV = pvTree.get_optional<std::string>("ndsda:WritePV").value_or(nameOpt.get() + ndsda::DAPV_SET_SUFFIX);

    if (nElem < 1) {
        return ndsda::ndsdaError("Invalid PV element: 'nElem' must be greater or equal to 1. Current value: " + std::to_string(nElem));
    }

    ndsda::PVTypeEnum type;
    try {
        type = PV_TYPE_STRMAP.at(typeOpt.get());
    } catch (const std::out_of_range&) {
        return ndsda::ndsdaError("Invalid PV element: Unsupported 'type' attribute value: " + typeOpt.get());
    }

    return std::make_shared<ndsda::NodeMapPV>(nameOpt.get(), registerOpt.get(), type, roundFloats, readPV, writePV, static_cast<size_t>(nElem));
}