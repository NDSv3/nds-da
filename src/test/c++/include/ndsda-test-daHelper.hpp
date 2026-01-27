#ifndef NDSDA_TEST_DAHELPER_H
#define NDSDA_TEST_DAHELPER_H

#include <ChimeraTK/Device.h>
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
