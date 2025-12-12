#include "ndsdaAccessor.hpp"
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <memory>
#include <stdexcept>
#include <variant>

#include "nds3/exceptions.h"

using ndsda::Accessor, ndsda::DARegisterType;
using nds::NdsError;

Accessor::Accessor(std::string dmapFilePath, std::string deviceName, std::vector<RegisterAccessor> registers) {
    ChimeraTK::setDMapFilePath(dmapFilePath);
    daDevice.open(deviceName);

    for (RegisterAccessor& ra : registers) {
        auto ptr = std::make_shared<RegisterAccessor>(ra);
        regReadMap.emplace(ra.readPV, ptr);
        regWriteMap.emplace(ra.writePV, ptr);
    }
}

Accessor::~Accessor() {
    daDevice.close();
}

template <DARegisterType T>
T Accessor::readRegister(const std::string& readPV) {
    std::shared_ptr<RegisterAccessor> acc;

    try {
        acc = regReadMap[readPV];
    } catch (std::out_of_range&) {
        throw NdsError("Register with read PV " + readPV + " not found");
    }

    if (!acc->daRegAccVariant) {
        acc->daRegAccVariant = daDevice.getScalarRegisterAccessor<T>(acc->registerName);
    }

    try {
        return std::get<ChimeraTK::OneDRegisterAccessor<T>>(acc->daRegAccVariant).read();
    } catch (std::bad_variant_access) {
        throw nds::NdsError("Register " + acc->registerName + " of PV " + acc->readPV + " has incompatible type");
    }
}

template <DARegisterType T>
void Accessor::writeRegister(const std::string& writePV, T value) {
    std::shared_ptr<RegisterAccessor> acc;

    try {
        acc = regReadMap[writePV];
    } catch (std::out_of_range&) {
        throw NdsError("Register with write PV " + writePV + " not found");
    }

    if (!acc->daRegAccVariant) {
        acc->daRegAccVariant = daDevice.getScalarRegisterAccessor<T>(acc->registerName);
    }

    try {
        auto& accessor = std::get<ChimeraTK::OneDRegisterAccessor<T>>(acc->daRegAccVariant);
        accessor       = value;
        accessor.write();
    } catch (std::bad_variant_access) {
        throw nds::NdsError("Register " + acc->registerName + " of PV " + acc->writePV + " has incompatible type");
    }
}