/**
 * @file giac_cabi.cpp
 * @brief C-ABI implementation — compiled with g++ to match GIAC's libstdc++ ABI
 *
 * This file includes giac_impl.h (the real implementation) and wraps every
 * function through extern "C" using only C-compatible types.
 * It is compiled with g++ alongside giac_impl.cpp into a static library
 * (libgiac_cabi.a) that is then linked into the final shared library.
 */

#include "giac_impl.h"
#include "giac_cabi.h"

#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

using namespace giac_julia;

// ============================================================================
// Helpers
// ============================================================================

static char* strdup_malloc(const std::string& s) {
    char* r = static_cast<char*>(malloc(s.size() + 1));
    if (r) memcpy(r, s.c_str(), s.size() + 1);
    return r;
}

static Gen* G(GenHandle h) { return static_cast<Gen*>(h); }
static GiacContext* CTX(GiacContextHandle h) { return static_cast<GiacContext*>(h); }

// Thread-local error buffer for exception propagation across extern "C"
static thread_local std::string cabi_last_error;

#define CABI_TRY try { cabi_last_error.clear();
#define CABI_CATCH_PTR } catch (const std::exception& e) { cabi_last_error = e.what(); return nullptr; } catch (...) { cabi_last_error = "unknown error"; return nullptr; }
#define CABI_CATCH_INT(def) } catch (const std::exception& e) { cabi_last_error = e.what(); return def; } catch (...) { cabi_last_error = "unknown error"; return def; }
#define CABI_CATCH_VOID } catch (const std::exception& e) { cabi_last_error = e.what(); } catch (...) { cabi_last_error = "unknown error"; }
#define CABI_CATCH_DBL(def) } catch (const std::exception& e) { cabi_last_error = e.what(); return def; } catch (...) { cabi_last_error = "unknown error"; return def; }

extern "C" {

// ============================================================================
// String management & error handling
// ============================================================================

void giac_cabi_free_string(char* s) { free(s); }

int giac_cabi_get_last_error(char* buf, int buf_size) {
    if (cabi_last_error.empty()) return 0;
    if (buf && buf_size > 0) {
        int len = static_cast<int>(cabi_last_error.size());
        if (len >= buf_size) len = buf_size - 1;
        memcpy(buf, cabi_last_error.c_str(), len);
        buf[len] = '\0';
    }
    return 1;
}

void giac_cabi_clear_error(void) { cabi_last_error.clear(); }

// ============================================================================
// Version Functions
// ============================================================================

char* giac_cabi_get_giac_version(void) {
    return strdup_malloc(get_giac_version());
}

char* giac_cabi_get_wrapper_version(void) {
    return strdup_malloc(get_wrapper_version());
}

int giac_cabi_check_giac_available(void) {
    return check_giac_available() ? 1 : 0;
}

// ============================================================================
// Configuration
// ============================================================================

void giac_cabi_set_xcasroot(const char* path) {
    set_xcasroot(std::string(path));
}

char* giac_cabi_get_xcasroot(void) {
    return strdup_malloc(get_xcasroot());
}

int giac_cabi_init_help(const char* aide_cas_path) {
    return init_help(std::string(aide_cas_path)) ? 1 : 0;
}

char* giac_cabi_list_commands(void) {
    return strdup_malloc(list_commands());
}

int giac_cabi_help_count(void) {
    return help_count();
}

// ============================================================================
// Expression Evaluation
// ============================================================================

GenHandle giac_cabi_giac_eval(const char* expr) {
    CABI_TRY
    return new Gen(giac_eval(std::string(expr)));
    CABI_CATCH_PTR
}

// ============================================================================
// Generic Dispatch
// ============================================================================

GenHandle giac_cabi_apply_func0(const char* name) {
    CABI_TRY
    return new Gen(apply_func0(std::string(name)));
    CABI_CATCH_PTR
}

GenHandle giac_cabi_apply_func1(const char* name, GenHandle arg) {
    CABI_TRY
    return new Gen(apply_func1(std::string(name), *G(arg)));
    CABI_CATCH_PTR
}

GenHandle giac_cabi_apply_func2(const char* name, GenHandle arg1, GenHandle arg2) {
    CABI_TRY
    return new Gen(apply_func2(std::string(name), *G(arg1), *G(arg2)));
    CABI_CATCH_PTR
}

GenHandle giac_cabi_apply_func3(const char* name, GenHandle arg1, GenHandle arg2, GenHandle arg3) {
    CABI_TRY
    return new Gen(apply_func3(std::string(name), *G(arg1), *G(arg2), *G(arg3)));
    CABI_CATCH_PTR
}

GenHandle giac_cabi_apply_funcN(const char* name, GenHandle* args, int32_t count) {
    CABI_TRY
    std::vector<Gen> vec;
    vec.reserve(count);
    for (int32_t i = 0; i < count; ++i)
        vec.push_back(*G(args[i]));
    return new Gen(apply_funcN(std::string(name), vec));
    CABI_CATCH_PTR
}

// ============================================================================
// Function Listing
// ============================================================================

char* giac_cabi_list_builtin_functions(void) {
    return strdup_malloc(list_builtin_functions());
}

int giac_cabi_builtin_function_count(void) {
    return builtin_function_count();
}

char* giac_cabi_list_all_functions(void) {
    return strdup_malloc(list_all_functions());
}

// ============================================================================
// Tier 1 Direct Wrappers
// ============================================================================

#define CABI_TIER1_1(cname, cppname) \
    GenHandle giac_cabi_##cname(GenHandle arg) { \
        CABI_TRY return new Gen(giac_##cppname(*G(arg))); CABI_CATCH_PTR \
    }

#define CABI_TIER1_2(cname, cppname) \
    GenHandle giac_cabi_##cname(GenHandle a, GenHandle b) { \
        CABI_TRY return new Gen(giac_##cppname(*G(a), *G(b))); CABI_CATCH_PTR \
    }

#define CABI_TIER1_3(cname, cppname) \
    GenHandle giac_cabi_##cname(GenHandle a, GenHandle b, GenHandle c) { \
        CABI_TRY return new Gen(giac_##cppname(*G(a), *G(b), *G(c))); CABI_CATCH_PTR \
    }

CABI_TIER1_1(sin, sin)
CABI_TIER1_1(cos, cos)
CABI_TIER1_1(tan, tan)
CABI_TIER1_1(asin, asin)
CABI_TIER1_1(acos, acos)
CABI_TIER1_1(atan, atan)
CABI_TIER1_1(exp, exp)
CABI_TIER1_1(ln, ln)
CABI_TIER1_1(log10, log10)
CABI_TIER1_1(sqrt, sqrt)
CABI_TIER1_1(abs, abs)
CABI_TIER1_1(sign, sign)
CABI_TIER1_1(floor, floor)
CABI_TIER1_1(ceil, ceil)
CABI_TIER1_1(re, re)
CABI_TIER1_1(im, im)
CABI_TIER1_1(conj, conj)
CABI_TIER1_1(normal, normal)
CABI_TIER1_1(evalf, evalf)

CABI_TIER1_2(diff, diff)
CABI_TIER1_2(integrate, integrate)
CABI_TIER1_2(solve, solve)
CABI_TIER1_2(gcd, gcd)
CABI_TIER1_2(lcm, lcm)
CABI_TIER1_2(pow, pow)

CABI_TIER1_3(subst, subst)
CABI_TIER1_3(limit, limit)
CABI_TIER1_3(series, series)

#undef CABI_TIER1_1
#undef CABI_TIER1_2
#undef CABI_TIER1_3

// ============================================================================
// Gen Lifecycle
// ============================================================================

GenHandle giac_cabi_gen_create_default(void) {
    CABI_TRY return new Gen(); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_create_from_string(const char* expr) {
    CABI_TRY return new Gen(std::string(expr)); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_create_from_int64(int64_t value) {
    CABI_TRY return new Gen(value); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_create_from_double(double value) {
    CABI_TRY return new Gen(value); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_copy(GenHandle src) {
    CABI_TRY return new Gen(*G(src)); CABI_CATCH_PTR
}

void giac_cabi_gen_destroy(GenHandle handle) {
    delete G(handle);
}

// ============================================================================
// Gen Accessors
// ============================================================================

char* giac_cabi_gen_to_string(GenHandle handle) {
    CABI_TRY return strdup_malloc(G(handle)->to_string()); CABI_CATCH_PTR
}

int giac_cabi_gen_type(GenHandle handle) {
    return G(handle)->type();
}

int32_t giac_cabi_gen_subtype(GenHandle handle) {
    return G(handle)->subtype();
}

char* giac_cabi_gen_type_name(GenHandle handle) {
    CABI_TRY return strdup_malloc(G(handle)->type_name()); CABI_CATCH_PTR
}

int64_t giac_cabi_gen_to_int64(GenHandle handle) {
    CABI_TRY return G(handle)->to_int64(); CABI_CATCH_INT(0)
}

int32_t giac_cabi_gen_to_int32(GenHandle handle) {
    CABI_TRY return G(handle)->to_int32(); CABI_CATCH_INT(0)
}

double giac_cabi_gen_to_double(GenHandle handle) {
    CABI_TRY return G(handle)->to_double(); CABI_CATCH_DBL(0.0)
}

char* giac_cabi_gen_zint_to_string(GenHandle handle) {
    CABI_TRY return strdup_malloc(G(handle)->zint_to_string()); CABI_CATCH_PTR
}

int giac_cabi_gen_zint_sign(GenHandle handle) {
    CABI_TRY return G(handle)->zint_sign(); CABI_CATCH_INT(0)
}

int32_t giac_cabi_gen_zint_to_bytes(GenHandle handle, uint8_t* buf, int32_t buf_size) {
    CABI_TRY
    std::vector<uint8_t> bytes = G(handle)->zint_to_bytes();
    int32_t needed = static_cast<int32_t>(bytes.size());
    if (buf && buf_size >= needed) {
        memcpy(buf, bytes.data(), needed);
    }
    return needed;
    CABI_CATCH_INT(0)
}

GenHandle giac_cabi_gen_cplx_re(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->cplx_re()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_cplx_im(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->cplx_im()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_frac_num(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->frac_num()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_frac_den(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->frac_den()); CABI_CATCH_PTR
}

int32_t giac_cabi_gen_vect_size(GenHandle handle) {
    CABI_TRY return G(handle)->vect_size(); CABI_CATCH_INT(0)
}

GenHandle giac_cabi_gen_vect_at(GenHandle handle, int32_t i) {
    CABI_TRY return new Gen(G(handle)->vect_at(i)); CABI_CATCH_PTR
}

char* giac_cabi_gen_symb_sommet_name(GenHandle handle) {
    CABI_TRY return strdup_malloc(G(handle)->symb_sommet_name()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_symb_feuille(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->symb_feuille()); CABI_CATCH_PTR
}

char* giac_cabi_gen_idnt_name(GenHandle handle) {
    CABI_TRY return strdup_malloc(G(handle)->idnt_name()); CABI_CATCH_PTR
}

char* giac_cabi_gen_strng_value(GenHandle handle) {
    CABI_TRY return strdup_malloc(G(handle)->strng_value()); CABI_CATCH_PTR
}

int32_t giac_cabi_gen_map_size(GenHandle handle) {
    CABI_TRY return G(handle)->map_size(); CABI_CATCH_INT(0)
}

GenHandle giac_cabi_gen_map_keys(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->map_keys()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_map_values(GenHandle handle) {
    CABI_TRY return new Gen(G(handle)->map_values()); CABI_CATCH_PTR
}

// ============================================================================
// Gen Predicates
// ============================================================================

int giac_cabi_gen_is_zero(GenHandle h)       { return G(h)->is_zero() ? 1 : 0; }
int giac_cabi_gen_is_one(GenHandle h)        { return G(h)->is_one() ? 1 : 0; }
int giac_cabi_gen_is_integer(GenHandle h)    { return G(h)->is_integer() ? 1 : 0; }
int giac_cabi_gen_is_approx(GenHandle h)     { return G(h)->is_approx() ? 1 : 0; }
int giac_cabi_gen_is_numeric(GenHandle h)    { return G(h)->is_numeric() ? 1 : 0; }
int giac_cabi_gen_is_vector(GenHandle h)     { return G(h)->is_vector() ? 1 : 0; }
int giac_cabi_gen_is_symbolic(GenHandle h)   { return G(h)->is_symbolic() ? 1 : 0; }
int giac_cabi_gen_is_identifier(GenHandle h) { return G(h)->is_identifier() ? 1 : 0; }
int giac_cabi_gen_is_fraction(GenHandle h)   { return G(h)->is_fraction() ? 1 : 0; }
int giac_cabi_gen_is_complex(GenHandle h)    { return G(h)->is_complex() ? 1 : 0; }
int giac_cabi_gen_is_string(GenHandle h)     { return G(h)->is_string() ? 1 : 0; }

// ============================================================================
// Gen Operations
// ============================================================================

GenHandle giac_cabi_gen_eval(GenHandle h) {
    CABI_TRY return new Gen(G(h)->eval()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_simplify(GenHandle h) {
    CABI_TRY return new Gen(G(h)->simplify()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_expand(GenHandle h) {
    CABI_TRY return new Gen(G(h)->expand()); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_factor(GenHandle h) {
    CABI_TRY return new Gen(G(h)->factor()); CABI_CATCH_PTR
}

// ============================================================================
// Gen Arithmetic
// ============================================================================

GenHandle giac_cabi_gen_add(GenHandle a, GenHandle b) {
    CABI_TRY return new Gen(*G(a) + *G(b)); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_sub(GenHandle a, GenHandle b) {
    CABI_TRY return new Gen(*G(a) - *G(b)); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_mul(GenHandle a, GenHandle b) {
    CABI_TRY return new Gen(*G(a) * *G(b)); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_div(GenHandle a, GenHandle b) {
    CABI_TRY return new Gen(*G(a) / *G(b)); CABI_CATCH_PTR
}

GenHandle giac_cabi_gen_neg(GenHandle a) {
    CABI_TRY return new Gen(-*G(a)); CABI_CATCH_PTR
}

int giac_cabi_gen_eq(GenHandle a, GenHandle b) {
    return (*G(a) == *G(b)) ? 1 : 0;
}

int giac_cabi_gen_neq(GenHandle a, GenHandle b) {
    return (*G(a) != *G(b)) ? 1 : 0;
}

// ============================================================================
// Gen Internal
// ============================================================================

void* giac_cabi_gen_get_impl(GenHandle handle) {
    return G(handle)->get_impl();
}

GenHandle giac_cabi_gen_from_impl(void* impl) {
    CABI_TRY return new Gen(Gen::from_impl(impl)); CABI_CATCH_PTR
}

// ============================================================================
// Gen Construction (Feature 051)
// ============================================================================

GenHandle giac_cabi_make_identifier(const char* name) {
    CABI_TRY return new Gen(make_identifier(std::string(name))); CABI_CATCH_PTR
}

GenHandle giac_cabi_make_zint_from_bytes(const uint8_t* bytes, int32_t byte_count, int32_t sign) {
    CABI_TRY
    std::vector<uint8_t> vec(bytes, bytes + byte_count);
    return new Gen(make_zint_from_bytes(vec, sign));
    CABI_CATCH_PTR
}

GenHandle giac_cabi_make_symbolic_unevaluated(const char* op_name, GenHandle* args, int32_t count) {
    CABI_TRY
    std::vector<Gen> vec;
    vec.reserve(count);
    for (int32_t i = 0; i < count; ++i)
        vec.push_back(*G(args[i]));
    return new Gen(make_symbolic_unevaluated(std::string(op_name), vec));
    CABI_CATCH_PTR
}

GenHandle giac_cabi_make_complex(GenHandle re, GenHandle im) {
    CABI_TRY return new Gen(make_complex(*G(re), *G(im))); CABI_CATCH_PTR
}

GenHandle giac_cabi_make_fraction(GenHandle num, GenHandle den) {
    CABI_TRY return new Gen(make_fraction(*G(num), *G(den))); CABI_CATCH_PTR
}

GenHandle giac_cabi_make_vect(GenHandle* elements, int32_t count, int32_t subtype) {
    CABI_TRY
    std::vector<Gen> vec;
    vec.reserve(count);
    for (int32_t i = 0; i < count; ++i)
        vec.push_back(*G(elements[i]));
    return new Gen(make_vect(vec, subtype));
    CABI_CATCH_PTR
}

// ============================================================================
// Gen Pointer Management (Feature 051/052)
// ============================================================================

void* giac_cabi_gen_to_heap_ptr(GenHandle gen) {
    CABI_TRY return gen_to_heap_ptr(*G(gen)); CABI_CATCH_PTR
}

void giac_cabi_free_gen_ptr(void* ptr) {
    free_gen_ptr(ptr);
}

char* giac_cabi_gen_ptr_to_string(void* ptr) {
    CABI_TRY return strdup_malloc(gen_ptr_to_string(ptr)); CABI_CATCH_PTR
}

int giac_cabi_gen_ptr_type(void* ptr) {
    return gen_ptr_type(ptr);
}

GenHandle giac_cabi_gen_from_heap_ptr(void* ptr) {
    CABI_TRY return new Gen(gen_from_heap_ptr(ptr)); CABI_CATCH_PTR
}

// ============================================================================
// GiacContext
// ============================================================================

GiacContextHandle giac_cabi_context_create(void) {
    CABI_TRY return new GiacContext(); CABI_CATCH_PTR
}

void giac_cabi_context_destroy(GiacContextHandle handle) {
    delete CTX(handle);
}

char* giac_cabi_context_eval(GiacContextHandle handle, const char* input) {
    CABI_TRY return strdup_malloc(CTX(handle)->eval(std::string(input))); CABI_CATCH_PTR
}

void giac_cabi_context_set_variable(GiacContextHandle handle, const char* name, const char* value) {
    CABI_TRY CTX(handle)->set_variable(std::string(name), std::string(value)); CABI_CATCH_VOID
}

char* giac_cabi_context_get_variable(GiacContextHandle handle, const char* name) {
    CABI_TRY return strdup_malloc(CTX(handle)->get_variable(std::string(name))); CABI_CATCH_PTR
}

void giac_cabi_context_set_timeout(GiacContextHandle handle, double seconds) {
    CTX(handle)->set_timeout(seconds);
}

double giac_cabi_context_get_timeout(GiacContextHandle handle) {
    return CTX(handle)->get_timeout();
}

void giac_cabi_context_set_precision(GiacContextHandle handle, int digits) {
    CTX(handle)->set_precision(digits);
}

int giac_cabi_context_get_precision(GiacContextHandle handle) {
    return CTX(handle)->get_precision();
}

int giac_cabi_context_is_complex_mode(GiacContextHandle handle) {
    return CTX(handle)->is_complex_mode() ? 1 : 0;
}

void giac_cabi_context_set_complex_mode(GiacContextHandle handle, int enable) {
    CTX(handle)->set_complex_mode(enable != 0);
}

} // extern "C"
