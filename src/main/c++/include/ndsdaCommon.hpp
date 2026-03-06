/**
 * @file ndsdaCommon.hpp
 * @brief NDS-DA Common definitions and utilities
 *
 * @author David Andrino - I2A2 UPM
 * @copyright GNU General Public License v3.0
 * @date 2026-02
 */
#ifndef NDSDA_REGISTERPV_H
#define NDSDA_REGISTERPV_H

#include <optional>
#include <string>
#include <ChimeraTK/Device.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <variant>
#include <type_traits>

#include <boost/outcome/result.hpp>

namespace ndsda {

// clang-format off
/**
 * @brief PV scalar types that fit in a 32-bit word
 */
template <typename T>
concept PVWordType = (std::same_as<T, std::int32_t> || std::same_as<T, float>);

/**
 * @brief PV scalar types that fit in a 64-bit word
 */
template <typename T>
concept PVDoubleWordType = ( std::same_as<T, std::int64_t> || std::same_as<T, double> );

/**
 * @brief PV vector types that contain 32-bit elements
 */
template <typename T>
concept PVWordVectorType = ( std::same_as<T, std::vector<bool>> ||
    std::same_as<T, std::vector<std::uint8_t>>  || std::same_as<T, std::vector<std::uint16_t>> ||
    std::same_as<T, std::vector<std::uint32_t>> ||
    std::same_as<T, std::vector<std::int8_t>>   || std::same_as<T, std::vector<std::int16_t>>  ||
    std::same_as<T, std::vector<std::int32_t>>  || std::same_as<T, std::vector<float>> );

/**
 * @brief PV vector types that contain 64-bit elements
 */
template <typename T>
concept PVDoubleWordVectorType = ( std::same_as<T, std::vector<std::int64_t>> || std::same_as<T, std::vector<double>> );

/**
 * @brief All supported scalar PV types
 */
template <typename T>
concept PVScalarType = ( PVWordType<T> || PVDoubleWordType<T> );

/**
 * @brief All supported vector PV types
 */
template <typename T>
concept PVVectorType = ( PVWordVectorType<T> || PVDoubleWordVectorType<T> || std::same_as<T, std::string> );

/**
 * @brief All supported PV data types
 */
template <typename T>
concept PVDataType = (PVScalarType<T> || PVVectorType<T>);

/**
 * @brief Runtime PV type identifiers
 */
enum class PVTypeEnum {
    INT32, INT64, FLOAT, DOUBLE, STRING,
    VEC_BOOL, VEC_UINT8, VEC_UINT16, VEC_UINT32,
    VEC_INT8, VEC_INT16, VEC_INT32, VEC_INT64,
    VEC_FLOAT, VEC_DOUBLE
};

/**
 * @brief Visit a runtime PV type as a compile-time type
 * @tparam V Callable visitor type
 * @param type PV type identifier
 * @param visitor Callable invoked with std::type_identity<T>
 */
template <typename V>
void visitPVType(PVTypeEnum type, V&& visitor) {
    switch (type) {
        case PVTypeEnum::INT32:      visitor(std::type_identity<int32_t>{});               break;
        case PVTypeEnum::INT64:      visitor(std::type_identity<int64_t>{});               break;
        case PVTypeEnum::FLOAT:      visitor(std::type_identity<float>{});                 break;
        case PVTypeEnum::DOUBLE:     visitor(std::type_identity<double>{});                break;
        case PVTypeEnum::STRING:     visitor(std::type_identity<std::string>{});           break;
        case PVTypeEnum::VEC_BOOL:   visitor(std::type_identity<std::vector<bool>>{});     break;
        case PVTypeEnum::VEC_UINT8:  visitor(std::type_identity<std::vector<uint8_t>>{});  break;
        case PVTypeEnum::VEC_UINT16: visitor(std::type_identity<std::vector<uint16_t>>{}); break;
        case PVTypeEnum::VEC_UINT32: visitor(std::type_identity<std::vector<uint32_t>>{}); break;
        case PVTypeEnum::VEC_INT8:   visitor(std::type_identity<std::vector<int8_t>>{});   break;
        case PVTypeEnum::VEC_INT16:  visitor(std::type_identity<std::vector<int16_t>>{});  break;
        case PVTypeEnum::VEC_INT32:  visitor(std::type_identity<std::vector<int32_t>>{});  break;
        case PVTypeEnum::VEC_INT64:  visitor(std::type_identity<std::vector<int64_t>>{});  break;
        case PVTypeEnum::VEC_FLOAT:  visitor(std::type_identity<std::vector<float>>{});    break;
        case PVTypeEnum::VEC_DOUBLE: visitor(std::type_identity<std::vector<double>>{});   break;
    }
}

template <typename T>
inline constexpr PVTypeEnum TtoPVTypeEnum = []() { static_assert(false, "Unsupported PV data type"); };

template <> inline constexpr PVTypeEnum TtoPVTypeEnum<int32_t>               = PVTypeEnum::INT32;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<int64_t>               = PVTypeEnum::INT64;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<float>                 = PVTypeEnum::FLOAT;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<double>                = PVTypeEnum::DOUBLE;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::string>           = PVTypeEnum::STRING;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<bool>>     = PVTypeEnum::VEC_BOOL;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<uint8_t>>  = PVTypeEnum::VEC_UINT8;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<uint16_t>> = PVTypeEnum::VEC_UINT16;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<uint32_t>> = PVTypeEnum::VEC_UINT32;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<int8_t>>   = PVTypeEnum::VEC_INT8;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<int16_t>>  = PVTypeEnum::VEC_INT16;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<int32_t>>  = PVTypeEnum::VEC_INT32;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<int64_t>>  = PVTypeEnum::VEC_INT64;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<float>>    = PVTypeEnum::VEC_FLOAT;
template <> inline constexpr PVTypeEnum TtoPVTypeEnum<std::vector<double>>   = PVTypeEnum::VEC_DOUBLE;
// clang-format on

/**
 * @brief Check whether a PV type is scalar
 * @param type PV type identifier
 * @return true if the type is scalar
 */
constexpr bool pvTypeEnum_isScalar(PVTypeEnum type) {
    return (type == PVTypeEnum::INT32 || type == PVTypeEnum::INT64 || type == PVTypeEnum::FLOAT || type == PVTypeEnum::DOUBLE || type == PVTypeEnum::STRING);
}

/**
 * @brief Suffix appended to generated read PV names
 */
const std::string DAPV_GET_SUFFIX = "Get";
/**
 * @brief Suffix appended to generated write PV names
 */
const std::string DAPV_SET_SUFFIX = "Set";

/**
 * @brief Memory element category
 */
enum class MemoryElementType {
    SCALAR,
    ONE_D
};

/**
 * @brief Normalize DA register names by stripping the leading '.'
 * @param baseName Raw register name
 * @return Normalized register name
 */
const inline std::string generateRegName(const std::string& baseName) {
    if (baseName.size() > 0 && baseName[0] == '.')
        return baseName.substr(1);
    else
        return baseName;
}

/**
 * @brief NDS-DA runtime error
 */
struct ndsdaError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * @brief IEEE 754 fractional bits sentinel for bit-cast floats
 */
static const size_t NDSDA_MEMORYELEMENT_IEEE754_FRACBITS = -1;

/**
 * @brief Base class for DeviceAccess memory elements.
 */
struct MemoryElement {
    std::string name;
    size_t fracBits;
    size_t elementWidthBits;
    bool isSigned;

    MemoryElement(const std::string& name, size_t elementWidthBits = 32, size_t fracBits = 0, bool isSigned = false)
        : name(generateRegName(name)),
          fracBits(fracBits),
          elementWidthBits(elementWidthBits),
          isSigned(isSigned) {};

    virtual MemoryElementType getType() const = 0;
    virtual ~MemoryElement()                  = default;

    virtual bool operator==(const MemoryElement& other) const = 0;
};

/**
 * @brief Generator template for ScalarRegisterAccessor variants. Instantiated with a macro for all
 * DeviceAccess-supported types to create the DARegAccessorVariant type alias.
 */
template <typename... Ts>
using VariantScalarAccessorGenerator = std::variant<ChimeraTK::ScalarRegisterAccessor<Ts>...>;
/**
 * @brief Generator template for OneDRegisterAccessor variants. Instantiated with a macro for all DeviceAccess-supported types to create the DAOneDRegAccessorVariant type alias.
 */
template <typename... Ts>
using VariantOneDAccessorGenerator = std::variant<ChimeraTK::OneDRegisterAccessor<Ts>...>;

#define DA_ACCESSOR_TYPE_LIST int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, \
                              int64_t, uint64_t, float, double, std::string,         \
                              ChimeraTK::Boolean, ChimeraTK::Void

/**
 * @brief Variant type for DeviceAccess scalar register accessors.
 */
using DARegAccessorVariant = VariantScalarAccessorGenerator<DA_ACCESSOR_TYPE_LIST>;

/**
 * @brief Variant type for DeviceAccess 1D register accessors.
 */
using DAOneDRegAccessorVariant = VariantOneDAccessorGenerator<DA_ACCESSOR_TYPE_LIST>;
#undef DA_ACCESSOR_TYPE_LIST

/**
 * @class RegisterElement
 * @brief Scalar register element definition
 *
 * Represents a single scalar register from the DeviceAccess memory map.
 * This class extends MemoryElement to add a cached register accessor.
 */
struct RegisterElement : MemoryElement {
public:
    /**
     * @brief Cached DeviceAccess scalar register accessor
     *
     * Stored in a variant to support multiple data types. Lazily initialized
     * on first read/write operation. Once initialized with a type, subsequent
     * operations must use the same type.
     */
    mutable std::optional<DARegAccessorVariant> daAccessor;  // Lazy initialization

    /**
     * @brief Construct a scalar register element
     *
     * @param name Register name (as found in memory map)
     * @param elementSizeBits Register width in bits
     * @param fracBits Number of fractional bits (for fixed-point conversion). There is a sentinel value for IEEE 754 floats.
     * @param isSigned Whether the register is signed
     */
    RegisterElement(const std::string& name, size_t elementSizeBits, size_t fracBits, bool isSigned)
        : MemoryElement(name, elementSizeBits, fracBits, isSigned), daAccessor(std::nullopt) {};

    /**
     * @brief Get the element type
     * @return MemoryElementType::SCALAR
     */
    MemoryElementType getType() const override { return MemoryElementType::SCALAR; }

    /**
     * @brief Compare this register with another memory element
     *
     * Two registers are equal if they have the same name, fractional bits,
     * signedness, and element width.
     *
     * @param other The memory element to compare with
     * @return true if both are RegisterElements with identical properties
     */
    bool operator==(const MemoryElement& other) const override {
        if (other.getType() != MemoryElementType::SCALAR) {
            return false;
        }
        const RegisterElement& otherReg = static_cast<const RegisterElement&>(other);
        return (this->name == otherReg.name) && (this->fracBits == otherReg.fracBits) && (this->isSigned == otherReg.isSigned) && (this->elementWidthBits == otherReg.elementWidthBits);
    }
};

/**
 * @class OneDElement
 * @brief 1D (array) register element definition
 *
 * Represents a one-dimensional array of register elements from the DeviceAccess
 * memory map. This class extends MemoryElement to add array-specific properties
 * and a cached 1D register accessor.
 */
struct OneDElement : MemoryElement {
    /**
     * @brief Number of elements in this 1D register
     */
    size_t elements;

    /**
     * @brief Cached DeviceAccess 1D register accessor
     *
     * Stored in a variant to support multiple data types. Lazily initialized
     * on first read/write operation. Once initialized with a type, subsequent
     * operations must use the same type.
     */
    mutable std::optional<DAOneDRegAccessorVariant> daAccessor;  // Lazy initialization

    /**
     * @brief Construct a 1D register element
     *
     * @param name Register name (as found in memory map)
     * @param elements Number of elements in the array
     * @param elementSize Width of each element in bits
     * @param fracBits Number of fractional bits per element (for fixed-point conversion). There is a sentinel value for IEEE 754 floats.
     * @param isSigned Whether elements are signed, default false
     */
    OneDElement(const std::string& name, size_t elements, size_t elementSize, size_t fracBits = 0, bool isSigned = false)
        : MemoryElement(name, elementSize, fracBits, isSigned),
          elements(elements),
          daAccessor(std::nullopt) {};

    /**
     * @brief Get the element type
     * @return MemoryElementType::ONE_D
     */
    MemoryElementType getType() const override { return MemoryElementType::ONE_D; }

    /**
     * @brief Compare this 1D register with another memory element
     *
     * Two 1D elements are equal if they have the same name, element count,
     * fractional bits, signedness, and element width.
     *
     * @param other The memory element to compare with
     * @return true if both are OneDElements with identical properties
     */
    bool operator==(const MemoryElement& other) const override {
        if (other.getType() != MemoryElementType::ONE_D) {
            return false;
        }
        const OneDElement& otherOneD = static_cast<const OneDElement&>(other);
        return (this->name == otherOneD.name) &&
               (this->elements == otherOneD.elements) &&
               (this->fracBits == otherOneD.fracBits) &&
               (this->isSigned == otherOneD.isSigned) &&
               (this->elementWidthBits == otherOneD.elementWidthBits);
    }
};

/**
 * @class NodeMapPV
 * @brief Node map PV (Process Variable) definition
 *
 * Represents a mapping from an NDS PV to a DeviceAccess register. Combines
 * PV metadata with the underlying memory element.
 */
struct NodeMapPV {
    /**
     * @brief NDS PV name
     *
     * Base name of the pv, used to generate the read and write PVs (if not overridden)
     */
    std::string name;

    /**
     * @brief DeviceAccess register name
     *
     * The register name in the device memory map. Automatically normalized
     * by removing a leading '.' if present.
     */
    std::string reg;

    /**
     * @brief PV data type
     *
     * Runtime type identifier indicating the data type of the PV
     * (e.g., INT32, FLOAT, VEC_DOUBLE).
     */
    ndsda::PVTypeEnum type;

    /**
     * @brief Number of elements
     *
     * For scalar PVs, always 1. For vector PVs, the vector length.
     */
    size_t nElem;

    /**
     * @brief NDS PV name for reading
     *
     * The automatically generated NDS PV name used for read operations.
     * Typically constructed as: name + DAPV_GET_SUFFIX
     */
    std::string readPV;

    /**
     * @brief NDS PV name for writing
     *
     * The automatically generated NDS PV name used for write operations.
     * Typically constructed as: name + DAPV_SET_SUFFIX
     */
    std::string writePV;

    /**
     * @brief Underlying memory element
     *
     * Pointer to either a RegisterElement (scalar) or OneDElement (array).
     * Owned by this object. Unique ownership prevents copying.
     */
    std::unique_ptr<MemoryElement> memoryElement;  // Deletes copy constructor

    /**
     * @brief Construct a node map PV
     *
     * @param name NDS PV name
     * @param reg Device register name (will be normalized)
     * @param type Data type of the PV
     * @param readPV NDS read PV name
     * @param writePV NDS write PV name
     * @param nElem Number of elements (default 1 for scalar)
     * @param memoryElement Pointer to the underlying register element (default nullptr)
     */
    NodeMapPV(const std::string& name, const std::string& reg, ndsda::PVTypeEnum type, const std::string& readPV, const std::string& writePV, size_t nElem = 1, std::unique_ptr<MemoryElement> memoryElement = nullptr)
        : name(name),
          reg(generateRegName(reg)),
          type(type),
          nElem(nElem),
          readPV(readPV),
          writePV(writePV),
          memoryElement(std::move(memoryElement)) {}

    /**
     * @brief Compare two node map PVs for equality
     *
     * Two NodeMapPVs are equal if they have the same name, register, type,
     * element count, read PV name, and write PV name. The memory element
     * pointer is not compared.
     *
     * @param other The PV to compare with
     * @return true if both PVs have identical metadata
     */
    bool operator==(const NodeMapPV& other) const {
        return (this->name == other.name) &&
               (this->reg == other.reg) &&
               (this->type == other.type) &&
               (this->nElem == other.nElem) &&
               (this->readPV == other.readPV) &&
               (this->writePV == other.writePV);
    }
};

/**
 * @brief Result type alias for NDS-DA operations
 * @tparam T Value type
 * @tparam E Error type
 */
template <typename T, typename E = ndsdaError>
using Result = boost::outcome_v2::result<T, E>;

}  // namespace ndsda

#endif
