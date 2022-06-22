# gtest_mpi

A header-only c++ library to support EXPECTS_* macros in [Google Test](https://github.com/google/googletest) in MPI environment.


## Usage

Just change all your `gtest/gtest.h` header file to `gtest_mpi.h`, and call `GTestMPI::Init` after initialize MPI.

```c++
#include "gtest_mpi.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);                   // Initialize MPI.
    GTestMPI::Init();                         // Initialize gtest_mpi.
    ::testing::InitGoogleTest(&argc, argv);   // Initialize Google Test.
    auto result = RUN_ALL_TESTS();            // Run tests.
    MPI_Finalize();                           // Finalize MPI.
    return result;
}
```

User should make sure that all processes in MPI **always** call the **same** EXPECT_* macro in Google Test **at the same time**.

```c++
TEST(BinaryTest, Test1) {
    if (GTestMPI::Rank() == 2) {
        double x = 1;
        double y = 2;
        EXPECT_EQ(x, y);       // Correct!
    } else {
        int a = 1;
        int b = 1;
        EXPECT_EQ(a, b);       // Correct!
    }
}

TEST(BinaryTest, Test2) {
    if (GTestMPI::Rank() == 2) {
        double x = 1;
        double y = 2;
        EXPECT_EQ(x, y);       // Wrong! All processes should call EXPECT_* macro at the same time.
    }
}

TEST(BinaryTest, Test3) {
    if (GTestMPI::Rank() == 2) {
        double x = 1;
        double y = 2;
        EXPECT_EQ(x, y);
    } else {
        int a = 1;
        int b = 1;
        EXPECT_GE(a, b);       // Wrong! All processes should call the same EXPECT_* macro.
    }
}
```

When running the correct code, the output will show the process number.

```
[ RUN      ] BinaryTest.Test1
~/gtest_mpi/example.cpp:25: Failure
Expected equality of these values:
  x, on process 2
    Which is: 1
  y, on process 2
    Which is: 2
[  FAILED  ] BinaryTest.Test1 (0 ms)
```

## Run Example

```shell
mkdir build
cd build
cmake .. && make
cd ..
./run.sh proc_num
```

## Supported

Now, only part of EXPECT_* macros are supported.
`EXPECT_NEAR`, `EXPECT_PRED*`, [Expection Assertions](http://google.github.io/googletest/reference/assertions.html#exceptions) and [Death Assertions](http://google.github.io/googletest/reference/assertions.html#death) are not supported yet. Coming soon!
