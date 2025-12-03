#include <gtest/gtest.h>
#include <nds3/nds.h>
#include <nds3/ndsTestFactory.h>

TEST(NdsDaTest, SampleTest) { EXPECT_EQ(1 + 1, 2); }

TEST(NdsDaTest, AnotherSampleTest) { EXPECT_TRUE(true); }

TEST(NdsDaTest, FailTest) { EXPECT_TRUE(false); }

int main(int argc, char **argv) {
    nds::Factory factory(std::shared_ptr<nds::FactoryBaseImpl>(new nds::tests::TestControlSystemFactoryImpl()));
    // factory.createDevice("NDS-DeviceAccess", "deviceAccessTestDevice", nds::namedParameters_t());

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}