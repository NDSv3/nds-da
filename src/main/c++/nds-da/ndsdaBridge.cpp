#include "ndsdaBridge.hpp"
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <memory>
#include <stdexcept>
#include <variant>

#include "nds3/exceptions.h"

using ndsda::DABridge, ndsda::DARegisterType;
using nds::NdsError;

DABridge::DABridge(std::string dmapFilePath, std::string deviceName, std::vector<std::shared_ptr<MemoryElement>> elements) {
    ChimeraTK::setDMapFilePath(dmapFilePath);
    daDevice.open(deviceName);

    for (std::shared_ptr<MemoryElement>& ptr : elements) {
        readPVMap.emplace(ptr->readPV, ptr);
        writePVMap.emplace(ptr->writePV, ptr);
    }
}

DABridge::~DABridge() {
    daDevice.close();
}

template <DARegisterType T>
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
        acc->daAccessor = daDevice.getScalarRegisterAccessor<T>(acc->name);
    }

    try {
        return std::get<ChimeraTK::ScalarRegisterAccessor<T>>(acc->daAccessor).read();
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->readPV + " has incompatible type");
    }
}

template <DARegisterType T>
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
        acc->daAccessor = daDevice.getScalarRegisterAccessor<T>(acc->name);
    }

    try {
        auto& accessor = std::get<ChimeraTK::ScalarRegisterAccessor<T>>(acc->daAccessor);
        accessor       = value;
        accessor.write();
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->writePV + " has incompatible type");
    }
}

template <DARegisterType T>
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
        acc->daAccessor = daDevice.getOneDRegisterAccessor<T>(acc->name);
    }

    try {
        auto& accessor = std::get<ChimeraTK::OneDRegisterAccessor<T>>(acc->daAccessor);
        accessor.read();
        return std::vector<T>(accessor.begin(), accessor.end());
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->readPV + " has incompatible type");
    }
}

template <DARegisterType T>
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
        acc->daAccessor = daDevice.getOneDRegisterAccessor<T>(acc->name);
    }

    try {
        auto& accessor = std::get<ChimeraTK::OneDRegisterAccessor<T>>(acc->daAccessor);
        std::copy(values.begin(), values.end(), accessor.begin());
        accessor.write();
    } catch (const std::bad_variant_access&) {
        throw nds::NdsError("Register " + acc->name + " of PV " + acc->writePV + " has incompatible type");
    }
}