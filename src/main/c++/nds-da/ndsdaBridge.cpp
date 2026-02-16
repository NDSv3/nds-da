#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <algorithm>
#include <ctime>
#include <memory>
#include <optional>

#include "ndsdaAccessor.hpp"
#include "ndsdaCommon.hpp"

using ndsda::DABridge;

DABridge::DABridge(std::unique_ptr<ndsda::DAAccessor> daAccessor) {
    if (!daAccessor) {
        throw ndsda::ndsdaError("daAccessor cannot be nullptr");
    }
    this->daAccessor = std::move(daAccessor);
}

static auto TRY_RESULT = [](auto&& expression, auto&& errorPrefix) {
    auto res = (expression);
    if (res.has_error()) {
        throw ndsda::ndsdaError(errorPrefix + std::string(res.assume_error().what()));
    }
    return std::move(res).assume_value();
};
static auto TRY_OPTION_ERROR = [](auto&& expression, auto&& errorPrefix) {
    auto res = (expression);
    if (res.has_value()) {
        throw ndsda::ndsdaError(errorPrefix + res.value().what());
    }
};

template <ndsda::PVDataType PVType>
static PVType dispatchedReadPV(timespec* ts, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor);
template <ndsda::PVDataType PVType>
static void dispatchedWritePV(timespec ts, const PVType& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor);

template <ndsda::PVDataType PVType>
void DABridge::readPV(timespec* ts, PVType* value, const ndsda::NodeMapPV& pv) {
    clock_gettime(CLOCK_REALTIME, ts);
    *value = dispatchedReadPV<PVType>(ts, pv, *daAccessor);
}

template <ndsda::PVDataType PVType>
void DABridge::writePV(timespec ts, const PVType& value, const ndsda::NodeMapPV& pv) {
    (void)ts;

    dispatchedWritePV<PVType>(ts, value, pv, *daAccessor);
}

template <ndsda::PVWordType PVType>  // Read Scalar registers
static PVType dispatchedReadPV(timespec* ts, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::SCALAR) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as scalar, but it is not");
    }
    auto& reg = static_cast<ndsda::RegisterElement&>(*pv.memoryElement);

    return TRY_RESULT(daAccessor.readRegister<PVType>(reg), "Failed to read PV '" + pv.name + "': ");
}

template <ndsda::PVWordVectorType PVType>  // Read vector/1D registers
static PVType dispatchedReadPV(timespec* ts, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }

    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    return TRY_RESULT(daAccessor.readOneDRegister<typename PVType::value_type>(reg), "Failed to read PV '" + pv.name + "': ");
}

template <>  // Special handling of vector<bool>
std::vector<bool> dispatchedReadPV(timespec* ts, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }

    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    auto booleanVec = TRY_RESULT(daAccessor.readOneDRegister<ChimeraTK::Boolean>(reg), "Failed to read PV '" + pv.name + "': ");

    auto vec = std::vector<bool>();
    vec.reserve(booleanVec.size());
    std::transform(booleanVec.begin(), booleanVec.end(), std::back_inserter(vec), [](ChimeraTK::Boolean b) {
        return static_cast<bool>(b);
    });
    return vec;
}

template <>  // Specialization for std::string
std::string dispatchedReadPV(timespec* ts, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }

    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    auto res     = TRY_RESULT(daAccessor.readOneDRegister<uint8_t>(reg), "Failed to read PV '" + pv.name + "': ");
    auto nullPos = std::find(res.begin(), res.end(), '\0');  // Terminate string on first NULL
    return std::string(res.begin(), nullPos);
}

template <ndsda::PVDataType PVType>  // Fallback to error
static PVType dispatchedReadPV(timespec* ts, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;
    (void)pv;
    (void)daAccessor;
    throw ndsda::ndsdaError("DeviceAccess does not support double word types yet");  // TODO:
}

template <ndsda::PVWordType PVType>
static void dispatchedWritePV(timespec ts, const PVType& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::SCALAR) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as scalar, but it is not");
    }
    auto& reg = static_cast<ndsda::RegisterElement&>(*pv.memoryElement);

    TRY_OPTION_ERROR(daAccessor.writeRegister<PVType>(reg, value), "Failed to write PV '" + pv.name + "': ");
}

template <ndsda::PVWordVectorType PVType>
static void dispatchedWritePV(timespec ts, const PVType& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }
    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    auto res = daAccessor.writeOneDRegister<typename PVType::value_type>(reg, value);
    if (res) {
        throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + std::string(res->what()));
    }
}

template <>
void dispatchedWritePV<std::vector<bool>>(timespec ts, const std::vector<bool>& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        throw ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }
    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    std::vector<ChimeraTK::Boolean> convertedValue;
    convertedValue.reserve(value.size());
    std::transform(value.begin(), value.end(), std::back_inserter(convertedValue), [](bool b) {
        return static_cast<ChimeraTK::Boolean>(b);
    });

    auto res = daAccessor.writeOneDRegister<ChimeraTK::Boolean>(reg, convertedValue);
    if (res) {
        throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + std::string(res->what()));
    }
}

template <>  // Specialization for std::string
void dispatchedWritePV<std::string>(timespec ts, const std::string& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)ts;

    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        throw ndsda::ndsdaError("Tried to write PV '" + pv.name + "' as 1D, but it is not");
    }
    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    std::vector<uint8_t> convertedValue(value.begin(), value.end());

    auto res = daAccessor.writeOneDRegister<uint8_t>(reg, convertedValue);
    if (res) {
        throw ndsda::ndsdaError("Failed to write PV '" + pv.name + "': " + std::string(res->what()));
    }
}

template <ndsda::PVDataType PVType>
// requires(!ndsda::PVWordType<PVType> && !ndsda::PVVectorType<PVType>)  // Idk why required here and not on read
static void dispatchedWritePV(timespec, const PVType&, const ndsda::NodeMapPV&, ndsda::DAAccessor&) {
    throw ndsda::ndsdaError("DAAccessor does not support double word types yet");  // TODO:
}
// Explicit template instantiations

#define INSTANTIATE_TEMPLATE(TYPE)                                      \
    template void DABridge::readPV(timespec*, TYPE*, const NodeMapPV&); \
    template void DABridge::writePV(timespec, const TYPE&, const NodeMapPV&);

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