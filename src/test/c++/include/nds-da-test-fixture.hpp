#ifndef NDS_DA_TEST_FIXTURE
#define NDS_DA_TEST_FIXTURE

#include <gtest/gtest.h>
#include <string>
#include "nds3/factory.h"
#include "nds3/ndsTestInterface.h"
#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"

using PTE = ndsda::PVTypeEnum;

const std::string NDSDA_TEST_DEVICEALIAS = "test_device";
const std::string NDSDA_TEST_DMAPFILE    = "../src/test/files/test-devices.dmap";
const std::string NDSDA_TEST_MMAPFILE    = "../src/test/files/test-memory.mmap";
const std::string NDSDA_TEST_NODEMAPFILE = "../src/test/files/nodeMap.xml";
const std::string NDSDA_TEST_INVALIDXML  = "../src/test/files/invalidNodeMap.xml";
const std::string NDSDA_TEST_REGISTERPV  = "PV1";
const std::string NDSDA_TEST_SIGNEDREGPV = "PV2";
const std::string NDSDA_TEST_ONEDPV      = "PV4";
const size_t NDSDA_TEST_ONEDELEMENTS     = 200;

// WARNING: MUST match the content of MEMORY_MAPFILE with DEVICE_ALIAS
static const std::vector<std::shared_ptr<ndsda::MemoryElement>> expectedMemoryElements = {
    std::make_shared<ndsda::OneDElement>(".BASE", 4, 32, 0, 0),
    std::make_shared<ndsda::RegisterElement>("BASE.FIRST_REG", 0, 0),
    std::make_shared<ndsda::RegisterElement>("BASE.SECOND_REG", 0, 1),
    std::make_shared<ndsda::RegisterElement>("BASE.THIRD_REG", 0, 0),
    std::make_shared<ndsda::RegisterElement>("BASE.ONE_BIT_REG", 0, 0),
    std::make_shared<ndsda::OneDElement>(".DMA_SAMPLES", 200, 32, 0, 0)
};

static const std::vector<std::shared_ptr<ndsda::NodeMapPV>> expectedNodeMapPVs = {
    std::make_shared<ndsda::NodeMapPV>(
        "PV1", "BASE.FIRST_REG", PTE::INT32, false, "PV1-Get", "PV1-Set", 1,
        std::make_unique<ndsda::RegisterElement>("BASE.FIRST_REG", 0, 0)
    ),
    std::make_shared<ndsda::NodeMapPV>(
        "PV2", "BASE.SECOND_REG", PTE::FLOAT, true, "PV2-Get", "PV2-Set", 1,
        std::make_unique<ndsda::RegisterElement>("BASE.SECOND_REG", 0, 1)
    ),
    std::make_shared<ndsda::NodeMapPV>(
        "PV3", "BASE.THIRD_REG", PTE::FLOAT, false, "PV3-Getter", "PV3-Setter", 1,
        std::make_unique<ndsda::RegisterElement>("BASE.THIRD_REG", 0, 0)
    ),
    std::make_shared<ndsda::NodeMapPV>(
        "PV4", ".DMA_SAMPLES", PTE::VEC_UINT32, false, "PV4-Get", "PV4-Set", 200,
        std::make_unique<ndsda::OneDElement>(".DMA_SAMPLES", 200, 32, 0, 0)
    )
};

class ndsDABasicDeviceFixture : public testing::Test {
protected:
    nds::Factory factory;
    nds::namedParameters_t params;
    nds::tests::TestControlSystemInterfaceImpl* iface;

    ndsDABasicDeviceFixture() : factory("test") {
        params.emplace("DeviceAlias", NDSDA_TEST_DEVICEALIAS);
        params.emplace("DeviceMapFile", NDSDA_TEST_DMAPFILE);
        params.emplace("NodeMapFile", NDSDA_TEST_NODEMAPFILE);
    }
    // ~ndsDABasicDeviceFixture() override {}

    void SetUp() override {
        factory.createDevice("DADevice", "deviceAccessDevice", params);
        iface = nds::tests::TestControlSystemInterfaceImpl::getInstance("deviceAccessDevice");
        ASSERT_NE(iface, nullptr);
    }
    void TearDown() override {
        factory.destroyDevice("deviceAccessDevice");
    }
};

/*
class ndsDAFileParserFixture : public testing::Test {
protected:
    const std::string DEVICE_ALIAS   = NDSDA_TEST_DEVICEALIAS;
    const std::string DEVICE_MAPFILE = NDSDA_TEST_DMAPFILE;
    const std::string MEMORY_MAPFILE = NDSDA_TEST_MMAPFILE;

    std::unique_ptr<ndsda::FileParser> parser;

    void SetUp() override {
        parser = std::make_unique<ndsda::FileParser>(DEVICE_MAPFILE, DEVICE_ALIAS);
    }

    void TearDown() override {}
};
*/

class ndsDABridgeFixture : public testing::Test {
protected:
    std::unique_ptr<ndsda::DABridge> bridge;

    void SetUp() override {
        bridge = std::make_unique<ndsda::DABridge>(NDSDA_TEST_DMAPFILE, NDSDA_TEST_DEVICEALIAS, expectedNodeMapPVs);
    }

    void TearDown() override {}
};
#endif  //! NDS_DA_TEST_FIXTURE
