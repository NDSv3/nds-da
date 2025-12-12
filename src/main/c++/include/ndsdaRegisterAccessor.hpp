#ifndef NDSDA_REGISTERPV_H
#define NDSDA_REGISTERPV_H

#include <optional>
#include <string>
#include <ChimeraTK/Device.h>
#include <variant>

namespace ndsda {

using DARegAccessorVariant = std::variant<
    ChimeraTK::ScalarRegisterAccessor<uint32_t>,
    ChimeraTK::ScalarRegisterAccessor<int32_t>,
    ChimeraTK::ScalarRegisterAccessor<float> >;

class RegisterAccessor {
public:
    RegisterAccessor(const std::string& registerName, const std::string& readPV, const std::string& writePV) : registerName(registerName), readPV(readPV), writePV(writePV) {};

    ~RegisterAccessor();

    std::optional<DARegAccessorVariant> daRegAccVariant;
    std::string registerName, readPV, writePV;
};

}  // namespace ndsda

#endif