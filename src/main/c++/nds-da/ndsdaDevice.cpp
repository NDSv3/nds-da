#include "ndsdaDevice.hpp"
#include <ctime>
#include <optional>
#include <utility>
#include <boost/outcome/try.hpp>
#include <variant>

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

    daBridge = std::make_unique<DABridge>(deviceMapFile, deviceAlias, nodeMapPVsResult);

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

// template <typename T>
// void Device::addScalarPV(const ndsda::NodeMapPV& pv) {
//     static_assert(PVScalarType<T>, "This type is not supported for scalar PVs");

//     const auto readLambda = [this, pv](timespec* ts, T* value) {
//         clock_gettime(CLOCK_REALTIME, ts);
//         *value = this->daBridge->readRegister<T>(pv.readPV);
//     };
//     const auto writeLambda = [this, pv](const timespec& ts, const T& value) {
//         (void)ts;  // TODO: Research why ts is unused
//         this->daBridge->writeRegister<T>(pv.writePV, value);
//     };

//     rootNode.addChild(nds::PVDelegateIn<T>(pv.readPV, readLambda));
//     rootNode.addChild(nds::PVDelegateOut<T>(pv.writePV, writeLambda));
// }

// template <>
// void Device::addScalarPV<uint32_t>(const ndsda::NodeMapPV& pv) {
//     const auto readLambda = [this, pv](timespec* ts, int32_t* value) {
//         clock_gettime(CLOCK_REALTIME, ts);
//         *value = std::bit_cast<int32_t>(this->daBridge->readRegister<uint32_t>(pv.readPV));
//     };
//     const auto writeLambda = [this, pv](const timespec& ts, const int32_t& value) {
//         (void)ts;  // TODO: Research why ts is unused
//         this->daBridge->writeRegister<uint32_t>(pv.writePV, std::bit_cast<int32_t>(value));
//     };

//     rootNode.addChild(nds::PVDelegateIn<int32_t>(pv.readPV, readLambda));
//     rootNode.addChild(nds::PVDelegateOut<int32_t>(pv.writePV, writeLambda));
// }

// template <typename T>
// void Device::addOneDPV(const ndsda::NodeMapPV& pv) {
//     const auto readLambda = [this, pv](timespec* ts, std::vector<T>* value) {
//         clock_gettime(CLOCK_REALTIME, ts);
//         *value = this->daBridge->readOneDRegister<T>(pv.readPV);
//     };
//     const auto writeLambda = [this, pv](const timespec& ts, const std::vector<T>& value) {
//         (void)ts;
//         this->daBridge->writeOneDRegister<T>(pv.writePV, value);
//     };

//     rootNode.addChild(nds::PVDelegateIn<std::vector<T>>(pv.readPV, readLambda));
//     rootNode.addChild(nds::PVDelegateOut<std::vector<T>>(pv.writePV, writeLambda));
// }