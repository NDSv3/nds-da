#ifndef NDSDA_TEST_CONSTANTS_H
#define NDSDA_TEST_CONSTANTS_H

#include <memory>
#include <string>

#include "ndsdaCommon.hpp"

using PTE = ndsda::PVTypeEnum;
using ndsda::OneDElement, ndsda::RegisterElement, ndsda::MemoryElement, ndsda::NodeMapPV;

const std::string NDSDA_TEST_DEVICEALIAS = "test_device";
const std::string NDSDA_TEST_DMAPFILE    = "../src/test/files/test-devices.dmap";
const std::string NDSDA_TEST_MMAPFILE    = "../src/test/files/test-memory.mmap";
const std::string NDSDA_TEST_NODEMAPFILE = "../src/test/files/nodeMap.xml";
const std::string NDSDA_TEST_INVALIDXML  = "../src/test/files/invalidNodeMap.xml";

const size_t NDSDA_TEST_ONEDELEMENTS = 10;

// clang-format off
// WARNING: MUST match the content of MEMORY_MAPFILE with DEVICE_ALIAS
static const auto expectedMemoryElements = std::to_array<std::shared_ptr<MemoryElement>>({
    std::make_shared<OneDElement>(".BASE", 157, 32),
    std::make_shared<RegisterElement>("BASE.INTEGER32", 32, 0, true),
    std::make_shared<RegisterElement>("BASE.UINTEGER32", 32, 0, false),
    std::make_shared<RegisterElement>("BASE.ONEBIT", 1, 0, false),
    std::make_shared<RegisterElement>("BASE.FLOATBITCAST", 32, ndsda::NDSDA_MEMORYELEMENT_IEEE754_FRACBITS, true),
    std::make_shared<RegisterElement>("BASE.FLOATFIXED8", 32, 8, true),
    std::make_shared<RegisterElement>("BASE.FLOATFIXED16", 32, 16, true),
    std::make_shared<RegisterElement>("BASE.CUSTOMGETSET", 32, 0, false),

    std::make_shared<OneDElement>("BASE.VECBOOL", NDSDA_TEST_ONEDELEMENTS, 1),
    std::make_shared<OneDElement>("BASE.VECINT8", NDSDA_TEST_ONEDELEMENTS, 8, 0, true),
    std::make_shared<OneDElement>("BASE.VECINT16", NDSDA_TEST_ONEDELEMENTS, 16, 0, true),
    std::make_shared<OneDElement>("BASE.VECINT32", NDSDA_TEST_ONEDELEMENTS, 32, 0, true),
    std::make_shared<OneDElement>("BASE.VECINT64", 2*NDSDA_TEST_ONEDELEMENTS, 32, 0, true),
    std::make_shared<OneDElement>("BASE.VECUINT8", NDSDA_TEST_ONEDELEMENTS, 8),
    std::make_shared<OneDElement>("BASE.VECUINT16", NDSDA_TEST_ONEDELEMENTS, 16),
    std::make_shared<OneDElement>("BASE.VECUINT32", NDSDA_TEST_ONEDELEMENTS, 32),
    std::make_shared<OneDElement>("BASE.VECFLOATFIXED8", NDSDA_TEST_ONEDELEMENTS, 32, 8, true),
    std::make_shared<OneDElement>("BASE.VECFLOATBITCAST", NDSDA_TEST_ONEDELEMENTS, 32, ndsda::NDSDA_MEMORYELEMENT_IEEE754_FRACBITS, true),
    std::make_shared<OneDElement>("BASE.VECDOUBLE", 2*NDSDA_TEST_ONEDELEMENTS, 32),
    std::make_shared<OneDElement>("BASE.STRING", NDSDA_TEST_ONEDELEMENTS, 8),

    std::make_shared<OneDElement>("HUNDREDI32VEC", 100, 32, 0, true),
});

#define CREATE_NODEMAP_PV(name, reg, typeEnum, nElem) \
    std::make_shared<NodeMapPV>(name, reg, typeEnum, std::string(name) + "-Get", std::string(name) + "-Set", nElem, nullptr)

struct expectedPVIdx {
    enum {
        INTEGER32 = 0, UINTEGER32, ONEBIT, FLOATBITCAST, FLOATFIXED8, FLOATFIXED16, CUSTOMGETSET,
        VECBOOL, VECINT8, VECINT16, VECINT32, VECINT64, VECUINT8, VECUINT16, VECUINT32, VECFLOATFIXED8,
        VECFLOATBITCAST, VECDOUBLE, STRING, HUNDREDI32VEC
    };
};
struct expectedMemoryElementIdx {
    enum {
        INTEGER32 = 1, UINTEGER32, ONEBIT, FLOATBITCAST, FLOATFIXED8, FLOATFIXED16, CUSTOMGETSET,
        VECBOOL, VECINT8, VECINT16, VECINT32, VECINT64, VECUINT8, VECUINT16, VECUINT32, VECFLOATFIXED8,
        VECFLOATBITCAST, VECDOUBLE, STRING, HUNDREDI32VEC
    };
};

static const auto __expectedNodeMapPVs = std::to_array<std::shared_ptr<NodeMapPV>>({
    CREATE_NODEMAP_PV("INTEGER32",       "BASE.INTEGER32",      PTE::INT32,      1),
    CREATE_NODEMAP_PV("UINTEGER32",      "BASE.UINTEGER32",     PTE::INT32,      1),
    CREATE_NODEMAP_PV("ONEBIT",          "BASE.ONEBIT",         PTE::INT32,      1),
    CREATE_NODEMAP_PV("FLOATBITCAST",    "BASE.FLOATBITCAST",   PTE::FLOAT,      1),
    CREATE_NODEMAP_PV("FLOATFIXED8",     "BASE.FLOATFIXED8",    PTE::FLOAT,      1),
    CREATE_NODEMAP_PV("FLOATFIXED16",    "BASE.FLOATFIXED16",   PTE::FLOAT,      1),
    CREATE_NODEMAP_PV("CUSTOMGETSET",    "BASE.CUSTOMGETSET",   PTE::INT32,      1),
    CREATE_NODEMAP_PV("VECBOOL",         "BASE.VECBOOL",        PTE::VEC_BOOL,   NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECINT8",         "BASE.VECINT8",        PTE::VEC_INT8,   NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECINT16",        "BASE.VECINT16",       PTE::VEC_INT16,  NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECINT32",        "BASE.VECINT32",       PTE::VEC_INT32,  NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECINT64",        "BASE.VECINT64",       PTE::VEC_INT64,  NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECUINT8",        "BASE.VECUINT8",       PTE::VEC_UINT8,  NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECUINT16",       "BASE.VECUINT16",      PTE::VEC_UINT16, NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECUINT32",       "BASE.VECUINT32",      PTE::VEC_UINT32, NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECFLOATFIXED8",  "BASE.VECFLOATFIXED8", PTE::VEC_FLOAT,  NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECFLOATBITCAST", "BASE.VECFLOATBITCAST", PTE::VEC_FLOAT,  NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("VECDOUBLE",       "BASE.VECDOUBLE",      PTE::VEC_DOUBLE, NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("STRING",          "BASE.STRING",         PTE::STRING, NDSDA_TEST_ONEDELEMENTS),
    CREATE_NODEMAP_PV("HUNDREDI32VEC",   ".HUNDREDI32VEC",      PTE::VEC_INT32,  100),
});
// clang-format on

static const auto expectedNodeMapPVs = []() {
    static_assert(expectedMemoryElements.size() == __expectedNodeMapPVs.size() + 1, "Mismatch between expected memory elements and node map PVs");
    std::remove_const_t<decltype(__expectedNodeMapPVs)> vec;
    for (size_t i = 0; i < __expectedNodeMapPVs.size(); ++i) {
        std::unique_ptr<MemoryElement> me;
        switch (expectedMemoryElements[i + 1]->getType()) {
            case ndsda::MemoryElementType::ONE_D:
                me = std::make_unique<OneDElement>(static_cast<OneDElement&>(*expectedMemoryElements[i + 1]));
                break;
            case ndsda::MemoryElementType::SCALAR:
                me = std::make_unique<RegisterElement>(static_cast<RegisterElement&>(*expectedMemoryElements[i + 1]));
                break;
            default:
                throw std::runtime_error("Unsupported MemoryElementType in test setup");
        }
        vec[i] = std::make_shared<NodeMapPV>(
            __expectedNodeMapPVs[i]->name, __expectedNodeMapPVs[i]->reg,
            __expectedNodeMapPVs[i]->type, __expectedNodeMapPVs[i]->readPV,
            __expectedNodeMapPVs[i]->writePV, __expectedNodeMapPVs[i]->nElem,
            std::move(me)
        );
    }
    vec[expectedPVIdx::CUSTOMGETSET]->writePV = "CustomGetSet-Setter";
    vec[expectedPVIdx::CUSTOMGETSET]->readPV  = "CustomGetSet-Getter";
    return vec;
}();

#undef CREATE_NODEMAP_PV

#endif  //! NDSDA_TEST_CONSTANTS_H