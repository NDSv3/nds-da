#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include "ndsdaCommon.hpp"

namespace ndsda {

template <typename T>
concept DARegisterType = std::same_as<T, uint32_t> || std::same_as<T, int32_t>;

class DAAccessor {
public:
    DAAccessor(std::string dmapFilePath, std::string deviceName, const std::vector<std::shared_ptr<NodeMapPV>>& elements);
    ~DAAccessor();

    template <DARegisterType RegisterType>
    Result<RegisterType, ndsdaError> readRegister(const RegisterElement& registerElement);

    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeRegister(const RegisterElement& reg, RegisterType value);

    template <DARegisterType RegisterType>
    Result<std::vector<RegisterType>, ndsdaError> readOneDRegister(const OneDElement& reg);

    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeOneDRegister(const OneDElement& reg, const std::vector<RegisterType>& values);

private:
    ChimeraTK::Device daDevice;
    std::map<std::string, std::shared_ptr<NodeMapPV>> readPVMap, writePVMap;
};

}  // namespace ndsda

#endif  //! NDSDA_ACCESSOR_H
