#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <memory>
#include <string>
#include "nds3/pvVariableIn.h"

#include "ndsdaFileParser.hpp"

namespace ndsda {

class Device {
private:
    std::string deviceName;
    nds::Port rootNode;

    std::unique_ptr<FileParser> fileParser;

    nds::PVVariableIn<std::string> deviceAlias, deviceMapFile;
    nds::PVDelegateIn<std::string> memoryMapFile;

public:
    Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    ~Device();
};

}  // namespace ndsda

#endif  //! NDS_DA_MAIN_H
