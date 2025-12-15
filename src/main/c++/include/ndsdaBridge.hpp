#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include <ChimeraTK/Device.h>
#include <memory>
#include "ndsdaMemoryElements.hpp"

namespace ndsda {

template <typename T>
concept DARegisterType = ChimeraTK::user_type<T>;

class DABridge {
public:
    DABridge(std::string dmapFilePath, std::string deviceName, std::vector<std::shared_ptr<MemoryElement>> elements);
    ~DABridge();

    template <DARegisterType T>
    T readRegister(const std::string& readPV);

    template <DARegisterType T>
    void writeRegister(const std::string& writePV, T value);

private:
    ChimeraTK::Device daDevice;

    std::map<std::string, std::shared_ptr<MemoryElement>> readPVMap, writePVMap;
};

}  // namespace ndsda

#endif