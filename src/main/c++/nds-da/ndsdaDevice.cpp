#include "ndsdaDevice.hpp"
#include <ctime>
#include <optional>
#include <utility>
#include <boost/outcome/try.hpp>

#include "nds3/definitions.h"
#include "nds3/exceptions.h"
#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"
#include "ndsdaFileParser.hpp"

#define BIND_DELEGATEIN(func) (std::bind(&func, this, std::placeholders::_1, std::placeholders::_2))

#define GET_NDS_PARAMETER(var, paramName)                        \
    try {                                                        \
        var = params.at(paramName);                              \
    } catch (const std::out_of_range&) {                         \
        throw nds::NdsError("Missing " #paramName " parameter"); \
    }

using ndsda::Device;
using ndsda::Result;

Device::Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
    : deviceName(deviceName),
      rootNode(deviceName) {
    nds::PVVariableIn<std::string> deviceAliasPV("deviceAlias"),
        deviceMapFilePV("deviceMapFile"), memoryMapFilePV("memoryMapFile"), nodeMapFilePV("nodeMapFile");

    GET_NDS_PARAMETER(this->deviceMapFile, "DeviceMapFile");
    GET_NDS_PARAMETER(this->deviceAlias, "DeviceAlias");
    GET_NDS_PARAMETER(this->nodeMapFile, "NodeMapFile");

    if (auto r = FileParser::getMemoryMapFile(deviceMapFile, deviceAlias)) {
        this->memoryMapFile = r.assume_value();
    } else {
        throw nds::NdsError(r.assume_error().what());
    }

    std::map<nds::PVVariableIn<std::string>*, std::string&> infoPVs = {
        {&deviceAliasPV, deviceAlias},
        {&deviceMapFilePV, deviceMapFile},
        {&memoryMapFilePV, memoryMapFile},
        {&nodeMapFilePV, nodeMapFile}
    };
    for (auto& [pv, val] : infoPVs) {
        pv->setValue(val);
        rootNode.addChild(*pv);
    }

    std::vector<std::shared_ptr<ndsda::NodeMapPV>> nodeMapPVsResult;
    if (auto r = FileParser::parseNodeMapFile(nodeMapFile)) {
        nodeMapPVsResult = std::move(r.assume_value());
    } else {
        throw nds::NdsError(r.assume_error().what());
    }

    auto mem = FileParser::parseMemoryMapFile(this->memoryMapFile);
    if (!mem) {
        throw nds::NdsError(mem.assume_error().what());
    }

    auto r = mergeMapElements(mem.assume_value(), nodeMapPVsResult);  // mem is moved from
    if (r) {
        throw nds::NdsError(r->what());
    }

    try {
        daBridge = std::make_unique<DABridge>(std::make_unique<ndsda::DAAccessor>(deviceMapFile, this->deviceAlias));
    } catch (const ndsda::ndsdaError& e) {
        throw nds::NdsError("Failed to create DAAccessor: " + std::string(e.what()));
    }

    rootNode.initialize(this, factory);
}

Device::~Device() {}

std::optional<ndsda::ndsdaError> Device::mergeMapElements(
    std::vector<std::unique_ptr<ndsda::MemoryElement>>& memoryElements,
    std::vector<std::shared_ptr<ndsda::NodeMapPV>>& nodeMapPVs
) {
    for (auto& pv : nodeMapPVs) {
        // Search the PV register in the memory elements
        auto reg = std::find_if(
            memoryElements.begin(), memoryElements.end(),
            [&pv](const std::unique_ptr<ndsda::MemoryElement>& element) {
                // Once an element is found, it is changed for nullptr, so check that it is not null before
                return element && (pv->reg == element->name);
            }
        );
        if (reg == memoryElements.end())
            return ndsda::ndsdaError("Invalid register '" + pv->reg + "' for PV '" + pv->name + "'");

        // Store MemoryElement in PV. Leaves nullptr in vector
        pv->memoryElement = std::move(*reg);

        auto r = this->generatePVs(*pv);
        if (r) return r;
    }

    return std::nullopt;  // OK
}

std::optional<ndsda::ndsdaError> Device::generatePVs(const ndsda::NodeMapPV& nodeMapPV) {
    auto readLambda = [&b = this->daBridge, &pv = nodeMapPV]<PVDataType T>(timespec* ts, T* value) {
        b->readPV(ts, value, pv);
    };
    auto writeLambda = [&b = this->daBridge, &pv = nodeMapPV]<PVDataType T>(timespec ts, T value) {
        b->writePV(ts, value, pv);
    };

    // Templated lambda, called by visitor pattern with the PV type
    auto generatePVLambda = [&rootNode = this->rootNode, &nodeMapPV, readLambda, writeLambda]<PVDataType T>(std::type_identity<T>) {
        rootNode.addChild(nds::PVDelegateIn<T>(nodeMapPV.readPV, readLambda));
        rootNode.addChild(nds::PVDelegateOut<T>(nodeMapPV.writePV, writeLambda));
    };
    visitPVType(nodeMapPV.type, generatePVLambda);

    return std::nullopt;  // OK
}

NDS_DEFINE_DRIVER(DADevice, Device)