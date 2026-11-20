/**
 * @file ndsdaBridge.cpp
 * @brief NDS-DA Middle layer for data conversion implementation
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <algorithm>
#include <ctime>
#include <memory>
#include <optional>

#include "ndsdaAccessor.hpp"
#include "ndsdaCommon.hpp"

using ndsda::DABridge, ndsda::Result, ndsda::ndsdaError;

DABridge::DABridge(std::unique_ptr<ndsda::DAAccessor> daAccessor) {
    if (!daAccessor) {
        throw ndsda::ndsdaError("daAccessor cannot be nullptr");
    }
    this->daAccessor = std::move(daAccessor);
}

// static auto TRY_RESULT = [](auto&& expression, auto&& errorPrefix) {
//     auto res = (expression);
//     if (res.has_error()) {
//         throw ndsda::ndsdaError(errorPrefix + std::string(res.assume_error().what()));
//     }
//     return std::move(res).assume_value();
// };
// static auto TRY_OPTION_ERROR = [](auto&& expression, auto&& errorPrefix) {
//     auto res = (expression);
//     if (res.has_value()) {
//         throw ndsda::ndsdaError(errorPrefix + res.value().what());
//     }
// };

template <ndsda::PVDataType PVType>
Result<PVType> dispatchedReadPV(const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor);
template <ndsda::PVDataType PVType>
std::optional<ndsdaError> dispatchedWritePV(const PVType& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor);

template <ndsda::PVWordType PVType>  // Read Scalar registers
Result<PVType> dispatchedReadPV(const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::SCALAR) {
        return ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as scalar, but it is not");
    }
    auto& reg = static_cast<ndsda::RegisterElement&>(*pv.memoryElement);

    return daAccessor.readRegister<PVType>(reg);
}

template <ndsda::PVWordVectorType PVType>  // Read vector/1D registers
Result<PVType> dispatchedReadPV(const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        return ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }

    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    return daAccessor.readOneDRegister<typename PVType::value_type>(reg);
}

template <>  // Special handling of vector<bool>
Result<std::vector<bool>> dispatchedReadPV(const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        return ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }

    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    auto readResult = daAccessor.readOneDRegister<ChimeraTK::Boolean>(reg);
    if (!readResult) {
        return readResult.assume_error();
    }

    auto& booleanVec = readResult.value();
    auto vec         = std::vector<bool>();
    vec.reserve(booleanVec.size());
    std::transform(booleanVec.begin(), booleanVec.end(), std::back_inserter(vec), [](ChimeraTK::Boolean b) {
        return static_cast<bool>(b);
    });
    return vec;
}

template <>  // Specialization for std::string
Result<std::string> dispatchedReadPV(const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        return ndsda::ndsdaError("Tried to read PV '" + pv.name + "' as 1D, but it is not");
    }

    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    auto res = daAccessor.readOneDRegister<uint8_t>(reg);
    if (!res) {
        return res.assume_error();
    }
    auto& strRes = res.value();
    auto nullPos = std::find(strRes.begin(), strRes.end(), '\0');  // Terminate string on first NULL
    return std::string(strRes.begin(), nullPos);
}

template <ndsda::PVDataType PVType>  // Fallback to error
Result<PVType> dispatchedReadPV(const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    (void)pv;
    (void)daAccessor;
    return ndsda::ndsdaError("DeviceAccess does not support double word types yet");  // TODO:
}

template <ndsda::PVWordType PVType>
std::optional<ndsdaError> dispatchedWritePV(const PVType& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::SCALAR) {
        return ndsda::ndsdaError("Tried to write PV '" + pv.name + "' as scalar, but it is not");
    }
    auto& reg = static_cast<ndsda::RegisterElement&>(*pv.memoryElement);

    return daAccessor.writeRegister<PVType>(reg, value);
}

template <ndsda::PVWordVectorType PVType>
std::optional<ndsdaError> dispatchedWritePV(const PVType& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        return ndsda::ndsdaError("Tried to write PV '" + pv.name + "' as 1D, but it is not");
    }
    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    return daAccessor.writeOneDRegister<typename PVType::value_type>(reg, value);
}

template <>
std::optional<ndsdaError> dispatchedWritePV<std::vector<bool>>(const std::vector<bool>& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        return ndsda::ndsdaError("Tried to write PV '" + pv.name + "' as 1D, but it is not");
    }
    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    std::vector<ChimeraTK::Boolean> convertedValue;
    convertedValue.reserve(value.size());
    std::transform(value.begin(), value.end(), std::back_inserter(convertedValue), [](bool b) {
        return static_cast<ChimeraTK::Boolean>(b);
    });

    return daAccessor.writeOneDRegister<ChimeraTK::Boolean>(reg, convertedValue);
}

template <>  // Specialization for std::string
std::optional<ndsdaError> dispatchedWritePV<std::string>(const std::string& value, const ndsda::NodeMapPV& pv, ndsda::DAAccessor& daAccessor) {
    if (pv.memoryElement->getType() != ndsda::MemoryElementType::ONE_D) {
        return ndsda::ndsdaError("Tried to write PV '" + pv.name + "' as 1D, but it is not");
    }
    auto& reg = static_cast<ndsda::OneDElement&>(*pv.memoryElement);

    std::vector<uint8_t> convertedValue(value.begin(), value.end());

    return daAccessor.writeOneDRegister<uint8_t>(reg, convertedValue);
}

template <ndsda::PVDataType PVType>
std::optional<ndsdaError> dispatchedWritePV(const PVType&, const ndsda::NodeMapPV&, ndsda::DAAccessor&) {
    return ndsda::ndsdaError("DAAccessor does not support double word types yet");  // TODO:
}

template <ndsda::PVDataType PVType>
Result<std::pair<PVType, timespec>> DABridge::readPV(const ndsda::NodeMapPV& pv) {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    auto val = dispatchedReadPV<PVType>(pv, *daAccessor);

    if (val) {
        return std::make_pair(val.value(), ts);
    } else {
        return val.as_failure();
    }
}

template <ndsda::PVDataType PVType>
std::optional<ndsdaError> DABridge::writePV(timespec ts, const PVType& value, const ndsda::NodeMapPV& pv) {
    (void)ts;
    return dispatchedWritePV<PVType>(value, pv, *daAccessor);
}

// Explicit template instantiations

#define INSTANTIATE_TEMPLATE(TYPE)                                                                            \
    template ndsda::Result<std::pair<TYPE, timespec>> ndsda::DABridge::readPV<TYPE>(const ndsda::NodeMapPV&); \
    template std::optional<ndsda::ndsdaError> ndsda::DABridge::writePV<TYPE>(timespec, const TYPE&, const ndsda::NodeMapPV&);

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
