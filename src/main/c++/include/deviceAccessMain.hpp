#ifndef NDS_DA_MAIN_H
#define NDS_DA_MAIN_H

#include <nds3/nds.h>

class DADevice {
private:
    std::string deviceName;
    nds::Port rootNode;
    nds::PVVariableIn<int32_t> testPV;

public:
    DADevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& params);
    ~DADevice();
};

#endif  //! NDS_DA_MAIN_H
