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

    MemoryElement(const std::string& name)
        : name(name), readPV(name + DAPV_GET_SUFFIX), writePV(name + DAPV_SET_SUFFIX) {};

    virtual MemoryElementType getType() const = 0;
    virtual ~MemoryElement()                  = default;

    virtual bool operator==(const MemoryElement& other) const = 0;
};

using DARegAccessorVariant = std::variant<
    ChimeraTK::ScalarRegisterAccessor<uint32_t>,
    ChimeraTK::ScalarRegisterAccessor<int32_t>,
    ChimeraTK::ScalarRegisterAccessor<float> >;

struct RegisterElement : MemoryElement {
public:
    std::optional<DARegAccessorVariant> daAccessor;

    RegisterElement(const std::string& name)
        : MemoryElement(name), daAccessor(std::nullopt) {};

    MemoryElementType getType() const override { return MemoryElementType::SCALAR; }

    bool operator==(const MemoryElement& other) const override {
        if (other.getType() != MemoryElementType::SCALAR) {
            return false;
        }
        const RegisterElement& otherReg = static_cast<const RegisterElement&>(other);
        return (this->name == otherReg.name) && (this->readPV == otherReg.readPV) && (this->writePV == otherReg.writePV);
    }
};

using DAOneDRegAccessorVariant = std::variant<
    ChimeraTK::OneDRegisterAccessor<uint32_t>,
    ChimeraTK::OneDRegisterAccessor<int32_t>,
    ChimeraTK::OneDRegisterAccessor<float> >;
struct OneDElement : MemoryElement {
    size_t elements, elementSize;
    std::optional<DAOneDRegAccessorVariant> daAccessor;

    OneDElement(const std::string& name, size_t elements, size_t elementSize)
        : MemoryElement(name),
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
               (this->elementSize == otherOneD.elementSize);
    }
};

}  // namespace ndsda

#endif