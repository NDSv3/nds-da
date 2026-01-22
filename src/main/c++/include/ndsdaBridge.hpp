#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include <ChimeraTK/Device.h>
#include <memory>
#include "ndsdaCommon.hpp"

namespace ndsda {

template <typename T>
concept DARegisterType = std::same_as<T, uint32_t> || std::same_as<T, int32_t>;

class DABridge {
public:
    DABridge(std::string dmapFilePath, std::string deviceName, const std::vector<std::shared_ptr<NodeMapPV>>& elements);
    ~DABridge();

    template <ndsda::PVDataType PVType>
    void readPV(timespec* ts, PVType* value, const NodeMapPV& pv);

    template <ndsda::PVDataType PVType>
    void writePV(timespec ts, PVType value, const NodeMapPV& pv);

private:
    ChimeraTK::Device daDevice;
    std::map<std::string, std::shared_ptr<NodeMapPV>> readPVMap, writePVMap;

    template <DARegisterType RegisterType>
    Result<RegisterType, ndsdaError> readRegister(const RegisterElement& registerElement);

    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeRegister(const RegisterElement& reg, RegisterType value);

    template <DARegisterType RegisterType>
    Result<std::vector<RegisterType>, ndsdaError> readOneDRegister(const OneDElement& reg);

    template <DARegisterType RegisterType>
    std::optional<ndsdaError> writeOneDRegister(const OneDElement& reg, const std::vector<RegisterType>& values);
};

}  // namespace ndsda

#endif