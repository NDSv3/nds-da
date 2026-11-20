/**
 * @file ndsdaBridge.hpp
 * @brief NDS-DA Middle layer for data conversion
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef NDSDA_BRIDGE_H
#define NDSDA_BRIDGE_H

#include <ChimeraTK/Device.h>
#include <memory>
#include "ndsdaCommon.hpp"
#include "ndsdaAccessor.hpp"

namespace ndsda {

/**
 * @class DABridge
 * @brief Middle NDS-DA layer for data conversion between ChimeraTK and NDS PVs
 */
class DABridge {
public:
    /**
     * @brief Construct a bridge with a preconfigured DeviceAccess accessor
     * @param daAccessor Low-level accessor (must be non-null)
     */
    explicit DABridge(std::unique_ptr<ndsda::DAAccessor> daAccessor);

    /**
     * @brief Read a PV value from the device
     * @tparam PVType PV data type (scalar, vector, or string)
     * @param ts Output timestamp
     * @param value Output PV value
     * @param pv Node map PV description
     */
    template <ndsda::PVDataType PVType>
    Result<std::pair<PVType, timespec>> readPV(const NodeMapPV& pv);

    /**
     * @brief Write a PV value to the device
     * @tparam PVType PV data type (scalar, vector, or string)
     * @param ts Input timestamp
     * @param value PV value to write
     * @param pv Node map PV description
     */
    template <ndsda::PVDataType PVType>
    std::optional<ndsda::ndsdaError> writePV(timespec ts, const PVType& value, const NodeMapPV& pv);

private:
    /**
     * @brief ChimeraTK device handle
     */
    ChimeraTK::Device daDevice;

    /**
     * @brief Low-level accessor used for register operations
     */
    std::unique_ptr<ndsda::DAAccessor> daAccessor;
};

}  // namespace ndsda

#endif
