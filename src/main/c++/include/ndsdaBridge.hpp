#ifndef NDSDA_ACCESSOR_H
#define NDSDA_ACCESSOR_H

#include <ChimeraTK/Device.h>
#include <memory>
#include "ndsdaCommon.hpp"

namespace ndsda {

template <typename T>
concept Integer32 = std::same_as<T, uint32_t> || std::same_as<T, int32_t>;

class DABridge {
public:
    DABridge(std::string dmapFilePath, std::string deviceName, std::vector<std::shared_ptr<MemoryElement>> elements);
    ~DABridge();

    template <Integer32 T>
    [[nodiscard]]
    T readRegister(const std::string& readPV);

    template <Integer32 T>
    void writeRegister(const std::string& writePV, T value);

    template <Integer32 T>
    [[nodiscard]]
    std::vector<T> readOneDRegister(const std::string& readPV);

    template <Integer32 T>
    void writeOneDRegister(const std::string& writePV, const std::vector<T>& values);

private:
    ChimeraTK::Device daDevice;

    std::map<std::string, std::shared_ptr<MemoryElement>> readPVMap, writePVMap;
};

}  // namespace ndsda

#endif