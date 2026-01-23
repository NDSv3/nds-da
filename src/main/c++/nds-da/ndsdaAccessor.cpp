#include "ndsdaAccessor.hpp"
#include <nds3/exceptions.h>

using nds::NdsError;
using ndsda::DAAccessor, ndsda::Result, ndsda::ndsdaError, ndsda::DARegisterType;

DAAccessor::DAAccessor(std::string dmapFilePath, std::string deviceName, const std::vector<std::shared_ptr<NodeMapPV>>& elements) {
    try {
        ChimeraTK::setDMapFilePath(dmapFilePath);
        daDevice.open(deviceName);
    } catch (const ChimeraTK::logic_error& e) {
        throw NdsError("Failed to initialize DeviceAccess: " + std::string(e.what()));
    }

    for (auto& ptr : elements) {
        if (!ptr) {
            throw NdsError("Null NodeMapPV provided to DAAccessor");
        }
        if (!ptr->memoryElement) {
            throw NdsError("NodeMapPV " + ptr->name + " has no associated MemoryElement");
        }
        readPVMap.emplace(ptr->readPV, ptr);
        writePVMap.emplace(ptr->writePV, ptr);
    }
}

DAAccessor::~DAAccessor() {
    daDevice.close();
}

template <DARegisterType RegisterType>
Result<RegisterType, ndsdaError> DAAccessor::readRegister(const RegisterElement& reg) {
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
std::optional<ndsdaError> DAAccessor::writeRegister(const RegisterElement& reg, RegisterType value) {
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
Result<std::vector<RegisterType>, ndsdaError> DAAccessor::readOneDRegister(const OneDElement& reg) {
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
std::optional<ndsdaError> DAAccessor::writeOneDRegister(const OneDElement& reg, const std::vector<RegisterType>& values) {
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

// Explicit template instantiations
template Result<int32_t, ndsdaError> DAAccessor::readRegister(const RegisterElement&);
template Result<uint32_t, ndsdaError> DAAccessor::readRegister(const RegisterElement& readPV);

template std::optional<ndsdaError> DAAccessor::writeRegister(const RegisterElement&, int32_t);
template std::optional<ndsdaError> DAAccessor::writeRegister(const RegisterElement& writePV, uint32_t value);

template Result<std::vector<uint32_t>, ndsdaError> DAAccessor::readOneDRegister(const OneDElement& readPV);
template Result<std::vector<int32_t>, ndsdaError> DAAccessor::readOneDRegister(const OneDElement&);

template std::optional<ndsdaError> DAAccessor::writeOneDRegister(const OneDElement&, const std::vector<int32_t>&);
template std::optional<ndsdaError> DAAccessor::writeOneDRegister(const OneDElement& writePV, const std::vector<uint32_t>& values);
