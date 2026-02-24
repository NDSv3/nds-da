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
    if(NOT GCOVR_EXECUTABLE)
        find_program(GCOVR_EXECUTABLE gcovr)
    endif()

    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR} 
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}/details
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}/nested
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        COMMAND ${GENHTML_EXECUTABLE} ${COVERAGE_DIR}/coverage.filtered.info --output-directory ${COVERAGE_DIR}
        COMMAND ${GCOVR_EXECUTABLE} -r ${CMAKE_SOURCE_DIR} --html-details ${COVERAGE_DIR}/details/coverage.html
        COMMAND ${GCOVR_EXECUTABLE} -r ${CMAKE_SOURCE_DIR} --html-nested ${COVERAGE_DIR}/nested/coverage.html
        COMMAND ${GCOVR_EXECUTABLE} -r ${CMAKE_SOURCE_DIR} -x ${COVERAGE_DIR}/coverage.xml
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
    )
endif()

find_program(VALGRIND_EXECUTABLE valgrind)
find_program(CG_ANNOTATE_EXECUTABLE cg_annotate)
find_program(CALLGRIND_ANNOTATE_EXECUTABLE callgrind_annotate)
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
            --log-file=${CMAKE_BINARY_DIR}/valgrind.log
            $<TARGET_FILE:nds-da-test>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
        USES_TERMINAL
    )
    # Cachegrind: CPU cache profiling
    add_custom_target(valgrind-cachegrind
        COMMAND ${CMAKE_COMMAND} -P ${VALGRIND_SUPPRESSIONS_SCRIPT}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/cachegrind.out.*
        COMMAND ${VALGRIND_EXECUTABLE}
            --tool=cachegrind
            --cachegrind-out-file=${CMAKE_BINARY_DIR}/cachegrind.out.%p
            --suppressions=${VALGRIND_SUPPRESSIONS_FILE}
            $<TARGET_FILE:nds-da-test>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
        USES_TERMINAL
    )

    # Callgrind: call-graph profiling (useful for generating flamegraphs/Callgrind analysis)
    add_custom_target(valgrind-callgrind
        COMMAND ${CMAKE_COMMAND} -P ${VALGRIND_SUPPRESSIONS_SCRIPT}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/callgrind.out.*
        COMMAND ${VALGRIND_EXECUTABLE}
            --tool=callgrind
            --collect-jumps=yes
            --callgrind-out-file=${CMAKE_BINARY_DIR}/callgrind.out.%p
            --suppressions=${VALGRIND_SUPPRESSIONS_FILE}
            $<TARGET_FILE:nds-da-test>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
        USES_TERMINAL
    )

    # Combined target: run Memcheck (xml), then Cachegrind, then Callgrind sequentially
    add_custom_target(valgrind-all
        COMMAND ${CMAKE_COMMAND} -P ${VALGRIND_SUPPRESSIONS_SCRIPT}
        # Memcheck (will stop on errors because of --error-exitcode=1)
        COMMAND ${VALGRIND_EXECUTABLE}
            --leak-check=full
            --show-leak-kinds=all
            --track-origins=yes
            --error-exitcode=1
            --suppressions=${VALGRIND_SUPPRESSIONS_FILE}
            --xml=yes
            --xml-file=${CMAKE_BINARY_DIR}/valgrind.xml
            $<TARGET_FILE:nds-da-test>
        # Cachegrind
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/cachegrind.out.*
        COMMAND ${VALGRIND_EXECUTABLE}
            --tool=cachegrind
            --cachegrind-out-file=${CMAKE_BINARY_DIR}/cachegrind.out.%p
            --suppressions=${VALGRIND_SUPPRESSIONS_FILE}
            $<TARGET_FILE:nds-da-test>
        # Callgrind
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/callgrind.out.*
        COMMAND ${VALGRIND_EXECUTABLE}
            --tool=callgrind
            --collect-jumps=yes
            --callgrind-out-file=${CMAKE_BINARY_DIR}/callgrind.out.%p
            --suppressions=${VALGRIND_SUPPRESSIONS_FILE}
            $<TARGET_FILE:nds-da-test>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
        USES_TERMINAL
    )

    # Post-build annotations: if annotate tools are present, convert the most
    # recent cachegrind/callgrind outputs to text summaries.
    if(CG_ANNOTATE_EXECUTABLE)
        add_custom_command(TARGET valgrind-all POST_BUILD
            COMMAND /bin/sh -c "f=$(ls -1t created \"${CMAKE_BINARY_DIR}/cachegrind.out.*\" 2>/dev/null | head -n1); \"${CG_ANNOTATE_EXECUTABLE}\" \"$f\" \"${CMAKE_BINARY_DIR}/cachegrind.txt\""
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()

    if(CALLGRIND_ANNOTATE_EXECUTABLE)
        add_custom_command(TARGET valgrind-all POST_BUILD
            COMMAND /bin/sh -c "f=$(ls -1t created \"${CMAKE_BINARY_DIR}/callgrind.out.*\" 2>/dev/null | head -n1); \"${CALLGRIND_ANNOTATE_EXECUTABLE}\" \"$f\" \"${CMAKE_BINARY_DIR}/callgrind.txt\""
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    endif()
else()
    message(WARNING "valgrind not found: custom target 'valgrind-test' will not be available")
endif()

# ------------------------------ Performance Target ---------------------------
find_program(PERF_EXECUTABLE perf)
find_program(FLAMEGRAPH_CARGO_EXECUTABLE flamegraph)

if(PERF_EXECUTABLE AND FLAMEGRAPH_CARGO_EXECUTABLE)
    add_custom_target(perf-flame
        COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_BINARY_DIR}/perf.data ${CMAKE_BINARY_DIR}/flamegraph.svg
        COMMAND ${PERF_EXECUTABLE} record -F 200 -g --call-graph fp -o ${CMAKE_BINARY_DIR}/perf.data -- $<TARGET_FILE:nds-da-test>
        COMMAND ${FLAMEGRAPH_CARGO_EXECUTABLE} --perfdata ${CMAKE_BINARY_DIR}/perf.data -o ${CMAKE_BINARY_DIR}/flamegraph.svg --min-width 1 --title "NDS-DeviceAccess gTest Flamegraph "
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS nds-da-test
        USES_TERMINAL
    )
else()
    message(WARNING "'perf' or 'flamegraph' not found.")
endif()
