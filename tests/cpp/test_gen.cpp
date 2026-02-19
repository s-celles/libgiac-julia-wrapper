/**
 * @file test_gen.cpp
 * @brief Tests for Gen objects (User Story 3)
 */

#include "giac_impl.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cstdint>
#include <algorithm>

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
        throw std::runtime_error("Expected: " + std::to_string(expected) + ", Got: " + std::to_string(actual)); \
    } \
} while(0)

// T043: Test Gen construction from string
TEST(gen_construction_string) {
    Gen g("x + 1");
    assert(!g.to_string().empty());
    std::cout << "Gen(string) = " << g.to_string() << " ";
}

// T-010: Test Gen construction from int64_t
TEST(gen_construction_int64) {
    Gen g(static_cast<int64_t>(42));
    assert(g.to_string() == "42");
    assert(g.type() == 0);  // _INT_ type
    std::cout << "Gen(42) = " << g.to_string() << " ";
}

// T-011: Test Gen construction from double
TEST(gen_construction_double) {
    Gen g(3.14);
    std::string s = g.to_string();
    assert(s.find("3.14") != std::string::npos || s.find("3.1") != std::string::npos);
    assert(g.type() == 1);  // _DOUBLE_ type
    std::cout << "Gen(3.14) = " << g.to_string() << " ";
}

// T-012: Test giac_eval function
TEST(giac_eval_function) {
    Gen result = giac_eval("2 + 3");
    assert(result.to_string() == "5");
    std::cout << "giac_eval(\"2 + 3\") = " << result.to_string() << " ";
}

TEST(giac_eval_symbolic) {
    Gen result = giac_eval("x^2 + 2*x + 1");
    std::string s = result.to_string();
    assert(!s.empty());
    std::cout << "giac_eval(\"x^2 + 2*x + 1\") = " << s << " ";
}

TEST(giac_eval_matrix) {
    Gen result = giac_eval("[[1,2],[3,4]]");
    std::string s = result.to_string();
    assert(s.find("1") != std::string::npos);
    assert(s.find("4") != std::string::npos);
    std::cout << "giac_eval(\"[[1,2],[3,4]]\") = " << s << " ";
}

// T-032: Test apply_func single argument
TEST(apply_func_single) {
    Gen n(static_cast<int64_t>(120));
    Gen result = apply_func1("ifactor", n);
    std::string s = result.to_string();
    // ifactor(120) = 2^3*3*5
    assert(s.find("2") != std::string::npos);
    assert(s.find("3") != std::string::npos);
    assert(s.find("5") != std::string::npos);
    std::cout << "apply_func1(\"ifactor\", 120) = " << s << " ";
}

TEST(apply_func_sin) {
    Gen x = giac_eval("x");
    Gen result = apply_func1("sin", x);
    std::string s = result.to_string();
    assert(s.find("sin") != std::string::npos);
    std::cout << "apply_func1(\"sin\", x) = " << s << " ";
}

// Test apply_func2 (two arguments)
TEST(apply_func2_diff) {
    Gen expr = giac_eval("x^2");
    Gen var = giac_eval("x");
    Gen result = apply_func2("diff", expr, var);
    std::string s = result.to_string();
    // diff(x^2, x) = 2*x
    assert(s.find("2") != std::string::npos);
    std::cout << "apply_func2(\"diff\", x^2, x) = " << s << " ";
}

TEST(apply_func2_det) {
    Gen matrix = giac_eval("[[1,2],[3,4]]");
    Gen result = apply_func1("det", matrix);
    std::string s = result.to_string();
    // det([[1,2],[3,4]]) = -2
    assert(s == "-2");
    std::cout << "apply_func1(\"det\", matrix) = " << s << " ";
}

// T-033: Test function listing
TEST(list_builtin_functions) {
    std::string funcs = list_builtin_functions();
    assert(!funcs.empty());
    // Check that some known functions exist
    assert(funcs.find("sin") != std::string::npos);
    std::cout << "list_builtin_functions() found " << std::count(funcs.begin(), funcs.end(), '\n') + 1 << " functions ";
}

TEST(builtin_function_count_test) {
    int count = builtin_function_count();
    assert(count > 0);
    std::cout << "builtin_function_count() = " << count << " ";
}

TEST(list_all_functions_test) {
    // First initialize help for documented commands
    init_help("/usr/share/giac/aide_cas");
    std::string funcs = list_all_functions();
    assert(!funcs.empty());
    std::cout << "list_all_functions() found " << std::count(funcs.begin(), funcs.end(), '\n') + 1 << " functions ";
}

// T-050: Test subtype accessor
TEST(gen_subtype) {
    Gen matrix = giac_eval("[[1,2],[3,4]]");
    int32_t st = matrix.subtype();
    // Matrix subtype should be 11
    assert(st == 11);
    std::cout << "subtype(matrix) = " << st << " ";
}

// T-055: Test to_int64 accessor
TEST(gen_to_int64) {
    Gen g(static_cast<int64_t>(42));
    int64_t val = g.to_int64();
    assert(val == 42);
    std::cout << "to_int64(42) = " << val << " ";
}

// T-056: Test to_double accessor
TEST(gen_to_double) {
    Gen g(3.14);
    double val = g.to_double();
    assert(val > 3.13 && val < 3.15);
    std::cout << "to_double(3.14) = " << val << " ";
}

// T-059: Test frac_num and frac_den
TEST(gen_frac_accessors) {
    Gen frac = giac_eval("3/7");
    Gen num = frac.frac_num();
    Gen den = frac.frac_den();
    assert(num.to_string() == "3");
    assert(den.to_string() == "7");
    std::cout << "frac_num(3/7) = " << num.to_string() << ", frac_den(3/7) = " << den.to_string() << " ";
}

// T-060: Test vect_size and vect_at
TEST(gen_vect_accessors) {
    Gen v = giac_eval("[1, 2, 3, 4, 5]");
    int32_t size = v.vect_size();
    assert(size == 5);
    Gen first = v.vect_at(0);
    Gen last = v.vect_at(4);
    assert(first.to_string() == "1");
    assert(last.to_string() == "5");
    std::cout << "vect_size = " << size << ", vect_at(0) = " << first.to_string() << " ";
}

// T-058: Test cplx_re and cplx_im
TEST(gen_cplx_accessors) {
    Gen c = giac_eval("2+3*i");
    Gen re = c.cplx_re();
    Gen im = c.cplx_im();
    assert(re.to_string() == "2");
    assert(im.to_string() == "3");
    std::cout << "cplx_re(2+3i) = " << re.to_string() << ", cplx_im(2+3i) = " << im.to_string() << " ";
}

// T-062: Test idnt_name
TEST(gen_idnt_name) {
    Gen x = giac_eval("x");
    std::string name = x.idnt_name();
    assert(name == "x");
    std::cout << "idnt_name(x) = " << name << " ";
}

// T-093 to T-096: Test predicates
TEST(gen_predicates) {
    Gen zero(static_cast<int64_t>(0));
    Gen one(static_cast<int64_t>(1));
    Gen two(static_cast<int64_t>(2));
    Gen pi = giac_eval("3.14159");

    assert(zero.is_zero());
    assert(!one.is_zero());
    assert(one.is_one());
    assert(!two.is_one());
    assert(zero.is_integer());
    assert(one.is_integer());
    assert(!pi.is_integer());

    std::cout << "is_zero(0)=" << zero.is_zero() << ", is_one(1)=" << one.is_one() << ", is_integer(2)=" << two.is_integer() << " ";
}

// ============================================================================
// Tier 1 Direct Wrapper Tests (Phase 7: T-141, T-142)
// ============================================================================

// Test trigonometric functions
TEST(tier1_trig) {
    Gen x = giac_eval("x");
    Gen result = giac_sin(x);
    std::string s = result.to_string();
    assert(s.find("sin") != std::string::npos);

    Gen cos_x = giac_cos(x);
    assert(cos_x.to_string().find("cos") != std::string::npos);

    Gen tan_x = giac_tan(x);
    assert(tan_x.to_string().find("tan") != std::string::npos);

    std::cout << "sin(x)=" << s << ", cos(x)=" << cos_x.to_string() << " ";
}

TEST(tier1_trig_numeric) {
    Gen zero(static_cast<int64_t>(0));
    Gen sin_0 = giac_sin(zero);
    assert(sin_0.is_zero());

    Gen cos_0 = giac_cos(zero);
    assert(cos_0.is_one());

    std::cout << "sin(0)=" << sin_0.to_string() << ", cos(0)=" << cos_0.to_string() << " ";
}

// Test exponential and logarithmic functions
TEST(tier1_exp_log) {
    Gen x = giac_eval("x");
    Gen exp_x = giac_exp(x);
    assert(exp_x.to_string().find("exp") != std::string::npos);

    Gen ln_x = giac_ln(x);
    assert(ln_x.to_string().find("ln") != std::string::npos);

    Gen sqrt_x = giac_sqrt(x);
    assert(sqrt_x.to_string().find("sqrt") != std::string::npos);

    std::cout << "exp(x)=" << exp_x.to_string() << ", ln(x)=" << ln_x.to_string() << " ";
}

TEST(tier1_exp_numeric) {
    Gen zero(static_cast<int64_t>(0));
    Gen exp_0 = giac_exp(zero);
    assert(exp_0.is_one());

    Gen one(static_cast<int64_t>(1));
    Gen ln_1 = giac_ln(one);
    assert(ln_1.is_zero());

    std::cout << "exp(0)=" << exp_0.to_string() << ", ln(1)=" << ln_1.to_string() << " ";
}

// Test arithmetic functions
TEST(tier1_arithmetic) {
    Gen minus_five(static_cast<int64_t>(-5));
    Gen abs_result = giac_abs(minus_five);
    assert(abs_result.to_string() == "5");

    Gen sign_result = giac_sign(minus_five);
    assert(sign_result.to_string() == "-1");

    Gen pi = giac_eval("3.7");
    Gen floor_result = giac_floor(pi);
    assert(floor_result.to_string() == "3");

    Gen ceil_result = giac_ceil(pi);
    assert(ceil_result.to_string() == "4");

    std::cout << "abs(-5)=" << abs_result.to_string() << ", floor(3.7)=" << floor_result.to_string() << " ";
}

// Test complex functions
TEST(tier1_complex) {
    Gen c = giac_eval("2+3*i");
    Gen re_c = giac_re(c);
    Gen im_c = giac_im(c);
    Gen conj_c = giac_conj(c);

    assert(re_c.to_string() == "2");
    assert(im_c.to_string() == "3");
    // conj(2+3i) = 2-3i
    std::string conj_s = conj_c.to_string();
    assert(conj_s.find("2") != std::string::npos);
    assert(conj_s.find("-3") != std::string::npos || conj_s.find("- 3") != std::string::npos);

    std::cout << "re(2+3i)=" << re_c.to_string() << ", im(2+3i)=" << im_c.to_string() << " ";
}

// Test calculus functions
TEST(tier1_diff) {
    Gen expr = giac_eval("x^2");
    Gen var = giac_eval("x");
    Gen result = giac_diff(expr, var);
    std::string s = result.to_string();
    // diff(x^2, x) = 2*x
    assert(s.find("2") != std::string::npos);
    assert(s.find("x") != std::string::npos);

    std::cout << "diff(x^2, x)=" << s << " ";
}

TEST(tier1_integrate) {
    Gen expr = giac_eval("x");
    Gen var = giac_eval("x");
    Gen result = giac_integrate(expr, var);
    std::string s = result.to_string();
    // integrate(x, x) = x^2/2
    assert(s.find("x") != std::string::npos);
    assert(s.find("2") != std::string::npos);

    std::cout << "integrate(x, x)=" << s << " ";
}

TEST(tier1_subst) {
    Gen expr = giac_eval("x^2 + x + 1");
    Gen var = giac_eval("x");
    Gen val(static_cast<int64_t>(2));
    Gen result = giac_subst(expr, var, val);
    std::string s = result.to_string();
    // subst(x^2 + x + 1, x, 2) = 4 + 2 + 1 = 7
    assert(s == "7");

    std::cout << "subst(x^2+x+1, x, 2)=" << s << " ";
}

// Test gcd and lcm
TEST(tier1_gcd_lcm) {
    Gen a(static_cast<int64_t>(12));
    Gen b(static_cast<int64_t>(18));

    Gen gcd_result = giac_gcd(a, b);
    assert(gcd_result.to_string() == "6");

    Gen lcm_result = giac_lcm(a, b);
    assert(lcm_result.to_string() == "36");

    std::cout << "gcd(12,18)=" << gcd_result.to_string() << ", lcm(12,18)=" << lcm_result.to_string() << " ";
}

// Test power function
TEST(tier1_pow) {
    Gen base(static_cast<int64_t>(2));
    Gen exp(static_cast<int64_t>(10));
    Gen result = giac_pow(base, exp);
    assert(result.to_string() == "1024");

    std::cout << "pow(2,10)=" << result.to_string() << " ";
}

int main() {
    std::cout << "=== GIAC Wrapper Gen Tests ===" << std::endl;

    // Scalar constructors and giac_eval tests (Phase 2: T-010 to T-014)
    RUN_TEST(gen_construction_string);
    RUN_TEST(gen_construction_int64);
    RUN_TEST(gen_construction_double);
    RUN_TEST(giac_eval_function);
    RUN_TEST(giac_eval_symbolic);
    RUN_TEST(giac_eval_matrix);

    // Generic dispatch tests (Phase 3: T-032)
    RUN_TEST(apply_func_single);
    RUN_TEST(apply_func_sin);
    RUN_TEST(apply_func2_diff);
    RUN_TEST(apply_func2_det);

    // Function listing tests (Phase 3: T-033)
    RUN_TEST(list_builtin_functions);
    RUN_TEST(builtin_function_count_test);
    RUN_TEST(list_all_functions_test);

    // Type introspection tests (Phase 5: T-050 to T-063)
    RUN_TEST(gen_subtype);
    RUN_TEST(gen_to_int64);
    RUN_TEST(gen_to_double);
    RUN_TEST(gen_frac_accessors);
    RUN_TEST(gen_vect_accessors);
    RUN_TEST(gen_cplx_accessors);
    RUN_TEST(gen_idnt_name);

    // Predicates tests (Phase 6: T-093 to T-096)
    RUN_TEST(gen_predicates);

    // Tier 1 Direct Wrapper tests (Phase 7: T-141, T-142)
    RUN_TEST(tier1_trig);
    RUN_TEST(tier1_trig_numeric);
    RUN_TEST(tier1_exp_log);
    RUN_TEST(tier1_exp_numeric);
    RUN_TEST(tier1_arithmetic);
    RUN_TEST(tier1_complex);
    RUN_TEST(tier1_diff);
    RUN_TEST(tier1_integrate);
    RUN_TEST(tier1_subst);
    RUN_TEST(tier1_gcd_lcm);
    RUN_TEST(tier1_pow);

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
