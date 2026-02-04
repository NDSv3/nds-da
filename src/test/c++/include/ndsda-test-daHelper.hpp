#ifndef NDSDA_TEST_DAHELPER_H
#define NDSDA_TEST_DAHELPER_H

#include <ChimeraTK/Device.h>
#include <ChimeraTK/OneDRegisterAccessor.h>
#include <ChimeraTK/Utilities.h>
#include "ndsda-test-constants.hpp"

class DATestHelper {
public:
    static DATestHelper& getInstance() {
        static DATestHelper instance;
        return instance;
    }

    template <typename T>
    T readRegister(const std::string& registerName) {
        return device.read<T>(registerName);
    }

    template <typename T>
    void writeRegister(const std::string& registerName, T&& value) {
        device.write(registerName, std::forward<T>(value));
    }

    template <typename T>
    std::array<T, NDSDA_TEST_ONEDELEMENTS> readOneD(const std::string& oneDName) {
        std::array<T, NDSDA_TEST_ONEDELEMENTS> values;

        ChimeraTK::OneDRegisterAccessor<T> acc = device.getOneDRegisterAccessor<T>(oneDName);
        acc.read();
        std::copy(acc.begin(), acc.end(), values.begin());

        return values;
    }

    template <typename T>
    void writeOneD(const std::string& oneDName, const std::array<T, NDSDA_TEST_ONEDELEMENTS>& values) {
        ChimeraTK::OneDRegisterAccessor<T> acc = device.getOneDRegisterAccessor<T>(oneDName);
        std::copy(values.begin(), values.end(), acc.begin());
        acc.write();
    }

    void operator=(const DATestHelper&) = delete;

private:
    ChimeraTK::Device device;

    DATestHelper() {
        ChimeraTK::setDMapFilePath(NDSDA_TEST_DMAPFILE);
        device.open(NDSDA_TEST_DEVICEALIAS);
    };

    ~DATestHelper() {
        device.close();
    };
};

#endif  //! NDSDA_TEST_DAHELPER_H
