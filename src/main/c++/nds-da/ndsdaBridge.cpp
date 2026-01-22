#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <variant>

#include "nds3/exceptions.h"
#include "ndsdaCommon.hpp"

using ndsda::DABridge, ndsda::DARegisterType, ndsda::Result, ndsda::ndsdaError;
using nds::NdsError;

DABridge::DABridge(std::string dmapFilePath, std::string deviceName, const std::vector<std::shared_ptr<NodeMapPV>>& elements) {

    try {
        ChimeraTK::setDMapFilePath(dmapFilePath);
        daDevice.open(deviceName);
    } catch (const ChimeraTK::logic_error& e) {
        throw NdsError("Failed to initialize DeviceAccess: " + std::string(e.what()));
    }

    for (auto& ptr : elements) {
        if (!ptr) {
            throw NdsError("Null NodeMapPV provided to DABridge");
        }
        if (!ptr->memoryElement) {
            throw NdsError("NodeMapPV " + ptr->name + " has no associated MemoryElement");
        }
        readPVMap.emplace(ptr->readPV, ptr);
        writePVMap.emplace(ptr->writePV, ptr);
    }
}

DABridge::~DABridge() {
    daDevice.close();
}

template <DARegisterType RegisterType>
Result<RegisterType, ndsdaError> DABridge::readRegister(const RegisterElement& reg) {
    if (!reg.daAccessor) {
        reg.daAccessor.emplace(daDevice.getScalarRegisterAccessor<RegisterType>(reg.name));
    }

    try {
        auto& acc = std::get<ChimeraTK::ScalarRegisterAccessor<RegisterType>>(reg.daAccessor.value());
        acc.read();
        return static_cast<RegisterType>(acc);
    } catch (const std::bad_variant_access&) {
        return ndsdaError("Tried to read register " + reg.name + " with incompatible type");
    }
}

template <DARegisterType RegisterType>
std::optional<ndsdaError> DABridge::writeRegister(const RegisterElement& reg, RegisterType value) {
    if (!reg.daAccessor) {
        reg.daAccessor.emplace(daDevice.getScalarRegisterAccessor<RegisterType>(reg.name));
    }

    try {
        auto& acc = std::get<ChimeraTK::ScalarRegisterAccessor<RegisterType>>(reg.daAccessor.value());
        acc       = value;
        acc.write();
    } catch (const std::bad_variant_access&) {
        return ndsdaError("Tried to write register " + reg.name + " with incompatible type");
    }

    return std::nullopt;  // OK
}

template <DARegisterType RegisterType>
Result<std::vector<RegisterType>, ndsdaError> DABridge::readOneDRegister(const OneDElement& reg) {
    if (!reg.daAccessor) {
        reg.daAccessor.emplace(daDevice.getOneDRegisterAccessor<RegisterType>(reg.name));
    }

    try {
        auto& acc = std::get<ChimeraTK::OneDRegisterAccessor<RegisterType>>(reg.daAccessor.value());
        acc.read();
        return std::vector<RegisterType>(acc.begin(), acc.end());
    } catch (const std::bad_variant_access&) {
        return ndsdaError("Tried to read OneDRegister " + reg.name + " with incompatible type");
    }
}

template <DARegisterType RegisterType>
std::optional<ndsdaError> DABridge::writeOneDRegister(const OneDElement& reg, const std::vector<RegisterType>& values) {
    if (values.size() > reg.elements) {
        return ndsdaError("Register " + reg.name + " expects " + std::to_string(reg.elements) + " elements, but " + std::to_string(values.size()) + " were provided");
    }

    if (!reg.daAccessor) {
        reg.daAccessor.emplace(daDevice.getOneDRegisterAccessor<RegisterType>(reg.name));
    }

    try {
        auto& accessor = std::get<ChimeraTK::OneDRegisterAccessor<RegisterType>>(reg.daAccessor.value());
        std::copy(values.begin(), values.end(), accessor.begin());
        accessor.write();
    } catch (const std::bad_variant_access&) {
        return ndsdaError("Tried to write OneD " + reg.name + " with incompatible type");
    }

    return std::nullopt;  // OK
}

template <ndsda::PVDataType PVType>
void DABridge::readPV(timespec* ts, PVType* value, const ndsda::NodeMapPV& pv) {
    using PTE = ndsda::PVTypeEnum;

    clock_gettime(CLOCK_REALTIME, ts);
    if constexpr (PVScalarType<PVType>) {
        *value = 1234;
    } else {
        *value = {0, 1, 0, 1};
    }
}

template <ndsda::PVDataType PVType>
void DABridge::writePV(timespec ts, PVType value, const ndsda::NodeMapPV& pv) {
    (void)ts;
    (void)value;
    (void)pv;
}

// Explicit template instantiations
template Result<int32_t, ndsdaError> DABridge::readRegister(const RegisterElement&);
template Result<uint32_t, ndsdaError> DABridge::readRegister(const RegisterElement& readPV);

template std::optional<ndsdaError> DABridge::writeRegister(const RegisterElement&, int32_t);
template std::optional<ndsdaError> DABridge::writeRegister(const RegisterElement& writePV, uint32_t value);

template Result<std::vector<uint32_t>, ndsdaError> DABridge::readOneDRegister(const OneDElement& readPV);
template Result<std::vector<int32_t>, ndsdaError> DABridge::readOneDRegister(const OneDElement&);

template std::optional<ndsdaError> DABridge::writeOneDRegister(const OneDElement&, const std::vector<int32_t>&);
template std::optional<ndsdaError> DABridge::writeOneDRegister(const OneDElement& writePV, const std::vector<uint32_t>& values);

template void DABridge::readPV(timespec*, std::int32_t*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::int64_t*, const NodeMapPV&);
template void DABridge::readPV(timespec*, float*, const NodeMapPV&);
template void DABridge::readPV(timespec*, double*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::string*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<bool>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::uint8_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::uint16_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::uint32_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int8_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int16_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int32_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int64_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<float>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<double>*, const NodeMapPV&);

template void DABridge::writePV(timespec, std::int32_t, const NodeMapPV&);
template void DABridge::writePV(timespec, std::int64_t, const NodeMapPV&);
template void DABridge::writePV(timespec, float, const NodeMapPV&);
template void DABridge::writePV(timespec, double, const NodeMapPV&);
template void DABridge::writePV(timespec, std::string, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<bool>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::uint8_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::uint16_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::uint32_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int8_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int16_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int32_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int64_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<float>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<double>, const NodeMapPV&);