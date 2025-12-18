#ifndef NDSDA_REGISTERPV_H
#define NDSDA_REGISTERPV_H

#include <optional>
#include <string>
#include <ChimeraTK/Device.h>
#include <variant>

namespace ndsda {

const std::string DAPV_GET_SUFFIX = "-Get";
const std::string DAPV_SET_SUFFIX = "-Set";

enum class MemoryElementType {
    SCALAR,
    ONE_D
};

// TODO: Extract PV name generation logic to allow parsing
struct MemoryElement {
    std::string name, readPV, writePV;
    size_t fracBits;
    bool isSigned;

    MemoryElement(const std::string& name, size_t fracBits = 0, bool isSigned = false)
        : name(name),
          readPV(name + DAPV_GET_SUFFIX),
          writePV(name + DAPV_SET_SUFFIX),
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