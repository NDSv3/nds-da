/**
 * @file ndsdaAccessor.hpp
 * @brief NDS-DA Low-level DA interface
 *
 * @author David Andrino - I2A2 UPM
 * @copyright GNU General Public License v3.0
 * @date 2026-02
 */
#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include "ndsdaCommon.hpp"
#include <shared_mutex>

namespace ndsda {

/**
 * @brief Concept to constrain register types to ChimeraTK user types
 *
 * This concept ensures that only types supported by ChimeraTK's register
 * accessor interface can be used for register operations.
 *
 * @tparam T The type to validate as a ChimeraTK user type
 */
template <typename T>
concept DARegisterType = ChimeraTK::user_type<T>;

/**
 * @class DAAccessor
 * @brief Low-level interface for ChimeraTK DeviceAccess operations
 *
 * This class provides a wrapper around ChimeraTK Device for accessing
 * hardware registers. It supports both scalar and 1D array register
 * operations with type-safe interfaces.
 */
class DAAccessor {
public:
    /**
     * @brief Construct a new DAAccessor object
     *
     * Opens a connection to the specified device using the provided device map file.
     *
     * @param dmapFilePath Path to the device map file (.dmap)
     * @param deviceName Name/alias of the device to open
     * @throws nds::NdsError If device initialization fails
     */
    DAAccessor(const std::string& dmapFilePath, const std::string& deviceName);

    /**
     * @brief Destroy the DAAccessor object
     *
     * Closes the connection to the device.
     */
    ~DAAccessor();

    /**
     * @brief Read a scalar register from the device
     *
     * Reads a single value from a scalar register. The register accessor is
     * cached in the RegisterElement for subsequent reads.
     *
     * Once a register is read with an accessor type, subsequent reads/writes to
     * the same register must use the same type.
     *
     * @tparam RegisterType The data type of the register (must satisfy DARegisterType)
     * @param registerElement The register element to read from
     * @return Result<RegisterType> The register value on success, or an error
     */
    template <DARegisterType RegisterType>
    Result<RegisterType> readRegister(const RegisterElement& registerElement);

    /**
     * @brief Write a value to a scalar register
     *
     * Writes a single value to a scalar register. The register accessor is
     * cached in the RegisterElement for subsequent writes.
     *
     * Once a register is written with an accessor type, subsequent reads/writes to
     * the same register must use the same type.
     *
     * @tparam RegisterType The data type of the register (must satisfy DARegisterType)
     * @param reg The register element to write to
     * @param value The value to write
     * @return std::optional<ndsdaError> Error if the write fails, std::nullopt on success
     */
    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeRegister(const RegisterElement& reg, RegisterType value);

    /**
     * @brief Read a 1D array register from the device
     *
     * Reads all elements from a 1D register array. The register accessor is
     * cached in the OneDElement for subsequent reads.
     *
     * Once a register is read with an accessor type, subsequent reads/writes to
     * the same register must use the same type.
     *
     * @tparam RegisterType The data type of the register elements (must satisfy DARegisterType)
     * @param reg The 1D register element to read from
     * @return Result<std::vector<RegisterType>> Vector of register values on success, or an error
     */
    template <DARegisterType RegisterType>
    Result<std::vector<RegisterType>> readOneDRegister(const OneDElement& reg);

    /**
     * @brief Write values to a 1D array register
     *
     * Writes a vector of values to a 1D register array. The register accessor is
     * cached in the OneDElement for subsequent writes.
     *
     * Once a register is written with an accessor type, subsequent reads/writes to
     * the same register must use the same type.
     *
     * @tparam RegisterType The data type of the register elements (must satisfy DARegisterType)
     * @param reg The 1D register element to write to
     * @param values Vector of values to write to the register
     * @return std::optional<ndsdaError> Error if the write fails, std::nullopt on success
     */
    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeOneDRegister(const OneDElement& reg, const std::vector<RegisterType>& values);

private:
    /**
     * @brief ChimeraTK device handle
     *
     * The underlying ChimeraTK Device object used for all hardware access operations.
     */
    ChimeraTK::Device daDevice;

    /**
     * @brief Mutex for thread-safe access to the device
     *
     * Ensures that concurrent register access operations are serialized to prevent errors
     */
    std::shared_mutex accessorMutex;
};

}  // namespace ndsda

#endif  //! NDSDA_ACCESSOR_H
