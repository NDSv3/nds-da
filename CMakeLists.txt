cmake_minimum_required(VERSION 3.23)

project(NDS-DeviceAccess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0167 OLD)

# ------------------------------- Device Access --------------------------------
find_package(ChimeraTK-DeviceAccess REQUIRED)

# ------------------------------ NDS-DeviceAccess ------------------------------
# Add every file under src/main/c++/nds-da
add_library(nds-da SHARED
    "src/main/c++/nds-da/ndsdaDevice.cpp"
    "src/main/c++/nds-da/ndsdaFileParser.cpp"
    "src/main/c++/nds-da/ndsdaBridge.cpp"
    "src/main/c++/nds-da/ndsdaAccessor.cpp"
)
target_include_directories(nds-da PUBLIC
    "src/main/c++/include"
)
target_link_libraries(nds-da PRIVATE nds3 ChimeraTK-DeviceAccess atomic)
target_compile_options(nds-da PRIVATE -Wall -Wextra -Wpedantic)
set(CMAKE_CXX_CLANG_TIDY clang-tidy)

# ------------------------------ Tests ------------------------------
cmake_policy(SET CMP0135 NEW)
include(FetchContent)
FetchContent_Declare( googletest
  URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(nds-da-test
    "src/test/c++/nds-da-test/ndada-device-tests.cpp"
    "src/test/c++/nds-da-test/nds-da-test.cpp"
    "src/test/c++/nds-da-test/ndsda-file-parser-tests.cpp"
    "src/test/c++/nds-da-test/ndsda-bridge-tests.cpp"
    "src/test/c++/nds-da-test/ndsda-accessor-tests.cpp"
)
target_compile_options(nds-da-test PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(nds-da-test PRIVATE
    ChimeraTK-DeviceAccess
    nds-da
    GTest::gtest_main
    atomic
)
target_include_directories(nds-da-test PRIVATE
    "src/test/c++/include"
)
set_target_properties(nds-da-test PROPERTIES
    BUILD_RPATH "/usr/local/lib64/"
)

include(GoogleTest)
gtest_discover_tests(nds-da-test)

# ------------------------------ NDS-Core Library ------------------------------
find_library(nds3 
    NAMES nds3 
    PATHS /usr/local/lib64 
    REQUIRED
)
find_library(nds3-testcontrolsystem 
    NAMES nds3-testcontrolsystem 
    PATHS /usr/local/lib64 
    REQUIRED
)

target_link_libraries(nds-da PUBLIC nds3)
target_include_directories(nds-da PRIVATE /usr/local/include)

target_link_libraries(nds-da-test PRIVATE nds3-testcontrolsystem)
target_include_directories(nds-da-test PRIVATE /usr/local/include)