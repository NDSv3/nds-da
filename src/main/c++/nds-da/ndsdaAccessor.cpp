/**
 * @file ndsdaAccessor.cpp
 * @brief NDS-DA Low-level DA interface implementation
 *
 * @author David Andrino - I2A2 UPM
 * @copyright GNU General Public License v3.0
 * @date 2026-02
 */
#include "ndsdaAccessor.hpp"
#include <nds3/exceptions.h>

using nds::NdsError;
using ndsda::DAAccessor, ndsda::Result, ndsda::ndsdaError, ndsda::DARegisterType;

DAAccessor::DAAccessor(const std::string& dmapFilePath, const std::string& deviceName) {
    try {
        ChimeraTK::setDMapFilePath(dmapFilePath);
        daDevice.open(deviceName);
    } catch (const ChimeraTK::logic_error& e) {
        throw NdsError("Failed to initialize DeviceAccess: " + std::string(e.what()));
    }
}

DAAccessor::~DAAccessor() {
    daDevice.close();
}

template <DARegisterType RegisterType>
Result<RegisterType> DAAccessor::readRegister(const RegisterElement& reg) {
    if (!reg.daAccessor) {
        reg.daAccessor.emplace(daDevice.getScalarRegisterAccessor<RegisterType>(reg.name));
    }

    try {
        auto& acc = std::get<ChimeraTK::ScalarRegisterAccessor<RegisterType>>(reg.daAccessor.value());
        return acc.readAndGet();
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
        acc.setAndWrite(value);

    } catch (const std::bad_variant_access&) {
        return ndsdaError("Tried to write register " + reg.name + " with incompatible type");
    }

    return std::nullopt;  // OK
}

template <DARegisterType RegisterType>
Result<std::vector<RegisterType>> DAAccessor::readOneDRegister(const OneDElement& reg) {
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

#define INSTANTIATE_TEMPLATE(Type)                                                              \
    template Result<Type> DAAccessor::readRegister(const RegisterElement&);                     \
    template std::optional<ndsdaError> DAAccessor::writeRegister(const RegisterElement&, Type); \
    template Result<std::vector<Type>> DAAccessor::readOneDRegister(const OneDElement&);        \
    template std::optional<ndsdaError> DAAccessor::writeOneDRegister(const OneDElement&, const std::vector<Type>&);

INSTANTIATE_TEMPLATE(int8_t)
INSTANTIATE_TEMPLATE(uint8_t)
INSTANTIATE_TEMPLATE(int16_t)
INSTANTIATE_TEMPLATE(uint16_t)
INSTANTIATE_TEMPLATE(int32_t)
INSTANTIATE_TEMPLATE(uint32_t)
INSTANTIATE_TEMPLATE(int64_t)
INSTANTIATE_TEMPLATE(uint64_t)
INSTANTIATE_TEMPLATE(float)
INSTANTIATE_TEMPLATE(double)
INSTANTIATE_TEMPLATE(ChimeraTK::Boolean)

#undef INSTANTIATE_TEMPLATE