#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>
#include <string>
#include "nds3/pvVariableIn.h"

class DADevice {
private:
    std::string deviceName;
    nds::Port rootNode;

    nds::PVVariableIn<std::string> memoryMapFile, deviceMapFile;

public:
    DADevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    ~DADevice();
};

#endif  //! NDS_DA_MAIN_H
