#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace whacker::tests {

[[noreturn]] inline void fail_check(const char* file, const int line, const char* expr) {
    std::cerr << file << ':' << line << ": TEST_CHECK failed: " << expr << '\n';
    std::abort();
}

[[noreturn]] inline void fail_check_eq(
    const char* file,
    const int line,
    const char* lhs_expr,
    const char* rhs_expr) {
    std::cerr << file << ':' << line << ": TEST_CHECK_EQ failed: " << lhs_expr << " == " << rhs_expr
              << '\n';
    std::abort();
}

[[noreturn]] inline void fail_check_near(
    const char* file,
    const int line,
    const char* lhs_expr,
    const char* rhs_expr,
    const char* eps_expr) {
    std::cerr << file << ':' << line << ": TEST_CHECK_NEAR failed: |" << lhs_expr << " - " << rhs_expr
              << "| <= " << eps_expr << '\n';
    std::abort();
}

template <typename TLhs, typename TRhs>
inline void check_eq_impl(
    const TLhs& lhs,
    const TRhs& rhs,
    const char* file,
    const int line,
    const char* lhs_expr,
    const char* rhs_expr) {
    if (!(lhs == rhs)) {
        fail_check_eq(file, line, lhs_expr, rhs_expr);
    }
}

template <typename TLhs, typename TRhs, typename TEps>
inline void check_near_impl(
    const TLhs lhs,
    const TRhs rhs,
    const TEps eps,
    const char* file,
    const int line,
    const char* lhs_expr,
    const char* rhs_expr,
    const char* eps_expr) {
    const double diff = std::fabs(static_cast<double>(lhs) - static_cast<double>(rhs));
    if (!(diff <= static_cast<double>(eps))) {
        fail_check_near(file, line, lhs_expr, rhs_expr, eps_expr);
    }
}

}  // namespace whacker::tests

#define TEST_CHECK(condition)                                                                          \
    do {                                                                                               \
        if (!(condition)) {                                                                            \
            ::whacker::tests::fail_check(__FILE__, __LINE__, #condition);                             \
        }                                                                                              \
    } while (false)

#define TEST_CHECK_EQ(lhs, rhs)                                                                        \
    do {                                                                                               \
        const auto& test_check_lhs = (lhs);                                                            \
        const auto& test_check_rhs = (rhs);                                                            \
        ::whacker::tests::check_eq_impl(                                                               \
            test_check_lhs, test_check_rhs, __FILE__, __LINE__, #lhs, #rhs);                          \
    } while (false)

#define TEST_CHECK_NEAR(lhs, rhs, eps)                                                                 \
    do {                                                                                               \
        const auto test_check_lhs = (lhs);                                                             \
        const auto test_check_rhs = (rhs);                                                             \
        const auto test_check_eps = (eps);                                                             \
        ::whacker::tests::check_near_impl(                                                             \
            test_check_lhs,                                                                             \
            test_check_rhs,                                                                             \
            test_check_eps,                                                                             \
            __FILE__,                                                                                   \
            __LINE__,                                                                                   \
            #lhs,                                                                                       \
            #rhs,                                                                                       \
            #eps);                                                                                      \
    } while (false)
