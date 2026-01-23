#include "ndsdaBridge.hpp"
#include <ChimeraTK/Exception.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ctime>
#include <memory>

#include "nds3/exceptions.h"
#include "ndsdaAccessor.hpp"
#include "ndsdaCommon.hpp"

using ndsda::DABridge;

DABridge::DABridge(std::string dmapFilePath, std::string deviceName, const std::vector<std::shared_ptr<NodeMapPV>>& elements) : daAccessor(std::make_unique<ndsda::DAAccessor>(dmapFilePath, deviceName, elements)) {}

template <ndsda::PVDataType PVType>
void DABridge::readPV(timespec* ts, PVType* value, const ndsda::NodeMapPV& pv) {
    using PTE = ndsda::PVTypeEnum;

    clock_gettime(CLOCK_REALTIME, ts);
    if constexpr (PVScalarType<PVType>) {
        *value = 1234;
    } else {
        *value = {0, 1, 0, 1};
    }
}

template <ndsda::PVDataType PVType>
void DABridge::writePV(timespec ts, PVType value, const ndsda::NodeMapPV& pv) {
    (void)ts;
    (void)value;
    (void)pv;
}

// Explicit template instantiations
template void DABridge::readPV(timespec*, std::int32_t*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::int64_t*, const NodeMapPV&);
template void DABridge::readPV(timespec*, float*, const NodeMapPV&);
template void DABridge::readPV(timespec*, double*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::string*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<bool>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::uint8_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::uint16_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::uint32_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int8_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int16_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int32_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<std::int64_t>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<float>*, const NodeMapPV&);
template void DABridge::readPV(timespec*, std::vector<double>*, const NodeMapPV&);

template void DABridge::writePV(timespec, std::int32_t, const NodeMapPV&);
template void DABridge::writePV(timespec, std::int64_t, const NodeMapPV&);
template void DABridge::writePV(timespec, float, const NodeMapPV&);
template void DABridge::writePV(timespec, double, const NodeMapPV&);
template void DABridge::writePV(timespec, std::string, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<bool>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::uint8_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::uint16_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::uint32_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int8_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int16_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int32_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<std::int64_t>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<float>, const NodeMapPV&);
template void DABridge::writePV(timespec, std::vector<double>, const NodeMapPV&);