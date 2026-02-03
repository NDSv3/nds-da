#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ctime>
#include <memory>
#include <optional>

#include "ndsdaCommon.hpp"

using ndsda::DABridge;

DABridge::DABridge(std::unique_ptr<ndsda::DAAccessor> daAccessor) {
    if (!daAccessor) {
        throw ndsda::ndsdaError("daAccessor cannot be nullptr");
    }
    this->daAccessor = std::move(daAccessor);
}

#define TRY_RESULT(resultVar, expression, errorMsg)                                       \
    auto resultVar = (expression);                                                        \
    if (resultVar.has_error()) {                                                          \
        throw ndsda::ndsdaError(errorMsg + std::string(resultVar.assume_error().what())); \
    }

template <ndsda::PVDataType PVType>
void DABridge::readPV(timespec* ts, PVType* value, const ndsda::NodeMapPV& pv) {
    if constexpr (PVDoubleWordType<PVType> || PVDoubleWordVectorType<PVType>) {
        throw ndsda::ndsdaError("DAAccessor does not support double word types yet");  // TODO:
    }

    clock_gettime(CLOCK_REALTIME, ts);

    if constexpr (PVWordType<PVType>) {
        if (pv.memoryElement->getType() != ndsda::MemoryElementType::SCALAR) {
            throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as scalar, but it is not");
        }
        auto& reg = static_cast<ndsda::RegisterElement&>(*pv.memoryElement);

        if (std::same_as<int32_t, PVType> || pv.memoryElement->fracBits != 0) {  // Int or round float
            if (auto result = daAccessor->readRegister<PVType>(reg)) {
                *value = result.assume_value();
            } else {
                throw ndsda::ndsdaError("Failed to read PV '" + pv.name + "': " + std::string(result.assume_error().what()));
            }
        } else {  // Bitcast float
            auto result = daAccessor->readRegister<uint32_t>(reg);
            if (result) {
                *value = std::bit_cast<float>(result.assume_value());
            } else {
                throw ndsda::ndsdaError("Failed to read PV '" + pv.name + "': " + std::string(result.assume_error().what()));
            }
        }
    } else {
        // TODO: Vector types
    }
}

template <ndsda::PVDataType PVType>
void DABridge::writePV(timespec ts, PVType value, const ndsda::NodeMapPV& pv) {
    if constexpr (PVDoubleWordType<PVType> || PVDoubleWordVectorType<PVType>) {
        throw ndsda::ndsdaError("DAAccessor does not support double word types yet");  // TODO:
    }

    (void)ts;

    if constexpr (PVWordType<PVType>) {
        auto& reg = dynamic_cast<ndsda::RegisterElement&>(*pv.memoryElement);

        if (std::same_as<PVType, int32_t> || pv.memoryElement->fracBits != 0) {  // Int or round float
            auto res = daAccessor->writeRegister<PVType>(reg, value);
            if (res) {
                throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + std::string(res->what()));
            }
        } else {
            auto res = daAccessor->writeRegister<uint32_t>(reg, std::bit_cast<uint32_t>(value));
            if (res) {
                throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + std::string(res->what()));
            }
        }
    } else {
        // TODO: Vector types
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