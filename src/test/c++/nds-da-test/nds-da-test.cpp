#include <gtest/gtest.h>

TEST(NdsDaTest, SampleTest) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(NdsDaTest, AnotherSampleTest) {
    EXPECT_TRUE(true);
}

TEST(NdsDaTest, FailTest) {
    EXPECT_TRUE(false);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}