cmake_minimum_required(VERSION 3.23)

project(NDS-DeviceAccess)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------ NDS-Core Library ------------------------------
add_library(nds3 SHARED
    "m-nds-core/src/main/c++/nds/base.cpp"
    "m-nds-core/src/main/c++/nds/baseImpl.cpp"
    "m-nds-core/src/main/c++/nds/dataAcquisition.cpp"
    "m-nds-core/src/main/c++/nds/dataAcquisitionImpl.cpp"
    "m-nds-core/src/main/c++/nds/dataMultiplexing.cpp"
    "m-nds-core/src/main/c++/nds/dataMultiplexingImpl.cpp"
    "m-nds-core/src/main/c++/nds/dataScheduling.cpp"
    "m-nds-core/src/main/c++/nds/dataSchedulingImpl.cpp"
    "m-nds-core/src/main/c++/nds/digitalIO.cpp"
    "m-nds-core/src/main/c++/nds/digitalIOImpl.cpp"
    "m-nds-core/src/main/c++/nds/exceptions.cpp"
    "m-nds-core/src/main/c++/nds/factory.cpp"
    "m-nds-core/src/main/c++/nds/factoryBaseImpl.cpp"
    "m-nds-core/src/main/c++/nds/firmware.cpp"
    "m-nds-core/src/main/c++/nds/firmwareImpl.cpp"
    "m-nds-core/src/main/c++/nds/FTE.cpp"
    "m-nds-core/src/main/c++/nds/FTEImpl.cpp"
    "m-nds-core/src/main/c++/nds/HQMonitor.cpp"
    "m-nds-core/src/main/c++/nds/HQMonitorImpl.cpp"
    "m-nds-core/src/main/c++/nds/iniFileParser.cpp"
    "m-nds-core/src/main/c++/nds/iniFileParserImpl.cpp"
    "m-nds-core/src/main/c++/nds/interfaceBaseImpl.cpp"
    "m-nds-core/src/main/c++/nds/logStreamGetterImpl.cpp"
    "m-nds-core/src/main/c++/nds/Makefile"
    "m-nds-core/src/main/c++/nds/ndsFactoryImpl.cpp"
    "m-nds-core/src/main/c++/nds/node.cpp"
    "m-nds-core/src/main/c++/nds/nodeImpl.cpp"
    "m-nds-core/src/main/c++/nds/port.cpp"
    "m-nds-core/src/main/c++/nds/portImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvAction.cpp"
    "m-nds-core/src/main/c++/nds/pvActionImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvBase.cpp"
    "m-nds-core/src/main/c++/nds/pvBaseImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvBaseIn.cpp"
    "m-nds-core/src/main/c++/nds/pvBaseInImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvBaseOut.cpp"
    "m-nds-core/src/main/c++/nds/pvBaseOutImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvDelegateIn.cpp"
    "m-nds-core/src/main/c++/nds/pvDelegateInImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvDelegateOut.cpp"
    "m-nds-core/src/main/c++/nds/pvDelegateOutImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvVariableIn.cpp"
    "m-nds-core/src/main/c++/nds/pvVariableInImpl.cpp"
    "m-nds-core/src/main/c++/nds/pvVariableOut.cpp"
    "m-nds-core/src/main/c++/nds/pvVariableOutImpl.cpp"
    "m-nds-core/src/main/c++/nds/routing.cpp"
    "m-nds-core/src/main/c++/nds/routingImpl.cpp"
    "m-nds-core/src/main/c++/nds/stateMachine.cpp"
    "m-nds-core/src/main/c++/nds/stateMachineImpl.cpp"
    "m-nds-core/src/main/c++/nds/thread.cpp"
    "m-nds-core/src/main/c++/nds/threadBaseImpl.cpp"
    "m-nds-core/src/main/c++/nds/threadStd.cpp"
    "m-nds-core/src/main/c++/nds/timestamping.cpp"
    "m-nds-core/src/main/c++/nds/timestampingImpl.cpp"
    "m-nds-core/src/main/c++/nds/timing.cpp"
    "m-nds-core/src/main/c++/nds/timingImpl.cpp"
    "m-nds-core/src/main/c++/nds/triggerAndClk.cpp"
    "m-nds-core/src/main/c++/nds/triggerAndClkImpl.cpp"
    "m-nds-core/src/main/c++/nds/waveformGeneration.cpp"
    "m-nds-core/src/main/c++/nds/waveformGenerationImpl.cpp"
)
target_include_directories(nds3 PUBLIC
    "m-nds-core/src/main/c++/include"
)

add_library(nds3-testcontrolsystem SHARED
    "m-nds-core/src/test/c++/testControlSystem/ndsTestFactory.cpp"
    "m-nds-core/src/test/c++/testControlSystem/ndsTestInterface.cpp"
    "m-nds-core/src/test/c++/testControlSystem/ndsTestUtils.cpp"
)
target_include_directories(nds3-testcontrolsystem PUBLIC
    "m-nds-core/src/test/c++/include/"
)
target_include_directories(nds3-testcontrolsystem PRIVATE
    "m-nds-core/src/main/c++/include"
)

# ------------------------------ NDS-DeviceAccess ------------------------------
add_library(nds-da SHARED
    "src/main/c++/nds-da/deviceAccessMain.cpp"
)
target_include_directories(nds-da PRIVATE
    "src/main/c++/include"
)
target_link_libraries(nds-da PRIVATE nds3)
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
    "src/test/c++/nds-da-test/nds-da-test.cpp"
)
target_compile_options(nds-da-test PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(nds-da-test
    nds-da
    nds3
    nds3-testcontrolsystem
    GTest::gtest_main
)
target_include_directories(nds-da-test PRIVATE
    "src/test/c++/include"
)

include(GoogleTest)
gtest_discover_tests(nds-da-test)