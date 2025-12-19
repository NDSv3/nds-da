#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <memory>
#include <string>
#include "nds3/pvVariableIn.h"

#include "ndsdaFileParser.hpp"
#include "ndsdaBridge.hpp"

namespace ndsda {

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
