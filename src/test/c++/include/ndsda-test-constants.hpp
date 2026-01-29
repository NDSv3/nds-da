#ifndef NDSDA_TEST_CONSTANTS_H
#define NDSDA_TEST_CONSTANTS_H

#include <memory>
#include <string>
#include <vector>

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
        "PV1", "BASE.FIRST_REG", PTE::INT32, "PV1-Get", "PV1-Set", 1,
        std::make_unique<ndsda::RegisterElement>("BASE.FIRST_REG", 2, false)
    ),
    std::make_shared<ndsda::NodeMapPV>(
        "PV2", "BASE.SECOND_REG", PTE::FLOAT, "PV2-Get", "PV2-Set", 1,
        std::make_unique<ndsda::RegisterElement>("BASE.SECOND_REG", 0, true)
    ),
    std::make_shared<ndsda::NodeMapPV>(
        "PV3", "BASE.THIRD_REG", PTE::FLOAT, "PV3-Getter", "PV3-Setter", 1,
        std::make_unique<ndsda::RegisterElement>("BASE.THIRD_REG", 0, false)
    ),
    std::make_shared<ndsda::NodeMapPV>(
        "PV4", ".DMA_SAMPLES", PTE::VEC_UINT32, "PV4-Get", "PV4-Set", 200,
        std::make_unique<ndsda::OneDElement>(".DMA_SAMPLES", 200, 32, false, 0)
    )
};

#endif  //! NDSDA_TEST_CONSTANTS_H