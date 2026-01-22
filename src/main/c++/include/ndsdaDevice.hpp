#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <memory>
#include <string>

#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"

namespace ndsda {

class Device {
private:
    std::string deviceName, deviceAlias, deviceMapFile, memoryMapFile, nodeMapFile;
    nds::Port rootNode;

    std::unique_ptr<DABridge> daBridge;

    template <typename T>
    void addScalarPV(const ndsda::NodeMapPV& pv);
    template <typename T>
    void addOneDPV(const ndsda::NodeMapPV& pv);

    std::optional<ndsda::ndsdaError> mergeMapElements(
        std::vector<std::unique_ptr<ndsda::MemoryElement>>& memoryElements, std::vector<std::shared_ptr<ndsda::NodeMapPV>>& nodeMapPVs
    );
    std::optional<ndsda::ndsdaError> generatePVs(const ndsda::NodeMapPV& nodeMapPV);

public:
    Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    ~Device();
};

}  // namespace ndsda

#endif  //! NDS_DA_MAIN_H
