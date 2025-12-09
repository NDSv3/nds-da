#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <string>
#include "nds3/pvVariableIn.h"

#include "ndsdaFileParser.hpp"

namespace ndsda {

class Device {
private:
    std::string deviceName;
    nds::Port rootNode;

    nds::PVVariableIn<std::string> deviceAlias, deviceMapFile;

public:
    Device(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    ~Device();
};

}  // namespace ndsda

#endif  //! NDS_DA_MAIN_H
