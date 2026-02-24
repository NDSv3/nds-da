cmake_minimum_required(VERSION 3.23)

project(NDS-DeviceAccess)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_policy(SET CMP0167 OLD)

find_program(CPPCHECK_PATH cppcheck)
if(CPPCHECK_PATH)
    set(CMAKE_CXX_CPPCHECK
        ${CPPCHECK_PATH}
        --enable=all
        --inline-suppr
        --std=c++20
        --template=gcc
        --check-level=exhaustive
        --suppress=missingIncludeSystem
        --suppress=missingInclude
        --suppress=unusedFunction
        --suppress=unmatchedSuppression
        --suppress=checkersReport
        -I/usr/local/include
        --suppress=*:*usr/local/*
        --suppress=*:*_deps/*
        --quiet
    )
endif()

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

# ------------------------------ Package Configuration ------------------------------
install(
    TARGETS nds-da
    LIBRARY DESTINATION lib64
    ARCHIVE DESTINATION lib64
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/main/c++/include/
        DESTINATION include)

set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "nds-deviceaccess")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_MANTAINER "David Andrino <david.andrino@upm.es>")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_DESCRIPTION "Compatibility layer for NDS3-based devices to be used with ChimeraTK-DeviceAccess.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NDS3 - Deviceaccess compatibility layer")
set(CPACK_RPM_PACKAGE_LICENSE "ITER")
set(CPACK_PACKAGE_VENDOR "I2A2")
include(CPack)

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

find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    set(VALGRIND_SUPPRESSIONS_FILE "${CMAKE_BINARY_DIR}/valgrind.supp")
    set(VALGRIND_SUPPRESSIONS_SCRIPT "${CMAKE_BINARY_DIR}/write_valgrind_supp.cmake")
    file(WRITE ${VALGRIND_SUPPRESSIONS_SCRIPT}
    "file(WRITE \"${VALGRIND_SUPPRESSIONS_FILE}\" [=[
    {
        deps-googletest-cond
        Memcheck:Cond
        ...
        obj:*/_deps/*
        ...
    }

    {
        deps-googletest-leaks
        Memcheck:Leak
        ...
        obj:*/_deps/*
        ...
    }
    ]=])\n")
    add_custom_target(valgrind-test
          COMMAND ${CMAKE_COMMAND} -P ${VALGRIND_SUPPRESSIONS_SCRIPT}
        COMMAND ${VALGRIND_EXECUTABLE}
            --leak-check=full
            --show-leak-kinds=all
            --track-origins=yes
            --error-exitcode=1
            --suppressions=${VALGRIND_SUPPRESSIONS_FILE}
            --xml=yes
            --xml-file=${CMAKE_BINARY_DIR}/valgrind.xml
            $<TARGET_FILE:nds-da-test>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
        USES_TERMINAL
    )
else()
    message(WARNING "valgrind not found: custom target 'valgrind-test' will not be available")
endif()
