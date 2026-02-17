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

option(ENABLE_COVERAGE "Enable GCC/Clang coverage instrumentation" OFF)

if(ENABLE_COVERAGE)
    add_library(coverage_flags INTERFACE)
    target_compile_options(coverage_flags INTERFACE -O0 -g --coverage)
    target_link_options(coverage_flags INTERFACE --coverage)

    target_link_libraries(nds-da PRIVATE coverage_flags)
    target_link_libraries(nds-da-test PRIVATE coverage_flags)

    set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-prog-name=gcov
        OUTPUT_VARIABLE GCOV_EXECUTABLE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(NOT GCOV_EXECUTABLE)
        find_program(GCOV_EXECUTABLE gcov)
    endif()

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
        COMMAND ${LCOV_EXECUTABLE} --gcov-tool ${GCOV_EXECUTABLE} --directory . --zerocounters
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${LCOV_EXECUTABLE} --gcov-tool ${GCOV_EXECUTABLE} --capture --directory . --ignore-errors mismatch --output-file ${COVERAGE_DIR}/coverage.info
        COMMAND ${LCOV_EXECUTABLE} --remove ${COVERAGE_DIR}/coverage.info
            "${CMAKE_BINARY_DIR}/*" "${CMAKE_BINARY_DIR}/_deps/*" "/usr/*"
            --ignore-errors unused --output-file ${COVERAGE_DIR}/coverage.filtered.info
        COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_DIR}/coverage.filtered.info --output-directory ${COVERAGE_DIR}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
    )
endif()