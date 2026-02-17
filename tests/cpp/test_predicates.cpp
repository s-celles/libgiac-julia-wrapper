/**
 * @file test_predicates.cpp
 * @brief Tests for Gen type predicates (Feature 004)
 *
 * Tests for REQ-C10..C17: Type predicate functions
 */

#include "giac_impl.h"
#include <iostream>
#include <cassert>
#include <string>
#include <stdexcept>

using namespace giac_julia;

// Simple test framework macros
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    try { test_##name(); std::cout << "PASSED" << std::endl; } \
    catch (const std::exception& e) { std::cout << "FAILED: " << e.what() << std::endl; return 1; } \
} while(0)

// ============================================================================
// REQ-C10: gen_is_integer - true iff type is _INT_ or _ZINT_
// ============================================================================

TEST(is_integer_for_int) {
    Gen g(static_cast<int64_t>(42));
    assert(g.is_integer());
    std::cout << "is_integer(42)=true ";
}

TEST(is_integer_for_bigint) {
    // factorial(100) produces a _ZINT_ (big integer)
    Gen g = giac_eval("factorial(100)");
    assert(g.is_integer());
    std::cout << "is_integer(factorial(100))=true ";
}

TEST(is_integer_for_double) {
    Gen g(3.14);
    assert(!g.is_integer());
    std::cout << "is_integer(3.14)=false ";
}

TEST(is_integer_for_fraction) {
    Gen g = giac_eval("3/7");
    assert(!g.is_integer());
    std::cout << "is_integer(3/7)=false ";
}

// ============================================================================
// REQ-C11: gen_is_numeric - true iff type is _INT_, _DOUBLE_, _ZINT_, or _REAL_
// ============================================================================

TEST(is_numeric_for_int) {
    Gen g(static_cast<int64_t>(42));
    assert(g.is_numeric());
    std::cout << "is_numeric(42)=true ";
}

TEST(is_numeric_for_double) {
    Gen g(3.14);
    assert(g.is_numeric());
    std::cout << "is_numeric(3.14)=true ";
}

TEST(is_numeric_for_bigint) {
    Gen g = giac_eval("factorial(100)");
    assert(g.is_numeric());
    std::cout << "is_numeric(factorial(100))=true ";
}

TEST(is_numeric_for_symbolic) {
    Gen g = giac_eval("sin(x)");
    assert(!g.is_numeric());
    std::cout << "is_numeric(sin(x))=false ";
}

TEST(is_numeric_for_fraction) {
    // Note: Fractions are NOT numeric in the strict sense (they're _FRAC_ type)
    Gen g = giac_eval("3/7");
    assert(!g.is_numeric());
    std::cout << "is_numeric(3/7)=false ";
}

// ============================================================================
// REQ-C12: gen_is_vector - true iff type is _VECT_
// ============================================================================

TEST(is_vector_for_list) {
    Gen g = giac_eval("[1, 2, 3]");
    assert(g.is_vector());
    std::cout << "is_vector([1,2,3])=true ";
}

TEST(is_vector_for_matrix) {
    Gen g = giac_eval("[[1,2],[3,4]]");
    assert(g.is_vector());
    std::cout << "is_vector(matrix)=true ";
}

TEST(is_vector_for_int) {
    Gen g(static_cast<int64_t>(42));
    assert(!g.is_vector());
    std::cout << "is_vector(42)=false ";
}

// ============================================================================
// REQ-C13: gen_is_symbolic - true iff type is _SYMB_
// ============================================================================

TEST(is_symbolic_for_expression) {
    Gen g = giac_eval("sin(x) + 1");
    assert(g.is_symbolic());
    std::cout << "is_symbolic(sin(x)+1)=true ";
}

TEST(is_symbolic_for_int) {
    Gen g(static_cast<int64_t>(42));
    assert(!g.is_symbolic());
    std::cout << "is_symbolic(42)=false ";
}

TEST(is_symbolic_for_identifier) {
    Gen g = giac_eval("x");
    // An identifier is _IDNT_, not _SYMB_
    assert(!g.is_symbolic());
    std::cout << "is_symbolic(x)=false (identifier) ";
}

// ============================================================================
// REQ-C14: gen_is_identifier - true iff type is _IDNT_
// ============================================================================

TEST(is_identifier_for_x) {
    Gen g = giac_eval("x");
    assert(g.is_identifier());
    std::cout << "is_identifier(x)=true ";
}

TEST(is_identifier_for_number) {
    Gen g(static_cast<int64_t>(42));
    assert(!g.is_identifier());
    std::cout << "is_identifier(42)=false ";
}

TEST(is_identifier_for_expression) {
    Gen g = giac_eval("x + 1");
    assert(!g.is_identifier());
    std::cout << "is_identifier(x+1)=false ";
}

// ============================================================================
// REQ-C15: gen_is_fraction - true iff type is _FRAC_
// ============================================================================

TEST(is_fraction_for_frac) {
    Gen g = giac_eval("3/7");
    assert(g.is_fraction());
    std::cout << "is_fraction(3/7)=true ";
}

TEST(is_fraction_for_int) {
    Gen g(static_cast<int64_t>(42));
    assert(!g.is_fraction());
    std::cout << "is_fraction(42)=false ";
}

TEST(is_fraction_for_reduced) {
    // 6/2 reduces to 3, so it's not a fraction
    Gen g = giac_eval("6/2");
    assert(!g.is_fraction());
    std::cout << "is_fraction(6/2)=false (reduces to 3) ";
}

// ============================================================================
// REQ-C16: gen_is_complex - true iff type is _CPLX_
// ============================================================================

TEST(is_complex_for_complex) {
    Gen g = giac_eval("2+3*i");
    assert(g.is_complex());
    std::cout << "is_complex(2+3i)=true ";
}

TEST(is_complex_for_real) {
    Gen g(3.14);
    assert(!g.is_complex());
    std::cout << "is_complex(3.14)=false ";
}

TEST(is_complex_for_pure_imag) {
    Gen g = giac_eval("5*i");
    assert(g.is_complex());
    std::cout << "is_complex(5i)=true ";
}

// ============================================================================
// REQ-C17: gen_is_string - true iff type is _STRNG_
// ============================================================================

TEST(is_string_for_string) {
    Gen g = giac_eval("\"hello\"");
    assert(g.is_string());
    std::cout << "is_string(\"hello\")=true ";
}

TEST(is_string_for_int) {
    Gen g(static_cast<int64_t>(42));
    assert(!g.is_string());
    std::cout << "is_string(42)=false ";
}

int main() {
    std::cout << "=== GIAC Wrapper Type Predicate Tests ===" << std::endl;

    // REQ-C10: is_integer
    RUN_TEST(is_integer_for_int);
    RUN_TEST(is_integer_for_bigint);
    RUN_TEST(is_integer_for_double);
    RUN_TEST(is_integer_for_fraction);

    // REQ-C11: is_numeric
    RUN_TEST(is_numeric_for_int);
    RUN_TEST(is_numeric_for_double);
    RUN_TEST(is_numeric_for_bigint);
    RUN_TEST(is_numeric_for_symbolic);
    RUN_TEST(is_numeric_for_fraction);

    // REQ-C12: is_vector
    RUN_TEST(is_vector_for_list);
    RUN_TEST(is_vector_for_matrix);
    RUN_TEST(is_vector_for_int);

    // REQ-C13: is_symbolic
    RUN_TEST(is_symbolic_for_expression);
    RUN_TEST(is_symbolic_for_int);
    RUN_TEST(is_symbolic_for_identifier);

    // REQ-C14: is_identifier
    RUN_TEST(is_identifier_for_x);
    RUN_TEST(is_identifier_for_number);
    RUN_TEST(is_identifier_for_expression);

    // REQ-C15: is_fraction
    RUN_TEST(is_fraction_for_frac);
    RUN_TEST(is_fraction_for_int);
    RUN_TEST(is_fraction_for_reduced);

    // REQ-C16: is_complex
    RUN_TEST(is_complex_for_complex);
    RUN_TEST(is_complex_for_real);
    RUN_TEST(is_complex_for_pure_imag);

    // REQ-C17: is_string
    RUN_TEST(is_string_for_string);
    RUN_TEST(is_string_for_int);

    std::cout << "=== All predicate tests passed ===" << std::endl;
    return 0;
}
