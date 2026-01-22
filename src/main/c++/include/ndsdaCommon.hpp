#ifndef NDSDA_REGISTERPV_H
#define NDSDA_REGISTERPV_H

#include <optional>
#include <string>
#include <ChimeraTK/Device.h>
#include <variant>
#include <type_traits>

#include <boost/outcome/result.hpp>

namespace ndsda {

// clang-format off
template <typename T>
concept PVScalarType = (
    std::same_as<T, std::int32_t> || std::same_as<T, std::int64_t> ||
    std::same_as<T, float>        || std::same_as<T, double> );

template <typename T>
concept PVVectorType = (
    std::same_as<T, std::vector<bool>>          ||
    std::same_as<T, std::vector<std::uint8_t>>  || std::same_as<T, std::vector<std::uint16_t>> ||
    std::same_as<T, std::vector<std::uint32_t>> ||
    std::same_as<T, std::vector<std::int8_t>>   || std::same_as<T, std::vector<std::int16_t>>  ||
    std::same_as<T, std::vector<std::int32_t>>  || std::same_as<T, std::vector<std::int64_t>>  ||
    std::same_as<T, std::vector<float>>         || std::same_as<T, std::vector<double>>        ||
    std::same_as<T, std::string> );

enum class PVTypeEnum {
    INT32, INT64, FLOAT, DOUBLE, STRING,
    VEC_BOOL, VEC_UINT8, VEC_UINT16, VEC_UINT32,
    VEC_INT8, VEC_INT16, VEC_INT32, VEC_INT64,
    VEC_FLOAT, VEC_DOUBLE
};

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
// clang-format on

constexpr bool pvTypeEnum_isScalar(PVTypeEnum type) {
    return (type == PVTypeEnum::INT32 || type == PVTypeEnum::INT64 || type == PVTypeEnum::FLOAT || type == PVTypeEnum::DOUBLE || type == PVTypeEnum::STRING);
}

template <typename T>
concept PVDataType = (PVScalarType<T> || PVVectorType<T>);
// clang-format on

template <typename T, typename E>
using Result = boost::outcome_v2::result<T, E>;

const std::string DAPV_GET_SUFFIX = "-Get";
const std::string DAPV_SET_SUFFIX = "-Set";

enum class MemoryElementType {
    SCALAR,
    ONE_D
};

static std::string generateName(const std::string& baseName) {
    if (baseName.size() > 0 && baseName[0] == '.')
        return baseName.substr(1);
    else
        return baseName;
}

struct ndsdaError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct MemoryElement {
    std::string name;
    size_t fracBits;
    bool isSigned;

    MemoryElement(const std::string& name, size_t fracBits = 0, bool isSigned = false)
        : name(generateName(name)),
          fracBits(fracBits),
          isSigned(isSigned) {};

    virtual MemoryElementType getType() const = 0;
    virtual ~MemoryElement()                  = default;

    virtual bool operator==(const MemoryElement& other) const = 0;
};

using DARegAccessorVariant = std::variant<
    ChimeraTK::ScalarRegisterAccessor<uint32_t>,
    ChimeraTK::ScalarRegisterAccessor<int32_t>>;

struct RegisterElement : MemoryElement {
public:
    mutable std::optional<DARegAccessorVariant> daAccessor;  // Lazy initialization

    RegisterElement(const std::string& name, size_t fracBits, bool isSigned)
        : MemoryElement(name, fracBits, isSigned), daAccessor(std::nullopt) {};

    MemoryElementType getType() const override { return MemoryElementType::SCALAR; }

    bool operator==(const MemoryElement& other) const override {
        if (other.getType() != MemoryElementType::SCALAR) {
            return false;
        }
        const RegisterElement& otherReg = static_cast<const RegisterElement&>(other);
        return (this->name == otherReg.name) && (this->fracBits == otherReg.fracBits) && (this->isSigned == otherReg.isSigned);
    }
};

using DAOneDRegAccessorVariant = std::variant<
    ChimeraTK::OneDRegisterAccessor<uint32_t>,
    ChimeraTK::OneDRegisterAccessor<int32_t>>;
struct OneDElement : MemoryElement {
    size_t elements, elementSize;
    mutable std::optional<DAOneDRegAccessorVariant> daAccessor;  // Lazy initialization

    OneDElement(const std::string& name, size_t elements, size_t elementSize, size_t fracBits = 0, bool isSigned = false)
        : MemoryElement(name, fracBits, isSigned),
          elements(elements),
          elementSize(elementSize),
          daAccessor(std::nullopt) {};

    MemoryElementType getType() const override { return MemoryElementType::ONE_D; }

    bool operator==(const MemoryElement& other) const override {
        if (other.getType() != MemoryElementType::ONE_D) {
            return false;
        }
        const OneDElement& otherOneD = static_cast<const OneDElement&>(other);
        return (this->name == otherOneD.name) &&
               (this->elements == otherOneD.elements) &&
               (this->elementSize == otherOneD.elementSize) &&
               (this->fracBits == otherOneD.fracBits) &&
               (this->isSigned == otherOneD.isSigned);
    }
};

struct NodeMapPV {
    std::string name;
    std::string reg;
    ndsda::PVTypeEnum type;
    bool approximateFloat = false;
    size_t nElem          = 1;
    std::string readPV;
    std::string writePV;
    std::unique_ptr<MemoryElement> memoryElement;  // Deletes copy constructor

    bool operator==(const NodeMapPV& other) const {
        return (this->name == other.name) &&
               (this->reg == other.reg) &&
               (this->type == other.type) &&
               (this->approximateFloat == other.approximateFloat) &&
               (this->nElem == other.nElem) &&
               (this->readPV == other.readPV) &&
               (this->writePV == other.writePV);
    }
};

}  // namespace ndsda

#endif