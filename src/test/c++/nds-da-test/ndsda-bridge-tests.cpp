
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

TEST(DABridge, ConstructorNullptrError) {
    EXPECT_THROW(
        ndsda::DABridge bridge(nullptr),
        ndsda::ndsdaError
    );
}
