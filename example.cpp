#include "gtest_mpi.h"

TEST(UnaryTest, Test0) { EXPECT_TRUE(true); }

TEST(UnaryTest, Test1) { EXPECT_FALSE(false); }

TEST(UnaryTest, Test2) {
    if (GTestMPI::Rank() == 3) {
        EXPECT_TRUE(1 == 2);
    } else {
        EXPECT_TRUE(true);
    }
}

TEST(BinaryTest, Test0) { EXPECT_EQ(1, 1); }

TEST(BinaryTest, Test1) {
    if (GTestMPI::Rank() == 2) {
        int x = 1;
        int y = 2;
        EXPECT_EQ(x, y);
    } else {
        int a = 1;
        int b = 1;
        EXPECT_EQ(a, b);
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    GTestMPI::Init();
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    MPI_Finalize();
    return result;
}