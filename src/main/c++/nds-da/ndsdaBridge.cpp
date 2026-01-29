#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ctime>
#include <memory>

#include "ndsdaCommon.hpp"
#include <boost/outcome/try.hpp>

using ndsda::DABridge;

DABridge::DABridge(std::unique_ptr<ndsda::DAAccessor> daAccessor) {
    if (!daAccessor) {
        throw ndsda::ndsdaError("daAccessor cannot be nullptr");
    }
    this->daAccessor = std::move(daAccessor);
}

template <ndsda::PVDataType PVType>
void DABridge::readPV(timespec* ts, PVType* value, const ndsda::NodeMapPV& pv) {
    using PTE = ndsda::PVTypeEnum;

    clock_gettime(CLOCK_REALTIME, ts);
    if constexpr (PVWordType<PVType>) {
        try {
            auto& reg = dynamic_cast<ndsda::RegisterElement&>(*pv.memoryElement);
            if (auto result = daAccessor->readRegister<uint32_t>(reg)) {
                *value = std::bit_cast<PVType>(result.assume_value());
            } else {
                throw ndsda::ndsdaError("Failed to read PV '" + pv.name + "': " + std::string(result.assume_error().what()));
            }
        } catch (const std::bad_cast&) {
            throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' with incompatible type");
        }
    } else {
        // TODO: Other types
    }
}

template <ndsda::PVDataType PVType>
void DABridge::writePV(timespec ts, PVType value, const ndsda::NodeMapPV& pv) {
    (void)ts;

    if constexpr (PVWordType<PVType>) {
        try {
            auto& reg = dynamic_cast<ndsda::RegisterElement&>(*pv.memoryElement);
            auto res  = daAccessor->writeRegister<uint32_t>(reg, std::bit_cast<uint32_t>(value));
            if (res) {
                throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + std::string(res->what()));
            }
        } catch (const std::bad_cast&) {
            throw ndsda::ndsdaError("Tried to write PV '" + pv.name + "' with incompatible type");
        }
    } else {
        // TODO: Other types
    }
}

// Explicit template instantiations

#define INSTANTIATE_TEMPLATE(TYPE)                                      \
    template void DABridge::readPV(timespec*, TYPE*, const NodeMapPV&); \
    template void DABridge::writePV(timespec, TYPE, const NodeMapPV&);

INSTANTIATE_TEMPLATE(std::int32_t)
INSTANTIATE_TEMPLATE(std::int64_t)
INSTANTIATE_TEMPLATE(float)
INSTANTIATE_TEMPLATE(double)
INSTANTIATE_TEMPLATE(std::string)
INSTANTIATE_TEMPLATE(std::vector<bool>)
INSTANTIATE_TEMPLATE(std::vector<std::uint8_t>)
INSTANTIATE_TEMPLATE(std::vector<std::uint16_t>)
INSTANTIATE_TEMPLATE(std::vector<std::uint32_t>)
INSTANTIATE_TEMPLATE(std::vector<std::int8_t>)
INSTANTIATE_TEMPLATE(std::vector<std::int16_t>)
INSTANTIATE_TEMPLATE(std::vector<std::int32_t>)
INSTANTIATE_TEMPLATE(std::vector<std::int64_t>)
INSTANTIATE_TEMPLATE(std::vector<float>)
INSTANTIATE_TEMPLATE(std::vector<double>)

#undef INSTANTIATE_TEMPLATE