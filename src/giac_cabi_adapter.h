/**
 * @file giac_cabi_adapter.h
 * @brief C++ adapter providing the same API as giac_impl.h via C-ABI calls
 *
 * When USE_GIAC_CABI is defined, giac_impl.h includes this header instead
 * of its normal declarations. This provides identical Gen, GiacContext
 * classes and free functions, but backed by extern "C" calls from
 * giac_cabi.h. This allows giac_wrapper.cpp to compile with clang++/libc++
 * while the real implementation (giac_impl.cpp) is compiled with
 * g++/libstdc++ to match GIAC's ABI.
 *
 * giac_wrapper.cpp does NOT need any modifications — it sees the same
 * public API regardless of whether USE_GIAC_CABI is defined.
 */

#ifndef GIAC_CABI_ADAPTER_H
#define GIAC_CABI_ADAPTER_H

#include "giac_cabi.h"

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

namespace giac_julia {

// Forward declarations
struct GiacContextImpl;  // Not used in adapter, but declared for API compat
struct GenImpl;           // Not used in adapter, but declared for API compat
class Gen;

// Helper: check C-ABI error and throw if needed
inline void cabi_check_error() {
    char buf[512];
    if (giac_cabi_get_last_error(buf, sizeof(buf))) {
        giac_cabi_clear_error();
        throw std::runtime_error(buf);
    }
}

// Helper: wrap malloc'd C string into std::string and free
inline std::string cabi_take_string(char* s) {
    if (!s) {
        cabi_check_error();
        return {};
    }
    std::string result(s);
    giac_cabi_free_string(s);
    return result;
}

// Helper: wrap GenHandle, check for errors
inline GenHandle cabi_check_handle(GenHandle h) {
    if (!h) cabi_check_error();
    return h;
}

// ============================================================================
// Version Functions
// ============================================================================

inline std::string get_giac_version() {
    return cabi_take_string(giac_cabi_get_giac_version());
}

inline std::string get_wrapper_version() {
    return cabi_take_string(giac_cabi_get_wrapper_version());
}

inline bool check_giac_available() {
    return giac_cabi_check_giac_available() != 0;
}

// ============================================================================
// Configuration Functions
// ============================================================================

inline void set_xcasroot(const std::string& path) {
    giac_cabi_set_xcasroot(path.c_str());
}

inline std::string get_xcasroot() {
    return cabi_take_string(giac_cabi_get_xcasroot());
}

inline bool init_help(const std::string& aide_cas_path) {
    return giac_cabi_init_help(aide_cas_path.c_str()) != 0;
}

inline std::string list_commands() {
    return cabi_take_string(giac_cabi_list_commands());
}

inline int help_count() {
    return giac_cabi_help_count();
}

// ============================================================================
// Function Listing
// ============================================================================

inline std::string list_builtin_functions() {
    return cabi_take_string(giac_cabi_list_builtin_functions());
}

inline int builtin_function_count() {
    return giac_cabi_builtin_function_count();
}

inline std::string list_all_functions() {
    return cabi_take_string(giac_cabi_list_all_functions());
}

// ============================================================================
// Gen class — adapter backed by GenHandle
// ============================================================================

class Gen {
public:
    Gen() : handle_(cabi_check_handle(giac_cabi_gen_create_default())) {}

    explicit Gen(const std::string& expr)
        : handle_(cabi_check_handle(giac_cabi_gen_create_from_string(expr.c_str()))) {}

    explicit Gen(int64_t value)
        : handle_(cabi_check_handle(giac_cabi_gen_create_from_int64(value))) {}

    explicit Gen(double value)
        : handle_(cabi_check_handle(giac_cabi_gen_create_from_double(value))) {}

    ~Gen() {
        if (handle_) giac_cabi_gen_destroy(handle_);
    }

    // Copy
    Gen(const Gen& other) : handle_(cabi_check_handle(giac_cabi_gen_copy(other.handle_))) {}
    Gen& operator=(const Gen& other) {
        if (this != &other) {
            if (handle_) giac_cabi_gen_destroy(handle_);
            handle_ = cabi_check_handle(giac_cabi_gen_copy(other.handle_));
        }
        return *this;
    }

    // Move
    Gen(Gen&& other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
    Gen& operator=(Gen&& other) noexcept {
        if (this != &other) {
            if (handle_) giac_cabi_gen_destroy(handle_);
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    // String conversion
    std::string to_string() const {
        return cabi_take_string(giac_cabi_gen_to_string(handle_));
    }

    // Type information
    int type() const { return giac_cabi_gen_type(handle_); }
    int32_t subtype() const { return giac_cabi_gen_subtype(handle_); }
    std::string type_name() const {
        return cabi_take_string(giac_cabi_gen_type_name(handle_));
    }

    // Typed accessors
    int64_t to_int64() const {
        int64_t r = giac_cabi_gen_to_int64(handle_);
        cabi_check_error();
        return r;
    }

    int32_t to_int32() const {
        int32_t r = giac_cabi_gen_to_int32(handle_);
        cabi_check_error();
        return r;
    }

    double to_double() const {
        double r = giac_cabi_gen_to_double(handle_);
        cabi_check_error();
        return r;
    }

    std::string zint_to_string() const {
        return cabi_take_string(giac_cabi_gen_zint_to_string(handle_));
    }

    int zint_sign() const {
        int r = giac_cabi_gen_zint_sign(handle_);
        cabi_check_error();
        return r;
    }

    std::vector<uint8_t> zint_to_bytes() const {
        int32_t size = giac_cabi_gen_zint_to_bytes(handle_, nullptr, 0);
        cabi_check_error();
        if (size == 0) return {};
        std::vector<uint8_t> result(size);
        giac_cabi_gen_zint_to_bytes(handle_, result.data(), size);
        return result;
    }

    // Complex
    Gen cplx_re() const { return Gen(cabi_check_handle(giac_cabi_gen_cplx_re(handle_))); }
    Gen cplx_im() const { return Gen(cabi_check_handle(giac_cabi_gen_cplx_im(handle_))); }

    // Fraction
    Gen frac_num() const { return Gen(cabi_check_handle(giac_cabi_gen_frac_num(handle_))); }
    Gen frac_den() const { return Gen(cabi_check_handle(giac_cabi_gen_frac_den(handle_))); }

    // Vector
    int32_t vect_size() const {
        int32_t r = giac_cabi_gen_vect_size(handle_);
        cabi_check_error();
        return r;
    }
    Gen vect_at(int32_t i) const {
        return Gen(cabi_check_handle(giac_cabi_gen_vect_at(handle_, i)));
    }

    // Symbolic
    std::string symb_sommet_name() const {
        return cabi_take_string(giac_cabi_gen_symb_sommet_name(handle_));
    }
    Gen symb_feuille() const {
        return Gen(cabi_check_handle(giac_cabi_gen_symb_feuille(handle_)));
    }

    // Identifier
    std::string idnt_name() const {
        return cabi_take_string(giac_cabi_gen_idnt_name(handle_));
    }

    // String
    std::string strng_value() const {
        return cabi_take_string(giac_cabi_gen_strng_value(handle_));
    }

    // Map
    int32_t map_size() const {
        int32_t r = giac_cabi_gen_map_size(handle_);
        cabi_check_error();
        return r;
    }
    Gen map_keys() const { return Gen(cabi_check_handle(giac_cabi_gen_map_keys(handle_))); }
    Gen map_values() const { return Gen(cabi_check_handle(giac_cabi_gen_map_values(handle_))); }

    // Predicates
    bool is_zero() const { return giac_cabi_gen_is_zero(handle_) != 0; }
    bool is_one() const { return giac_cabi_gen_is_one(handle_) != 0; }
    bool is_integer() const { return giac_cabi_gen_is_integer(handle_) != 0; }
    bool is_approx() const { return giac_cabi_gen_is_approx(handle_) != 0; }
    bool is_numeric() const { return giac_cabi_gen_is_numeric(handle_) != 0; }
    bool is_vector() const { return giac_cabi_gen_is_vector(handle_) != 0; }
    bool is_symbolic() const { return giac_cabi_gen_is_symbolic(handle_) != 0; }
    bool is_identifier() const { return giac_cabi_gen_is_identifier(handle_) != 0; }
    bool is_fraction() const { return giac_cabi_gen_is_fraction(handle_) != 0; }
    bool is_complex() const { return giac_cabi_gen_is_complex(handle_) != 0; }
    bool is_string() const { return giac_cabi_gen_is_string(handle_) != 0; }

    // Operations
    Gen eval() const { return Gen(cabi_check_handle(giac_cabi_gen_eval(handle_))); }
    Gen simplify() const { return Gen(cabi_check_handle(giac_cabi_gen_simplify(handle_))); }
    Gen expand() const { return Gen(cabi_check_handle(giac_cabi_gen_expand(handle_))); }
    Gen factor() const { return Gen(cabi_check_handle(giac_cabi_gen_factor(handle_))); }

    // Arithmetic
    Gen operator+(const Gen& o) const { return Gen(cabi_check_handle(giac_cabi_gen_add(handle_, o.handle_))); }
    Gen operator-(const Gen& o) const { return Gen(cabi_check_handle(giac_cabi_gen_sub(handle_, o.handle_))); }
    Gen operator*(const Gen& o) const { return Gen(cabi_check_handle(giac_cabi_gen_mul(handle_, o.handle_))); }
    Gen operator/(const Gen& o) const { return Gen(cabi_check_handle(giac_cabi_gen_div(handle_, o.handle_))); }
    Gen operator-() const { return Gen(cabi_check_handle(giac_cabi_gen_neg(handle_))); }
    bool operator==(const Gen& o) const { return giac_cabi_gen_eq(handle_, o.handle_) != 0; }
    bool operator!=(const Gen& o) const { return giac_cabi_gen_neq(handle_, o.handle_) != 0; }

    // Internal
    void* get_impl() const { return giac_cabi_gen_get_impl(handle_); }
    static Gen from_impl(void* impl) {
        return Gen(cabi_check_handle(giac_cabi_gen_from_impl(impl)));
    }

private:
    GenHandle handle_;

    // Private: construct from raw handle (takes ownership)
    explicit Gen(GenHandle h) : handle_(h) {}

    // Allow all friend free functions to use GenHandle and private constructor
    friend Gen giac_eval(const std::string& expr);
    friend Gen apply_func0(const std::string& name);
    friend Gen apply_func1(const std::string& name, const Gen& arg);
    friend Gen apply_func2(const std::string& name, const Gen& arg1, const Gen& arg2);
    friend Gen apply_func3(const std::string& name, const Gen& arg1, const Gen& arg2, const Gen& arg3);
    friend Gen apply_funcN(const std::string& name, const std::vector<Gen>& args);
    friend Gen giac_sin(const Gen& arg);
    friend Gen giac_cos(const Gen& arg);
    friend Gen giac_tan(const Gen& arg);
    friend Gen giac_asin(const Gen& arg);
    friend Gen giac_acos(const Gen& arg);
    friend Gen giac_atan(const Gen& arg);
    friend Gen giac_exp(const Gen& arg);
    friend Gen giac_ln(const Gen& arg);
    friend Gen giac_log10(const Gen& arg);
    friend Gen giac_sqrt(const Gen& arg);
    friend Gen giac_abs(const Gen& arg);
    friend Gen giac_sign(const Gen& arg);
    friend Gen giac_floor(const Gen& arg);
    friend Gen giac_ceil(const Gen& arg);
    friend Gen giac_re(const Gen& arg);
    friend Gen giac_im(const Gen& arg);
    friend Gen giac_conj(const Gen& arg);
    friend Gen giac_normal(const Gen& arg);
    friend Gen giac_evalf(const Gen& arg);
    friend Gen giac_diff(const Gen& expr, const Gen& var);
    friend Gen giac_integrate(const Gen& expr, const Gen& var);
    friend Gen giac_subst(const Gen& expr, const Gen& var, const Gen& val);
    friend Gen giac_solve(const Gen& expr, const Gen& var);
    friend Gen giac_limit(const Gen& expr, const Gen& var, const Gen& val);
    friend Gen giac_series(const Gen& expr, const Gen& var, const Gen& order);
    friend Gen giac_gcd(const Gen& a, const Gen& b);
    friend Gen giac_lcm(const Gen& a, const Gen& b);
    friend Gen giac_pow(const Gen& base, const Gen& exp);
    friend Gen make_identifier(const std::string& name);
    friend Gen make_zint_from_bytes(const std::vector<uint8_t>& bytes, int32_t sign);
    friend Gen make_symbolic_unevaluated(const std::string& op_name, const std::vector<Gen>& args);
    friend Gen make_complex(const Gen& re, const Gen& im);
    friend Gen make_fraction(const Gen& num, const Gen& den);
    friend Gen make_vect(const std::vector<Gen>& elements, int32_t subtype);
    friend void* gen_to_heap_ptr(const Gen& gen);
    friend Gen gen_from_heap_ptr(void* ptr);
};

// ============================================================================
// Expression Evaluation
// ============================================================================

inline Gen giac_eval(const std::string& expr) {
    return Gen(cabi_check_handle(giac_cabi_giac_eval(expr.c_str())));
}

// ============================================================================
// Generic Dispatch
// ============================================================================

inline Gen apply_func0(const std::string& name) {
    return Gen(cabi_check_handle(giac_cabi_apply_func0(name.c_str())));
}

inline Gen apply_func1(const std::string& name, const Gen& arg) {
    return Gen(cabi_check_handle(giac_cabi_apply_func1(name.c_str(), arg.handle_)));
}

inline Gen apply_func2(const std::string& name, const Gen& arg1, const Gen& arg2) {
    return Gen(cabi_check_handle(giac_cabi_apply_func2(name.c_str(), arg1.handle_, arg2.handle_)));
}

inline Gen apply_func3(const std::string& name, const Gen& arg1, const Gen& arg2, const Gen& arg3) {
    return Gen(cabi_check_handle(giac_cabi_apply_func3(name.c_str(), arg1.handle_, arg2.handle_, arg3.handle_)));
}

inline Gen apply_funcN(const std::string& name, const std::vector<Gen>& args) {
    std::vector<GenHandle> handles(args.size());
    for (size_t i = 0; i < args.size(); ++i)
        handles[i] = args[i].handle_;
    return Gen(cabi_check_handle(
        giac_cabi_apply_funcN(name.c_str(), handles.data(), static_cast<int32_t>(args.size()))));
}

// ============================================================================
// Tier 1 Direct Wrappers
// ============================================================================

#define ADAPTER_TIER1_1(name) \
    inline Gen giac_##name(const Gen& arg) { \
        return Gen(cabi_check_handle(giac_cabi_##name(arg.handle_))); \
    }

#define ADAPTER_TIER1_2(name) \
    inline Gen giac_##name(const Gen& a, const Gen& b) { \
        return Gen(cabi_check_handle(giac_cabi_##name(a.handle_, b.handle_))); \
    }

#define ADAPTER_TIER1_3(name) \
    inline Gen giac_##name(const Gen& a, const Gen& b, const Gen& c) { \
        return Gen(cabi_check_handle(giac_cabi_##name(a.handle_, b.handle_, c.handle_))); \
    }

ADAPTER_TIER1_1(sin)
ADAPTER_TIER1_1(cos)
ADAPTER_TIER1_1(tan)
ADAPTER_TIER1_1(asin)
ADAPTER_TIER1_1(acos)
ADAPTER_TIER1_1(atan)
ADAPTER_TIER1_1(exp)
ADAPTER_TIER1_1(ln)
ADAPTER_TIER1_1(log10)
ADAPTER_TIER1_1(sqrt)
ADAPTER_TIER1_1(abs)
ADAPTER_TIER1_1(sign)
ADAPTER_TIER1_1(floor)
ADAPTER_TIER1_1(ceil)
ADAPTER_TIER1_1(re)
ADAPTER_TIER1_1(im)
ADAPTER_TIER1_1(conj)
ADAPTER_TIER1_1(normal)
ADAPTER_TIER1_1(evalf)

ADAPTER_TIER1_2(diff)
ADAPTER_TIER1_2(integrate)
ADAPTER_TIER1_2(solve)
ADAPTER_TIER1_2(gcd)
ADAPTER_TIER1_2(lcm)
ADAPTER_TIER1_2(pow)

ADAPTER_TIER1_3(subst)
ADAPTER_TIER1_3(limit)
ADAPTER_TIER1_3(series)

#undef ADAPTER_TIER1_1
#undef ADAPTER_TIER1_2
#undef ADAPTER_TIER1_3

// ============================================================================
// Gen Construction Functions (Feature 051)
// ============================================================================

inline Gen make_identifier(const std::string& name) {
    return Gen(cabi_check_handle(giac_cabi_make_identifier(name.c_str())));
}

inline Gen make_zint_from_bytes(const std::vector<uint8_t>& bytes, int32_t sign) {
    return Gen(cabi_check_handle(
        giac_cabi_make_zint_from_bytes(bytes.data(), static_cast<int32_t>(bytes.size()), sign)));
}

inline Gen make_symbolic_unevaluated(const std::string& op_name, const std::vector<Gen>& args) {
    std::vector<GenHandle> handles(args.size());
    for (size_t i = 0; i < args.size(); ++i)
        handles[i] = args[i].handle_;
    return Gen(cabi_check_handle(
        giac_cabi_make_symbolic_unevaluated(op_name.c_str(), handles.data(),
                                             static_cast<int32_t>(args.size()))));
}

inline Gen make_complex(const Gen& re, const Gen& im) {
    return Gen(cabi_check_handle(giac_cabi_make_complex(re.handle_, im.handle_)));
}

inline Gen make_fraction(const Gen& num, const Gen& den) {
    return Gen(cabi_check_handle(giac_cabi_make_fraction(num.handle_, den.handle_)));
}

inline Gen make_vect(const std::vector<Gen>& elements, int32_t subtype) {
    std::vector<GenHandle> handles(elements.size());
    for (size_t i = 0; i < elements.size(); ++i)
        handles[i] = elements[i].handle_;
    return Gen(cabi_check_handle(
        giac_cabi_make_vect(handles.data(), static_cast<int32_t>(elements.size()), subtype)));
}

// ============================================================================
// Gen Pointer Management (Feature 051/052)
// ============================================================================

inline void* gen_to_heap_ptr(const Gen& gen) {
    return giac_cabi_gen_to_heap_ptr(gen.handle_);
}

inline void free_gen_ptr(void* ptr) {
    giac_cabi_free_gen_ptr(ptr);
}

inline std::string gen_ptr_to_string(void* ptr) {
    return cabi_take_string(giac_cabi_gen_ptr_to_string(ptr));
}

inline int gen_ptr_type(void* ptr) {
    return giac_cabi_gen_ptr_type(ptr);
}

inline Gen gen_from_heap_ptr(void* ptr) {
    return Gen(cabi_check_handle(giac_cabi_gen_from_heap_ptr(ptr)));
}

// ============================================================================
// GiacContext class — adapter backed by GiacContextHandle
// ============================================================================

class GiacContext {
public:
    GiacContext() : handle_(giac_cabi_context_create()) {
        if (!handle_) cabi_check_error();
    }

    ~GiacContext() {
        if (handle_) giac_cabi_context_destroy(handle_);
    }

    // Non-copyable
    GiacContext(const GiacContext&) = delete;
    GiacContext& operator=(const GiacContext&) = delete;

    // Movable
    GiacContext(GiacContext&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }
    GiacContext& operator=(GiacContext&& other) noexcept {
        if (this != &other) {
            if (handle_) giac_cabi_context_destroy(handle_);
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    std::string eval(const std::string& input) {
        return cabi_take_string(giac_cabi_context_eval(handle_, input.c_str()));
    }

    void set_variable(const std::string& name, const std::string& value) {
        giac_cabi_context_set_variable(handle_, name.c_str(), value.c_str());
        cabi_check_error();
    }

    std::string get_variable(const std::string& name) {
        return cabi_take_string(giac_cabi_context_get_variable(handle_, name.c_str()));
    }

    void set_timeout(double seconds) { giac_cabi_context_set_timeout(handle_, seconds); }
    double get_timeout() const { return giac_cabi_context_get_timeout(handle_); }
    void set_precision(int digits) { giac_cabi_context_set_precision(handle_, digits); }
    int get_precision() const { return giac_cabi_context_get_precision(handle_); }
    bool is_complex_mode() const { return giac_cabi_context_is_complex_mode(handle_) != 0; }
    void set_complex_mode(bool enable) { giac_cabi_context_set_complex_mode(handle_, enable ? 1 : 0); }

    // Not bridged across ABI (not exposed in giac_wrapper.cpp)
    void set_warning_handler(std::function<void(const std::string&)>) {}
    void clear_warning_handler() {}

private:
    GiacContextHandle handle_;
};

} // namespace giac_julia

#endif // GIAC_CABI_ADAPTER_H
