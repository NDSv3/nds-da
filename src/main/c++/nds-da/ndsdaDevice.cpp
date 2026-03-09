/**
 * @file ndsdaDevice.cpp
 * @brief NDS-DA Device base class implementation
 *
 * @author David Andrino - I2A2 UPM
 * @copyright GNU General Public License v3.0
 * @date 2026-02
 */
#include "ndsdaDevice.hpp"
#include <algorithm>
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

using ndsda::ndsdaDevice;
using ndsda::Result;

ndsdaDevice::ndsdaDevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
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

    if (auto r = FileParser::parseNodeMapFile(nodeMapFile)) {
        this->nodeMapPVs = std::move(r.assume_value());
    } else {
        throw nds::NdsError(r.assume_error().what());
    }

    auto mem = FileParser::parseMemoryMapFile(this->memoryMapFile);
    if (!mem) {
        throw nds::NdsError(mem.assume_error().what());
    }

    auto r = mergeMapElements(mem.assume_value(), this->nodeMapPVs);  // mem is moved from
    if (r) {
        throw nds::NdsError(r->what());
    }

    try {
        daBridge = std::make_unique<DABridge>(std::make_unique<ndsda::DAAccessor>(deviceMapFile, this->deviceAlias));
    } catch (const ndsda::ndsdaError& e) {
        throw nds::NdsError("Failed to create DAAccessor: " + std::string(e.what()));
    }

    // rootNode.initialize(this, factory);
}
ndsdaDevice::~ndsdaDevice() {}

std::optional<ndsda::ndsdaError> ndsdaDevice::mergeMapElements(
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

std::optional<ndsda::ndsdaError> ndsdaDevice::generatePVs(const ndsda::NodeMapPV& nodeMapPV) {
    auto readLambda = [&b = this->daBridge, &pv = nodeMapPV]<PVDataType T>(timespec* ts, T* value) {
        auto readPair = b->readPV<T>(pv);
        if (readPair.has_value()) {
            *value = readPair.value().first;
            *ts    = readPair.value().second;
        } else {
            throw ndsda::ndsdaError("Failed to read PV '" + pv.name + "': " + readPair.assume_error().what());
        }
    };
    auto writeLambda = [&b = this->daBridge, &pv = nodeMapPV]<PVDataType T>(timespec ts, T value) {
        auto writeRes = b->writePV(ts, value, pv);
        if (writeRes.has_value()) {
            throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + writeRes.value().what());
        }
    };

    // Templated lambda, called by visitor pattern with the PV type
    auto generatePVLambda = [&rootNode = this->rootNode, &nodeMapPV, readLambda, writeLambda]<PVDataType T>(std::type_identity<T>) {
        rootNode.addChild(nds::PVDelegateIn<T>(nodeMapPV.readPV, readLambda));
        rootNode.addChild(nds::PVDelegateOut<T>(nodeMapPV.writePV, writeLambda));
    };
    visitPVType(nodeMapPV.type, generatePVLambda);

    return std::nullopt;  // OK
}

template <ndsda::PVDataType T>
Result<std::pair<T, timespec>> ndsdaDevice::readPV(const std::string& pv) {
    auto nodePV = std::find_if(this->nodeMapPVs.begin(), this->nodeMapPVs.end(), [pv](const std::shared_ptr<ndsda::NodeMapPV>& nodePV) {
        return nodePV->readPV == pv || nodePV->name == pv;
    });
    if (nodePV == this->nodeMapPVs.end()) {
        return ndsda::ndsdaError("PV '" + pv + "' not found");
    } else if ((**nodePV).type != TtoPVTypeEnum<T>) {
        return ndsda::ndsdaError("PV '" + pv + "' has incompatible type");
    } else {
        return daBridge->readPV<T>(**nodePV);
    }
}

template <ndsda::PVDataType T>
std::optional<ndsda::ndsdaError> ndsdaDevice::writePV(const std::string& pv, T&& value, timespec ts) {
    auto nodePV = std::find_if(this->nodeMapPVs.begin(), this->nodeMapPVs.end(), [pv](const std::shared_ptr<ndsda::NodeMapPV>& nodePV) {
        return nodePV->writePV == pv || nodePV->name == pv;
    });
    if (nodePV == this->nodeMapPVs.end()) {
        return ndsda::ndsdaError("PV '" + pv + "' not found");
    } else if ((**nodePV).type != TtoPVTypeEnum<T>) {
        return ndsda::ndsdaError("PV '" + pv + "' has incompatible type");
    } else {
        return daBridge->writePV(ts, value, **nodePV);
    }
}

#define INSTANTIATE_READWRITEPV_TEMPLATE(TYPE)                                                              \
    template ndsda::Result<std::pair<TYPE, timespec>> ndsda::ndsdaDevice::readPV<TYPE>(const std::string&); \
    template std::optional<ndsda::ndsdaError> ndsda::ndsdaDevice::writePV<TYPE>(const std::string&, TYPE&&, timespec);

INSTANTIATE_READWRITEPV_TEMPLATE(std::int32_t)
INSTANTIATE_READWRITEPV_TEMPLATE(std::int64_t)
INSTANTIATE_READWRITEPV_TEMPLATE(float)
INSTANTIATE_READWRITEPV_TEMPLATE(double)
INSTANTIATE_READWRITEPV_TEMPLATE(std::string)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<bool>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::uint8_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::uint16_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::uint32_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::int8_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::int16_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::int32_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<std::int64_t>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<float>)
INSTANTIATE_READWRITEPV_TEMPLATE(std::vector<double>)

#undef INSTANTIATE_READWRITEPV_TEMPLATE