/**
 * @file test_context.cpp
 * @brief Tests for context management (User Story 2)
 */

#include "giac_wrapper.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace giac_julia;

// Simple test framework macros
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    try { test_##name(); std::cout << "PASSED" << std::endl; } \
    catch (const std::exception& e) { std::cout << "FAILED: " << e.what() << std::endl; return 1; } \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Expected: " + std::string(expected) + ", Got: " + std::string(actual)); \
    } \
} while(0)

// T030: Test variable assignment - assign "a:=5", then evaluate "a+3" = "8"
TEST(variable_assignment) {
    GiacContext ctx;
    ctx.set_variable("a", "5");
    std::string result = ctx.eval("a+3");
    ASSERT_EQ("8", result);
}

// T031: Test context isolation - variables in one context don't affect another
TEST(context_isolation) {
    GiacContext ctx1;
    GiacContext ctx2;

    ctx1.set_variable("x", "10");
    ctx2.set_variable("x", "20");

    ASSERT_EQ("10", ctx1.get_variable("x"));
    ASSERT_EQ("20", ctx2.get_variable("x"));
}

// Test timeout configuration
TEST(timeout_config) {
    GiacContext ctx;

    // Default timeout should be 30 seconds
    assert(ctx.get_timeout() == DEFAULT_TIMEOUT_SECONDS);

    ctx.set_timeout(60);
    assert(ctx.get_timeout() == 60);

    ctx.set_timeout(0);  // No limit
    assert(ctx.get_timeout() == 0);
}

// Test precision configuration
TEST(precision_config) {
    GiacContext ctx;

    ctx.set_precision(50);
    assert(ctx.get_precision() == 50);
}

// Test complex mode
TEST(complex_mode) {
    GiacContext ctx;

    ctx.set_complex_mode(true);
    assert(ctx.is_complex_mode() == true);

    ctx.set_complex_mode(false);
    assert(ctx.is_complex_mode() == false);
}

int main() {
    std::cout << "=== GIAC Wrapper Context Tests ===" << std::endl;

    RUN_TEST(variable_assignment);
    RUN_TEST(context_isolation);
    RUN_TEST(timeout_config);
    RUN_TEST(precision_config);
    RUN_TEST(complex_mode);

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
