#include "ndsdaDevice.hpp"
#include <cstdint>
#include <ctime>
#include <utility>
#include "nds3/definitions.h"
#include "nds3/exceptions.h"
#include "ndsdaBridge.hpp"
#include "ndsdaFileParser.hpp"

#define BIND_DELEGATEIN(func) (std::bind(&func, this, std::placeholders::_1, std::placeholders::_2))

using ndsda::Device;

template <>
void Device::addScalarPV<uint32_t>(const std::shared_ptr<ndsda::MemoryElement>& e);

Device::Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
    : deviceName(deviceName),
      rootNode(deviceName),
      deviceAlias("deviceAlias"),
      deviceMapFile("deviceMapFile"),
      memoryMapFilePV("memoryMapFile", [this](timespec* ts, std::string* value) {
          clock_gettime(CLOCK_REALTIME, ts);
          *value = this->memoryMapFile;
      }) {

    std::string deviceMapFileStr, deviceAliasStr;
    try {
        deviceMapFileStr = params.at("DeviceMapFile");
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DADeviceMapFile" << std::endl;
        throw nds::NdsError("Missing DeviceMapFile parameter");
    }

    try {
        deviceAliasStr = params.at("DeviceAlias");
    } catch (const std::out_of_range&) {
        // ndsErrorStream(rootNode) << "Missing DAMemoryMapFile" << std::endl;
        throw nds::NdsError("Missing MemoryMapFile parameter");
    }

    if (auto r = FileParser::getMemoryMapFile(deviceMapFileStr, deviceAliasStr)) {
        this->memoryMapFile = r.assume_value();
    } else {
        throw nds::NdsError(r.assume_error().what());
    }

    std::vector<std::shared_ptr<ndsda::MemoryElement>> elements;
    if (auto r = FileParser::parseMemoryMapFile(this->memoryMapFile)) {
        elements = std::move(r.assume_value());
    } else {
        throw nds::NdsError(r.assume_error().what());
    }

    daBridge = std::make_unique<DABridge>(deviceMapFileStr, deviceAliasStr, elements);

    using met = ndsda::MemoryElementType;
    for (auto& e : elements) {
        if (e->getType() == met::SCALAR) {
            if (e->isSigned) {
                addScalarPV<int32_t>(e);
            } else {
                addScalarPV<uint32_t>(e);
            }
        } else if (e->getType() == met::ONE_D) {
            if (e->isSigned) {
                addOneDPV<int32_t>(e);
            } else {
                addOneDPV<uint32_t>(e);
            }
        }
    }

    deviceMapFile.setValue(deviceMapFileStr);
    deviceAlias.setValue(deviceAliasStr);

    rootNode.addChild(deviceAlias);
    rootNode.addChild(deviceMapFile);
    rootNode.addChild(memoryMapFilePV);

    rootNode.initialize(this, factory);
}

Device::~Device() {}

template <typename T>
void Device::addScalarPV(const std::shared_ptr<ndsda::MemoryElement>& e) {
    static_assert(PVScalarType<T>, "This type is not supported for scalar PVs");

    const auto readLambda = [this, e](timespec* ts, T* value) {
        clock_gettime(CLOCK_REALTIME, ts);
        *value = this->daBridge->readRegister<T>(e->readPV);
    };
    const auto writeLambda = [this, e](const timespec& ts, const T& value) {
        (void)ts;  // TODO: Research why ts is unused
        this->daBridge->writeRegister<T>(e->writePV, value);
    };

    rootNode.addChild(nds::PVDelegateIn<T>(e->readPV, readLambda));
    rootNode.addChild(nds::PVDelegateOut<T>(e->writePV, writeLambda));
}

// TODO: What to do with uint32_t? NDS does not support unsigned PVs
template <>
void Device::addScalarPV<uint32_t>(const std::shared_ptr<ndsda::MemoryElement>& e) {
    const auto readLambda = [this, e](timespec* ts, int32_t* value) {
        clock_gettime(CLOCK_REALTIME, ts);
        *value = std::bit_cast<int32_t>(this->daBridge->readRegister<uint32_t>(e->readPV));
    };
    const auto writeLambda = [this, e](const timespec& ts, const int32_t& value) {
        (void)ts;  // TODO: Research why ts is unused
        this->daBridge->writeRegister<uint32_t>(e->writePV, std::bit_cast<int32_t>(value));
    };

    rootNode.addChild(nds::PVDelegateIn<int32_t>(e->readPV, readLambda));
    rootNode.addChild(nds::PVDelegateOut<int32_t>(e->writePV, writeLambda));
}

template <typename T>
void Device::addOneDPV(const std::shared_ptr<ndsda::MemoryElement>& e) {
    const auto readLambda = [this, e](timespec* ts, std::vector<T>* value) {
        clock_gettime(CLOCK_REALTIME, ts);
        *value = this->daBridge->readOneDRegister<T>(e->readPV);
    };
    const auto writeLambda = [this, e](const timespec& ts, const std::vector<T>& value) {
        (void)ts;
        this->daBridge->writeOneDRegister<T>(e->writePV, value);
    };

    rootNode.addChild(nds::PVDelegateIn<std::vector<T>>(e->readPV, readLambda));
    rootNode.addChild(nds::PVDelegateOut<std::vector<T>>(e->writePV, writeLambda));
}

NDS_DEFINE_DRIVER(DADevice, Device)
