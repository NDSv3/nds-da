#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <memory>
#include <string>
#include "nds3/pvVariableIn.h"

#include "ndsdaFileParser.hpp"
#include "ndsdaBridge.hpp"

namespace ndsda {

// clang-format off
template <typename T>
concept PVScalarType = (
    std::same_as<T, std::int32_t> || std::same_as<T, std::int64_t> ||
    std::same_as<T, float>        || std::same_as<T, double>       ||
    std::same_as<T, std::string> );

template <typename T>
concept PVVectorType = (
    std::same_as<T, std::vector<bool>>          ||
    std::same_as<T, std::vector<std::uint8_t>>  || std::same_as<T, std::vector<std::uint16_t>> ||
    std::same_as<T, std::vector<std::uint32_t>> ||
    std::same_as<T, std::vector<std::int8_t>>   || std::same_as<T, std::vector<std::int16_t>>  ||
    std::same_as<T, std::vector<std::int32_t>>  || std::same_as<T, std::vector<std::int64_t>>  ||
    std::same_as<T, std::vector<float>>         || std::same_as<T, std::vector<double>> );

template <typename T>
concept PVDataType = (PVScalarType<T> || PVVectorType<T>);
// clang-format on

class Device {
private:
    std::string deviceName;
    nds::Port rootNode;

    std::unique_ptr<FileParser> fileParser;
    std::unique_ptr<DABridge> daBridge;

    nds::PVVariableIn<std::string> deviceAlias, deviceMapFile;
    nds::PVDelegateIn<std::string> memoryMapFile;

    template <typename T>
    void addScalarPV(const std::shared_ptr<MemoryElement>& element);
    template <typename T>
    void addOneDPV(const std::shared_ptr<MemoryElement>& element);

public:
    Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    ~Device();
};

}  // namespace ndsda

#endif  //! NDS_DA_MAIN_H
