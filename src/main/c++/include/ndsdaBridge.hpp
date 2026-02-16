#ifndef NDSDA_BRIDGE_H
#define NDSDA_BRIDGE_H

#include <ChimeraTK/Device.h>
#include <memory>
#include "ndsdaCommon.hpp"
#include "ndsdaAccessor.hpp"

namespace ndsda {

class DABridge {
public:
    DABridge(std::unique_ptr<ndsda::DAAccessor> daAccessor);

    template <ndsda::PVDataType PVType>
    void readPV(timespec* ts, PVType* value, const NodeMapPV& pv);

    template <ndsda::PVDataType PVType>
    void writePV(timespec ts, const PVType& value, const NodeMapPV& pv);

private:
    ChimeraTK::Device daDevice;

    std::unique_ptr<ndsda::DAAccessor> daAccessor;
};

}  // namespace ndsda

#endif