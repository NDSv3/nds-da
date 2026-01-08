#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <memory>
#include <stdexcept>
#include <variant>

#include "nds3/exceptions.h"

using ndsda::DABridge, ndsda::Integer32;
using nds::NdsError;

DABridge::DABridge(std::string dmapFilePath, std::string deviceName, std::vector<std::shared_ptr<MemoryElement>> elements) {

    try {
        ChimeraTK::setDMapFilePath(dmapFilePath);
        daDevice.open(deviceName);
    } catch (const ChimeraTK::logic_error& e) {
        throw NdsError("Failed to initialize DeviceAccess: " + std::string(e.what()));
    }

    for (std::shared_ptr<MemoryElement>& ptr : elements) {
        if (!ptr) {
            throw NdsError("Null MemoryElement provided to DABridge");
        }
        readPVMap.emplace(ptr->readPV, ptr);
        writePVMap.emplace(ptr->writePV, ptr);
    }
}

DABridge::~DABridge() {
    daDevice.close();
}

template <Integer32 T>
T DABridge::readRegister(const std::string& readPV) {
    std::shared_ptr<RegisterElement> acc;

    try {
        acc = std::dynamic_pointer_cast<RegisterElement>(readPVMap.at(readPV));
    } catch (std::out_of_range&) {
        throw NdsError("Register with read PV " + readPV + " not found");
    }

    if (!acc) {
        throw NdsError("Register with read PV " + readPV + " is not a scalar register");
    }

    if (!acc->daAccessor) {
        if constexpr (std::is_same_v<T, uint32_t>) {
            if (acc->isSigned) {
                throw nds::NdsError("Tried to read signed scalar " + acc->name + " as unsigned");
            }
        } else {
            if (!acc->isSigned) {
                throw nds::NdsError("Tried to read unsigned scalar " + acc->name + " as signed");
            }
        }

        acc->daAccessor.emplace(daDevice.getScalarRegisterAccessor<T>(acc->name));
    }

    try {
        auto& accessor = std::get<ChimeraTK::ScalarRegisterAccessor<T>>(acc->daAccessor.value());
        accessor.read();
        return static_cast<T>(accessor);
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->readPV + " has incompatible type");
    }
}

template <Integer32 T>
void DABridge::writeRegister(const std::string& writePV, T value) {
    std::shared_ptr<RegisterElement> acc;

    try {
        acc = std::dynamic_pointer_cast<RegisterElement>(writePVMap.at(writePV));
    } catch (std::out_of_range&) {
        throw NdsError("Register with write PV " + writePV + " not found");
    }

    if (!acc) {
        throw NdsError("Register with write PV " + writePV + " is not a scalar register");
    }

    if (!acc->daAccessor) {
        if constexpr (std::is_same_v<T, uint32_t>) {
            if (acc->isSigned) {
                throw nds::NdsError("Tried to write signed scalar " + acc->name + " as unsigned");
            }
        } else {
            if (!acc->isSigned) {
                throw nds::NdsError("Tried to write unsigned scalar " + acc->name + " as signed");
            }
        }

        acc->daAccessor.emplace(daDevice.getScalarRegisterAccessor<T>(acc->name));
    }

    try {
        auto& accessor = std::get<ChimeraTK::ScalarRegisterAccessor<T>>(acc->daAccessor.value());
        accessor       = value;
        accessor.write();
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->writePV + " has incompatible type");
    }
}

template <Integer32 T>
std::vector<T> DABridge::readOneDRegister(const std::string& readPV) {
    std::shared_ptr<OneDElement> acc;

    try {
        acc = std::dynamic_pointer_cast<OneDElement>(readPVMap.at(readPV));
    } catch (std::out_of_range&) {
        throw NdsError("Register with read PV " + readPV + " not found");
    }

    if (!acc) {
        throw NdsError("Register with read PV " + readPV + " is not a OneD register");
    }

    if (!acc->daAccessor) {
        if constexpr (std::is_same_v<T, uint32_t>) {
            if (acc->isSigned) {
                throw nds::NdsError("Tried to read signed register " + acc->name + " as unsigned");
            }
        } else {
            if (!acc->isSigned) {
                throw nds::NdsError("Tried to read unsigned register " + acc->name + " as signed");
            }
        }

        acc->daAccessor.emplace(daDevice.getOneDRegisterAccessor<T>(acc->name));
    }

    try {
        auto& accessor = std::get<ChimeraTK::OneDRegisterAccessor<T>>(acc->daAccessor.value());
        accessor.read();
        return std::vector<T>(accessor.begin(), accessor.end());
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->readPV + " has incompatible type");
    }
}

template <Integer32 T>
void DABridge::writeOneDRegister(const std::string& writePV, const std::vector<T>& values) {
    std::shared_ptr<OneDElement> acc;

    try {
        acc = std::dynamic_pointer_cast<OneDElement>(writePVMap.at(writePV));
    } catch (std::out_of_range&) {
        throw NdsError("Register with write PV " + writePV + " not found");
    }

    if (!acc) {
        throw NdsError("Register with write PV " + writePV + " is not a OneD register");
    }

    if (values.size() != acc->elements) {
        throw nds::NdsError("Register " + acc->name + " expects " + std::to_string(acc->elements) + " elements, but " + std::to_string(values.size()) + " were provided");
    }

    if (!acc->daAccessor) {
        if constexpr (std::is_same_v<T, uint32_t>) {
            if (acc->isSigned) {
                throw nds::NdsError("Tried to write signed register " + acc->name + " as unsigned");
            }
        } else {
            if (!acc->isSigned) {
                throw nds::NdsError("Tried to write unsigned register " + acc->name + " as signed");
            }
        }
        acc->daAccessor.emplace(daDevice.getOneDRegisterAccessor<T>(acc->name));
    }

    try {
        auto& accessor = std::get<ChimeraTK::OneDRegisterAccessor<T>>(acc->daAccessor.value());
        std::copy(values.begin(), values.end(), accessor.begin());
        accessor.write();
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->writePV + " has incompatible type");
    }
}

// Explicit template instantiations
template int32_t DABridge::readRegister(const std::string& readPV);
template void DABridge::writeRegister(const std::string& writePV, int32_t value);
template std::vector<int32_t> DABridge::readOneDRegister(const std::string& readPV);
template void DABridge::writeOneDRegister(const std::string& writePV, const std::vector<int32_t>& values);

template uint32_t DABridge::readRegister(const std::string& readPV);
template void DABridge::writeRegister(const std::string& writePV, uint32_t value);
template std::vector<uint32_t> DABridge::readOneDRegister(const std::string& readPV);
template void DABridge::writeOneDRegister(const std::string& writePV, const std::vector<uint32_t>& values);