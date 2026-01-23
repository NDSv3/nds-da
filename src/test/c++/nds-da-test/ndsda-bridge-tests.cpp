
#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include "nds-da-test-fixture.hpp"

#include "ndsdaBridge.hpp"
#include "ndsdaCommon.hpp"

TEST(DABridge, ConstructorWithInvalidDmapFilePath) {
    EXPECT_THROW(
        ndsda::DABridge bridge("invalid/path.dmap", NDSDA_TEST_DEVICEALIAS),
        nds::NdsError
    );
}

TEST(DABridge, ConstructorWithInvalidDeviceName) {
    EXPECT_THROW(
        ndsda::DABridge bridge(NDSDA_TEST_DMAPFILE, "NONEXISTENT_DEVICE"),
        nds::NdsError
    );
}
