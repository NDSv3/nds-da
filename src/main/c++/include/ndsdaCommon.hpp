#ifndef NDSDA_REGISTERPV_H
#define NDSDA_REGISTERPV_H

#include <optional>
#include <string>
#include <ChimeraTK/Device.h>
#include <variant>

#include <boost/outcome/result.hpp>

namespace ndsda {

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
static std::string generateWritePV(const std::string& baseName) {
    return generateName(baseName) + DAPV_SET_SUFFIX;
}
static std::string generateReadPV(const std::string& baseName) {
    return generateName(baseName) + DAPV_GET_SUFFIX;
}

struct ndsdaError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct NodeMapPV {
    std::string name;
    std::string reg;
    std::string type;
    bool approximateFloat = false;
    size_t nElem          = 1;
    std::string readPV;
    std::string writePV;

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

// TODO: Extract PV name generation logic to allow parsing
struct MemoryElement {
    std::string name, readPV, writePV;
    size_t fracBits;
    bool isSigned;

    MemoryElement(const std::string& name, size_t fracBits = 0, bool isSigned = false)
        : name(generateName(name)),
          readPV(generateReadPV(name)),
          writePV(generateWritePV(name)),
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
    std::optional<DARegAccessorVariant> daAccessor;

    RegisterElement(const std::string& name, size_t fracBits, bool isSigned)
        : MemoryElement(name, fracBits, isSigned), daAccessor(std::nullopt) {};

    MemoryElementType getType() const override { return MemoryElementType::SCALAR; }

    bool operator==(const MemoryElement& other) const override {
        if (other.getType() != MemoryElementType::SCALAR) {
            return false;
        }
        const RegisterElement& otherReg = static_cast<const RegisterElement&>(other);
        return (this->name == otherReg.name) && (this->readPV == otherReg.readPV) && (this->writePV == otherReg.writePV) && (this->fracBits == otherReg.fracBits) && (this->isSigned == otherReg.isSigned);
    }
};

using DAOneDRegAccessorVariant = std::variant<
    ChimeraTK::OneDRegisterAccessor<uint32_t>,
    ChimeraTK::OneDRegisterAccessor<int32_t>>;
struct OneDElement : MemoryElement {
    size_t elements, elementSize;
    std::optional<DAOneDRegAccessorVariant> daAccessor;

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
               (this->readPV == otherOneD.readPV) &&
               (this->writePV == otherOneD.writePV) &&
               (this->elements == otherOneD.elements) &&
               (this->elementSize == otherOneD.elementSize) &&
               (this->fracBits == otherOneD.fracBits) &&
               (this->isSigned == otherOneD.isSigned);
    }
};

}  // namespace ndsda

#endif