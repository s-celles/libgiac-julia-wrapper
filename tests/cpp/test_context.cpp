/**
 * @file test_context.cpp
 * @brief Tests for context management (User Story 2)
 */

#include "giac_impl.h"
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

// Issue #3: free-function giac_eval(expr, ctx) — returns a Gen and
// preserves per-context isolation. Binding `a := 5` through one context
// must NOT be visible to a fresh context.
TEST(giac_eval_with_context_returns_gen) {
    GiacContext ctx1;
    GiacContext ctx2;

    // Bind in ctx1 via the new context-aware giac_eval.
    Gen r1 = giac_eval("ctx_iso_a := 7", ctx1);
    ASSERT_EQ("7", r1.to_string());

    // Reading the same name in ctx1 sees the binding.
    Gen r1_read = giac_eval("ctx_iso_a", ctx1);
    ASSERT_EQ("7", r1_read.to_string());

    // ctx2 is independent — the same name is still the unbound symbol.
    Gen r2_read = giac_eval("ctx_iso_a", ctx2);
    ASSERT_EQ("ctx_iso_a", r2_read.to_string());
}

// Issue #3 regression (MCP scenario): binding `y` in one context must not
// poison `desolve(..., y)` in a fresh, independent context. Under the
// pre-fix singleton-context behavior, this combination produced
// `Error: Dependent variable assigned. Run purge(y)`.
TEST(issue3_bound_var_does_not_poison_desolve_in_other_context) {
    GiacContext ctx_with_binding;
    GiacContext ctx_fresh;

    // Establish the binding that used to leak.
    (void) giac_eval("y := 42", ctx_with_binding);

    // In a fresh, independent context, a desolve referencing y must work.
    // (Calling this same desolve in ctx_with_binding throws
    // "Dependent variable assigned. Run purge(y)" — verified manually.)
    Gen result = giac_eval("desolve(diff(y,t)=cos(t), t, y)", ctx_fresh);
    std::string s = result.to_string();

    if (s.find("Dependent variable") != std::string::npos) {
        throw std::runtime_error(
            "desolve in ctx_fresh was poisoned by ctx_with_binding's binding: "
            + s
        );
    }
    if (s.empty()) {
        throw std::runtime_error("desolve returned an empty result string");
    }

    // And ctx_with_binding still holds y = 42 — bindings inside a context
    // persist across calls within that context.
    Gen y_in_ctx1 = giac_eval("y", ctx_with_binding);
    ASSERT_EQ("42", y_in_ctx1.to_string());
}

// Test timeout configuration
TEST(timeout_config) {
    GiacContext ctx;

    // Timeout is not yet implemented, always returns 0
    assert(ctx.get_timeout() == 0);

    ctx.set_timeout(60);
    assert(ctx.get_timeout() == 0);  // stub: still returns 0

    ctx.set_timeout(0);
    assert(ctx.get_timeout() == 0);
}

// Test precision configuration
TEST(precision_config) {
    GiacContext ctx;

    // Precision setter is not yet implemented, get_precision always returns 15
    ctx.set_precision(50);
    assert(ctx.get_precision() == 15);  // stub: still returns default
}

// Test complex mode
TEST(complex_mode) {
    GiacContext ctx;

    // Complex mode setter is not yet implemented, always returns false
    ctx.set_complex_mode(true);
    assert(ctx.is_complex_mode() == false);  // stub: still returns false

    ctx.set_complex_mode(false);
    assert(ctx.is_complex_mode() == false);
}

int main() {
    std::cout << "=== GIAC Wrapper Context Tests ===" << std::endl;

    RUN_TEST(variable_assignment);
    RUN_TEST(context_isolation);
    RUN_TEST(giac_eval_with_context_returns_gen);
    RUN_TEST(issue3_bound_var_does_not_poison_desolve_in_other_context);
    RUN_TEST(timeout_config);
    RUN_TEST(precision_config);
    RUN_TEST(complex_mode);

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
