#ifndef GTEST_MPI_GTEST_MPI_H
#define GTEST_MPI_GTEST_MPI_H

#include <ccomplex>
#include <string>

#include "gtest/gtest.h"
#include "mpi.h"

#undef GTEST_TEST_BOOLEAN_
#define GTEST_TEST_BOOLEAN_(expression, text, actual, expected, fail)          \
    GTEST_AMBIGUOUS_ELSE_BLOCKER_ {                                            \
        char local_expression = static_cast<bool>(expression);                 \
        char reduced_expression;                                               \
        GTestMPI::Allreduce(reduced_expression, local_expression, MPI_BAND);   \
        std::string text_new(text), actual_new(#actual),                       \
            expected_new(#expected);                                           \
        if (!reduced_expression) {                                             \
            int death_rank = GTestMPI::SelectMinimalRankID(!local_expression); \
            if (death_rank != GTestMPI::RootRank) {                            \
                if (GTestMPI::Rank() == death_rank) {                          \
                    GTestMPI::Send(std::string(text), GTestMPI ::RootRank);    \
                    GTestMPI::Send(std::string(#actual), GTestMPI ::RootRank); \
                    GTestMPI::Send(std::string(#expected),                     \
                                   GTestMPI ::RootRank);                       \
                }                                                              \
                if (GTestMPI::Rank() == GTestMPI::RootRank) {                  \
                    GTestMPI::Recv(text_new, death_rank);                      \
                    GTestMPI::Recv(actual_new, death_rank);                    \
                    GTestMPI::Recv(expected_new, death_rank);                  \
                }                                                              \
            }                                                                  \
            text_new += ", on process " + std::to_string(death_rank);          \
        }                                                                      \
        if (const ::testing::AssertionResult gtest_ar_ =                       \
                ::testing::AssertionResult(reduced_expression))                \
            ;                                                                  \
        else                                                                   \
            fail(::testing::internal::GetBoolAssertionFailureMessage(          \
                     gtest_ar_, text_new.c_str(), actual_new.c_str(),          \
                     expected_new.c_str())                                     \
                     .c_str());                                                \
    }

#undef GTEST_PRED_FORMAT2_
#define GTEST_PRED_FORMAT2_(pred_format, v1, v2, on_failure)                   \
    {                                                                          \
        char local_expression =                                                \
            static_cast<bool>(pred_format(#v1, #v2, v1, v2));                  \
        char reduced_expression;                                               \
        GTestMPI::Allreduce(reduced_expression, local_expression, MPI_BAND);   \
        if (!reduced_expression) {                                             \
            int death_rank = GTestMPI::SelectMinimalRankID(!local_expression); \
            int v1_death = v1, v2_death = v2;                                  \
            std::string v1_name(#v1), v2_name(#v2);                            \
            if (death_rank != GTestMPI::RootRank) {                            \
                if (GTestMPI::Rank() == death_rank) {                          \
                    GTestMPI::Send(v1_death, GTestMPI ::RootRank);             \
                    GTestMPI::Send(v2_death, GTestMPI ::RootRank);             \
                    GTestMPI::Send(std::string(#v1), GTestMPI ::RootRank);     \
                    GTestMPI::Send(std::string(#v2), GTestMPI ::RootRank);     \
                }                                                              \
                if (GTestMPI::Rank() == GTestMPI::RootRank) {                  \
                    GTestMPI::Recv(v1_death, death_rank);                      \
                    GTestMPI::Recv(v2_death, death_rank);                      \
                    GTestMPI::Recv(v1_name, death_rank);                       \
                    GTestMPI::Recv(v2_name, death_rank);                       \
                }                                                              \
            }                                                                  \
            v1_name += ", on process " + std::to_string(death_rank);           \
            v2_name += ", on process " + std::to_string(death_rank);           \
            GTEST_ASSERT_(pred_format(v1_name.c_str(), v2_name.c_str(),        \
                                      v1_death, v2_death),                     \
                          on_failure);                                         \
        } else {                                                               \
            GTEST_ASSERT_(pred_format(#v1, #v2, v1, v2), on_failure);          \
        }                                                                      \
    }

#define RUN_ALL_TESTS()                                 \
    []() {                                              \
        int ret = RUN_ALL_TESTS();                      \
        int reduced_ret;                                \
        GTestMPI::Allreduce(reduced_ret, ret, MPI_MAX); \
        return reduced_ret;                             \
    }()

namespace GTestMPI {
const int RootRank = 0;

int Rank() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}

int Size() {
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    return size;
}

void Init() {
    ::testing::TestEventListeners &listeners =
        ::testing::UnitTest::GetInstance()->listeners();
    if (Rank() != RootRank) {
        delete listeners.Release(listeners.default_result_printer());
    }
}

template <class T>
MPI_Datatype MPIType() {
    if (std::is_same<T, float>::value) {
        return MPI_FLOAT;
    } else if (std::is_same<T, double>::value) {
        return MPI_DOUBLE;
    } else if (std::is_same<T, float __complex__>::value) {
        return MPI_C_COMPLEX;
    } else if (std::is_same<T, double __complex__>::value) {
        return MPI_C_DOUBLE_COMPLEX;
    } else if (std::is_same<T, int>::value) {
        return MPI_INT;
    } else if (std::is_same<T, long long>::value) {
        return MPI_LONG_LONG_INT;
    } else if (std::is_same<T, char>::value) {
        return MPI_CHAR;
    } else {
        return MPI_INT32_T;
    }
}

template <class T>
void Allreduce(T &reduced, T local, MPI_Op op) {
    MPI_Allreduce(&local, &reduced, 1, MPIType<T>(), op, MPI_COMM_WORLD);
}

template <class T>
void Send(T data, int rank) {
    MPI_Send(&data, 1, MPIType<T>(), rank, 0, MPI_COMM_WORLD);
}

template <>
void Send(const std::string data, int rank) {
    MPI_Send(data.c_str(), (int)data.size(), MPI_CHAR, rank, 0, MPI_COMM_WORLD);
}

template <class T>
void Recv(T &data, int rank) {
    MPI_Status status;
    MPI_Recv(&data, 1, MPIType<T>(), rank, 0, MPI_COMM_WORLD, &status);
}

template <>
void Recv(std::string &data, int rank) {
    MPI_Status status;
    int count;
    MPI_Probe(rank, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &count);
    char recv[count];
    MPI_Recv(recv, count, MPI_CHAR, rank, 0, MPI_COMM_WORLD, &status);
    data = std::string(recv, count);
}

int SelectMinimalRankID(bool b) {
    int ret;
    int myrank = Rank();
    if (!b) {
        myrank = Size();
    }
    Allreduce(ret, myrank, MPI_MIN);
    return ret;
}
}  // namespace GTestMPI

#endif  // GTEST_MPI_GTEST_MPI_H
