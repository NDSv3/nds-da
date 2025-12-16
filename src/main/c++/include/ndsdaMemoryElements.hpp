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
    MemoryElement(const std::string& name)
        : name(name), readPV(name + DAPV_GET_SUFFIX), writePV(name + DAPV_SET_SUFFIX) {};

    virtual MemoryElementType getType() const = 0;

    virtual ~MemoryElement() = default;

    std::string name, readPV, writePV;
};

using DARegAccessorVariant = std::variant<
    ChimeraTK::ScalarRegisterAccessor<uint32_t>,
    ChimeraTK::ScalarRegisterAccessor<int32_t>,
    ChimeraTK::ScalarRegisterAccessor<float> >;

struct RegisterElement : MemoryElement {
public:
    RegisterElement(const std::string& name)
        : MemoryElement(name), daAccessor(std::nullopt) {};

    MemoryElementType getType() const override { return MemoryElementType::SCALAR; }

    std::optional<DARegAccessorVariant> daAccessor;
};

using DAOneDRegAccessorVariant = std::variant<
    ChimeraTK::OneDRegisterAccessor<uint32_t>,
    ChimeraTK::OneDRegisterAccessor<int32_t>,
    ChimeraTK::OneDRegisterAccessor<float> >;
struct OneDElement : MemoryElement {
    OneDElement(const std::string& name, size_t elements, size_t elementSize)
        : MemoryElement(name),
          elements(elements),
          elementSize(elementSize),
          daAccessor(std::nullopt) {};

    MemoryElementType getType() const override { return MemoryElementType::ONE_D; }

    size_t elements, elementSize;
    std::optional<DAOneDRegAccessorVariant> daAccessor;
};

}  // namespace ndsda

#endif