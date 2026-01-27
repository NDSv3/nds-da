#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include "ndsdaCommon.hpp"

namespace ndsda {

template <typename T>
concept DARegisterType = std::same_as<T, uint32_t> || std::same_as<T, int32_t>;

class DAAccessor {
public:
    DAAccessor(std::string dmapFilePath, std::string deviceName);
    ~DAAccessor();

    template <DARegisterType RegisterType>
    Result<RegisterType> readRegister(const RegisterElement& registerElement);

    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeRegister(const RegisterElement& reg, RegisterType value);

    template <DARegisterType RegisterType>
    Result<std::vector<RegisterType>> readOneDRegister(const OneDElement& reg);

    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeOneDRegister(const OneDElement& reg, const std::vector<RegisterType>& values);

private:
    ChimeraTK::Device daDevice;
};

}  // namespace ndsda

#endif  //! NDSDA_ACCESSOR_H
