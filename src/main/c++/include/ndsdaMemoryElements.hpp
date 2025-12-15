#ifndef NDSDA_REGISTERPV_H
#define NDSDA_REGISTERPV_H

#include <optional>
#include <string>
#include <ChimeraTK/Device.h>
#include <variant>

namespace ndsda {

enum class MemoryElementType {
    SCALAR,
    ONE_D
};

struct MemoryElement {
    MemoryElement(const std::string& name, const std::string& readPV, const std::string& writePV)
        : name(name), readPV(readPV), writePV(writePV) {};

    virtual MemoryElementType getType() const = 0;

    std::string name, readPV, writePV;
};

using DARegAccessorVariant = std::variant<
    ChimeraTK::ScalarRegisterAccessor<uint32_t>,
    ChimeraTK::ScalarRegisterAccessor<int32_t>,
    ChimeraTK::ScalarRegisterAccessor<float> >;

struct RegisterElement : MemoryElement {
public:
    RegisterElement(const std::string& name, const std::string& readPV, const std::string& writePV)
        : MemoryElement(name, readPV, writePV), daAccessor(std::nullopt) {};

    ~RegisterElement();

    MemoryElementType getType() const override { return MemoryElementType::SCALAR; }

    std::optional<DARegAccessorVariant> daAccessor;
};

using DAOneDRegAccessorVariant = std::variant<
    ChimeraTK::OneDRegisterAccessor<uint32_t>,
    ChimeraTK::OneDRegisterAccessor<int32_t>,
    ChimeraTK::OneDRegisterAccessor<float> >;
struct OneDElement : MemoryElement {
    OneDElement(const std::string& name, const std::string& readPV, const std::string& writePV, size_t elements, size_t elementSize)
        : MemoryElement(name, readPV, writePV),
          elements(elements),
          elementSize(elementSize),
          daAccessor(std::nullopt) {};

    ~OneDElement();

    MemoryElementType getType() const override { return MemoryElementType::ONE_D; }

    size_t elements, elementSize;
    std::optional<DAOneDRegAccessorVariant> daAccessor;
};

}  // namespace ndsda

#endif