#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include <ChimeraTK/Device.h>
#include "ndsdaRegisterAccessor.hpp"

namespace ndsda {

template <typename T>
concept DARegisterType = ChimeraTK::user_type<T>;

class DABridge {
public:
    DABridge(std::string dmapFilePath, std::string deviceName, std::vector<RegisterAccessor> registers);
    ~DABridge();

    template <DARegisterType T>
    T readRegister(const std::string& readPV);

    template <DARegisterType T>
    void writeRegister(const std::string& writePV, T value);

private:
    ChimeraTK::Device daDevice;

    std::map<std::string, std::shared_ptr<RegisterAccessor>> regReadMap, regWriteMap;
};

}  // namespace ndsda

#endif