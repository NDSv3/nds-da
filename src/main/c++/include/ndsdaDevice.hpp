/**
 * @file ndsdaDevice.hpp
 * @brief NDS-DA Device base class definition
 *
 * @author David Andrino - I2A2 UPM
 * @copyright 2026 I2A2-UPM
 * @date 2026-02
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <memory>
#include <string>

#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"

namespace ndsda {

/**
 * @class ndsdaDevice
 * @brief Base NDS device implementation backed by DeviceAccess. Should be inherited to implement
 * specific device drivers.
 *
 * The derived constructor must call the base constructor with the NDS factory, device name and
 * configuration parameters and **MUST** call rootNode.initialize(this, factory) after the base
 * constructor to register PVs. Additional PVs and nodes can be added to the rootNode before
 * this call.
 *
 * The required configuration parameters are:
 * - DeviceAlias: Alias of the device in the device map file
 * - DeviceMapFile: Path to the device map file (.dmap)
 * - NodeMapFile: Path to the node map file (.xml)
 *
 * On initialization, the device will parse the device map to find the memory map file, then parse
 * the memory map and node map files to create the appropriate PVs and register them in the NDS tree.
 *
 * Minimal inheritance example:
 *
 * ```cpp
 * #include <ndsdaDevice.hpp>
 *
 * class MyDevice : public ndsda::ndsdaDevice {
 * public:
 *    MyDevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params)
 *        : ndsda::ndsdaDevice(factory, deviceName, params) {
 *
 *        // Additional PVs and nodes
 *
 *        rootNode.initialize(this, factory);
 *    };
 * }
 * ```
 */
class ndsdaDevice {
private:
    /**
     * @brief Device identifiers and file paths
     */
    std::string deviceName, deviceAlias, deviceMapFile, memoryMapFile, nodeMapFile;

    /**
     * @brief Lower level reference
     */
    std::unique_ptr<DABridge> daBridge;
    /**
     * @brief Parsed node map PV definitions
     */
    std::vector<std::shared_ptr<ndsda::NodeMapPV>> nodeMapPVs;

    /**
     * @brief Add scalar PV to NDS based on PV description
     * @tparam T PV data type
     * @param pv Node map PV description
     */
    template <typename T>
    void addScalarPV(const ndsda::NodeMapPV& pv);
    /**
     * @brief Add 1D PV to NDS based on PV description
     * @tparam T PV data type
     * @param pv Node map PV description
     */
    template <typename T>
    void addOneDPV(const ndsda::NodeMapPV& pv);

    /**
     * @brief Add reference to the lower level MemoryElement to the high-level NodeMapPV
     *
     * Calls generatePV to add each one to the rootNode. The memoryElements are moved from
     * the vector and are REPLACED WITH NULLPTR in the vector.
     *
     * @param memoryElements Parsed memory elements
     * @param nodeMapPVs Parsed PV definitions
     * @return std::optional<ndsda::ndsdaError> Error if merge fails
     */
    std::optional<ndsda::ndsdaError> mergeMapElements(
        std::vector<std::unique_ptr<ndsda::MemoryElement>>& memoryElements, std::vector<std::shared_ptr<ndsda::NodeMapPV>>& nodeMapPVs
    );
    /**
     * @brief Generate NDS PVs for a node map entry. Creates the appropriate delegate
     * PVs based on the type and adds them to the rootNode.
     *
     * @param nodeMapPV PV description
     * @return std::optional<ndsda::ndsdaError> Error if PV generation fails
     */
    std::optional<ndsda::ndsdaError> generatePVs(const ndsda::NodeMapPV& nodeMapPV);

protected:
    /**
     * @brief Construct the base device and register PVs
     *
     * The required configuration parameters are:
     * - DeviceAlias: Alias of the device in the device map file
     * - DeviceMapFile: Path to the device map file (.dmap)
     * - NodeMapFile: Path to the node map file (.xml)
     *
     * @param factory NDS factory reference
     * @param deviceName Device name used for PV root
     * @param params Device configuration parameters
     */
    ndsdaDevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    /**
     * @brief Destroy the device and release resources
     */
    ~ndsdaDevice();

    /**
     * @brief Root NDS node for PV registration
     */
    nds::Port rootNode;

public:
    template <PVDataType T>
    Result<std::pair<T, timespec>> readPV(const std::string& pv);

    template <PVDataType T>
    std::optional<ndsda::ndsdaError> writePV(const std::string& pv, const T& value, timespec ts);
};

}  // namespace ndsda

#endif  //! NDS_DA_MAIN_H
