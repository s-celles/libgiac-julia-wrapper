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

// Test timeout configuration: default 30, round-trip, per-context isolation.
TEST(timeout_config) {
    // (a) Default.
    GiacContext ctx;
    if (ctx.get_timeout() != 30.0) {
        throw std::runtime_error("Expected default timeout 30.0, got: " +
                                 std::to_string(ctx.get_timeout()));
    }

    // (b) Round-trip.
    ctx.set_timeout(60.0);
    if (ctx.get_timeout() != 60.0) {
        throw std::runtime_error("Expected get_timeout() == 60.0 after set_timeout(60.0), got: " +
                                 std::to_string(ctx.get_timeout()));
    }
    ctx.set_timeout(0.0);
    if (ctx.get_timeout() != 0.0) {
        throw std::runtime_error("Expected get_timeout() == 0.0 after set_timeout(0.0), got: " +
                                 std::to_string(ctx.get_timeout()));
    }

    // (c) Per-context isolation.
    GiacContext ctx_a;
    GiacContext ctx_b;
    ctx_a.set_timeout(60.0);
    ctx_b.set_timeout(120.0);
    if (ctx_a.get_timeout() != 60.0 || ctx_b.get_timeout() != 120.0) {
        throw std::runtime_error("Per-context isolation broken: a=" +
                                 std::to_string(ctx_a.get_timeout()) + ", b=" +
                                 std::to_string(ctx_b.get_timeout()));
    }
    // A fresh context still reads the default — no leak from a or b.
    GiacContext ctx_c;
    if (ctx_c.get_timeout() != 30.0) {
        throw std::runtime_error("Fresh context did not get default timeout, got: " +
                                 std::to_string(ctx_c.get_timeout()));
    }
}

// Test precision configuration: round-trip, isolation, effect on evalf(pi).
TEST(precision_config) {
    GiacContext ctx;

    // (a) Read GIAC's default — empirically 14 on this libgiac, but we don't hardcode it.
    int default_prec = ctx.get_precision();
    if (default_prec <= 0) {
        throw std::runtime_error("Expected positive default precision, got: " +
                                 std::to_string(default_prec));
    }

    // (b) Round-trip.
    ctx.set_precision(50);
    if (ctx.get_precision() != 50) {
        throw std::runtime_error("Expected get_precision() == 50 after set_precision(50), got: " +
                                 std::to_string(ctx.get_precision()));
    }

    // (c) Per-context isolation.
    GiacContext ctx_a;
    GiacContext ctx_b;
    ctx_a.set_precision(50);
    ctx_b.set_precision(20);
    if (ctx_a.get_precision() != 50 || ctx_b.get_precision() != 20) {
        throw std::runtime_error("Per-context isolation broken: a=" +
                                 std::to_string(ctx_a.get_precision()) + ", b=" +
                                 std::to_string(ctx_b.get_precision()));
    }

    // (d) Effect on evaluation: evalf(pi) at precision 50 must carry at least 45
    // significant digits. Result strings start with "3." — count chars after the dot.
    GiacContext ctx_50;
    ctx_50.set_precision(50);
    std::string pi_50 = ctx_50.eval("evalf(pi)");
    auto dot = pi_50.find('.');
    if (dot == std::string::npos) {
        throw std::runtime_error("Expected evalf(pi) to contain a decimal point, got: " + pi_50);
    }
    // Count consecutive digits after the dot.
    size_t digits_after_dot = 0;
    for (size_t i = dot + 1; i < pi_50.size() && std::isdigit(static_cast<unsigned char>(pi_50[i])); ++i) {
        ++digits_after_dot;
    }
    if (digits_after_dot < 45) {
        throw std::runtime_error("Expected at least 45 digits after the dot at precision 50, got " +
                                 std::to_string(digits_after_dot) + " digits in: " + pi_50);
    }
}

// Test complex mode: round-trip, per-context isolation, effect on evaluation.
TEST(complex_mode) {
    // (a) Round-trip on a single context.
    GiacContext ctx;
    ctx.set_complex_mode(true);
    assert(ctx.is_complex_mode() == true);
    ctx.set_complex_mode(false);
    assert(ctx.is_complex_mode() == false);

    // (b) Per-context isolation: two contexts with opposite values both report their own.
    GiacContext ctx_a;
    GiacContext ctx_b;
    ctx_a.set_complex_mode(true);
    ctx_b.set_complex_mode(false);
    assert(ctx_a.is_complex_mode() == true);
    assert(ctx_b.is_complex_mode() == false);
    // Re-read after flipping ctx_a — ctx_b must be unaffected.
    ctx_a.set_complex_mode(false);
    assert(ctx_a.is_complex_mode() == false);
    assert(ctx_b.is_complex_mode() == false);

    // (c) Effect on evaluation: factor(x^2+1) splits over complex only when complex mode is on.
    // Empirically sqrt(-1) returns 'i' regardless of complex_mode in this giac version, so it
    // is not a witness for the flag's effect on evaluation; factor and solve are.
    // mode=1: factor(x^2+1) => (x+i)*(x-i)    mode=0: factor(x^2+1) => x^2+1
    GiacContext ctx_on;
    ctx_on.set_complex_mode(true);
    std::string r_on = ctx_on.eval("factor(x^2+1)");
    if (r_on.find('i') == std::string::npos) {
        throw std::runtime_error(
            "Expected complex-mode factor(x^2+1) to contain 'i', got: " + r_on);
    }

    GiacContext ctx_off;
    ctx_off.set_complex_mode(false);
    std::string r_off = ctx_off.eval("factor(x^2+1)");
    if (r_off.find('i') != std::string::npos) {
        throw std::runtime_error(
            "Expected real-mode factor(x^2+1) to NOT contain 'i', got: " + r_off);
    }
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
