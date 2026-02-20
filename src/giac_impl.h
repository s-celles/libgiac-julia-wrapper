/**
 * @file giac_impl.h
 * @brief Pure C++ interface to GIAC, isolated from jlcxx
 *
 * This header provides a clean interface that doesn't require
 * including GIAC headers, avoiding macro conflicts with Julia/jlcxx.
 */

#ifndef GIAC_IMPL_H
#define GIAC_IMPL_H

#include <string>
#include <memory>
#include <functional>
#include <vector>

namespace giac_julia {

// Forward declaration of opaque types
struct GiacContextImpl;
struct GenImpl;
class Gen;  // Forward declaration for free functions

// ============================================================================
// Version Functions
// ============================================================================

std::string get_giac_version();
std::string get_wrapper_version();
bool check_giac_available();

// ============================================================================
// Configuration Functions
// ============================================================================

void set_xcasroot(const std::string& path);
std::string get_xcasroot();
bool init_help(const std::string& aide_cas_path);
std::string list_commands();
int help_count();

// ============================================================================
// Expression Evaluation
// ============================================================================

/**
 * @brief Parse and evaluate a Giac expression string
 * @param expr Expression string (e.g., "sin(x)+1", "[[1,2],[3,4]]")
 * @return Evaluated Gen
 * @note This is the preferred entry point for string expressions
 */
Gen giac_eval(const std::string& expr);

// ============================================================================
// Generic Dispatch (Tier 2)
// ============================================================================

/**
 * @brief Apply a Giac function by name with zero arguments
 * @param name Function name (e.g., "rand")
 * @return Result of function application
 */
Gen apply_func0(const std::string& name);

/**
 * @brief Apply a Giac function by name to a single argument
 * @param name Function name (e.g., "ifactor", "sin")
 * @param arg Argument Gen
 * @return Result of function application
 * @note Uses gen(name, &ctx) lookup; falls back to string eval if not _FUNC
 */
Gen apply_func1(const std::string& name, const Gen& arg);

/**
 * @brief Apply a Giac function by name to two arguments
 */
Gen apply_func2(const std::string& name, const Gen& arg1, const Gen& arg2);

/**
 * @brief Apply a Giac function by name to three arguments
 */
Gen apply_func3(const std::string& name, const Gen& arg1, const Gen& arg2, const Gen& arg3);

/**
 * @brief Apply a Giac function by name to N arguments
 * @param args Vector of arguments
 */
Gen apply_funcN(const std::string& name, const std::vector<Gen>& args);

// ============================================================================
// Function Listing
// ============================================================================

/**
 * @brief List all builtin functions from the lexer table
 * @return Newline-separated list of function names
 */
std::string list_builtin_functions();

/**
 * @brief Count of builtin lexer functions
 * @return Number of functions in builtin_lexer_functions table
 */
int builtin_function_count();

/**
 * @brief Union of documented commands and builtin functions
 * @return Newline-separated, deduplicated, sorted list of all function names
 * @note Requires init_help() to have been called for documented commands
 */
std::string list_all_functions();

// ============================================================================
// Tier 1 Direct Wrappers (High Performance - No Name Lookup)
// ============================================================================

// Trigonometry
Gen giac_sin(const Gen& arg);
Gen giac_cos(const Gen& arg);
Gen giac_tan(const Gen& arg);
Gen giac_asin(const Gen& arg);
Gen giac_acos(const Gen& arg);
Gen giac_atan(const Gen& arg);

// Exponential / Logarithm
Gen giac_exp(const Gen& arg);
Gen giac_ln(const Gen& arg);
Gen giac_log10(const Gen& arg);
Gen giac_sqrt(const Gen& arg);

// Arithmetic
Gen giac_abs(const Gen& arg);
Gen giac_sign(const Gen& arg);
Gen giac_floor(const Gen& arg);
Gen giac_ceil(const Gen& arg);

// Complex
Gen giac_re(const Gen& arg);
Gen giac_im(const Gen& arg);
Gen giac_conj(const Gen& arg);

// Algebra
Gen giac_normal(const Gen& arg);
Gen giac_evalf(const Gen& arg);

// Calculus (multi-argument)
Gen giac_diff(const Gen& expr, const Gen& var);
Gen giac_integrate(const Gen& expr, const Gen& var);
Gen giac_subst(const Gen& expr, const Gen& var, const Gen& val);
Gen giac_solve(const Gen& expr, const Gen& var);
Gen giac_limit(const Gen& expr, const Gen& var, const Gen& val);
Gen giac_series(const Gen& expr, const Gen& var, const Gen& order);

// Arithmetic (multi-argument)
Gen giac_gcd(const Gen& a, const Gen& b);
Gen giac_lcm(const Gen& a, const Gen& b);

// Power
Gen giac_pow(const Gen& base, const Gen& exp);

// ============================================================================
// GiacContext - Opaque wrapper around giac::context
// ============================================================================

class GiacContext {
public:
    GiacContext();
    ~GiacContext();

    // Non-copyable
    GiacContext(const GiacContext&) = delete;
    GiacContext& operator=(const GiacContext&) = delete;

    // Movable
    GiacContext(GiacContext&& other) noexcept;
    GiacContext& operator=(GiacContext&& other) noexcept;

    // Evaluation
    std::string eval(const std::string& input);

    // Variable management
    void set_variable(const std::string& name, const std::string& value);
    std::string get_variable(const std::string& name);

    // Configuration
    void set_timeout(double seconds);
    double get_timeout() const;
    void set_precision(int digits);
    int get_precision() const;
    bool is_complex_mode() const;
    void set_complex_mode(bool enable);

    // Warning handler
    void set_warning_handler(std::function<void(const std::string&)> handler);
    void clear_warning_handler();

private:
    std::unique_ptr<GiacContextImpl> impl_;
};

// ============================================================================
// Gen - Opaque wrapper around giac::gen
// ============================================================================

class Gen {
public:
    Gen();
    explicit Gen(const std::string& expr);
    explicit Gen(int64_t value);
    explicit Gen(double value);
    ~Gen();

    // Copyable
    Gen(const Gen& other);
    Gen& operator=(const Gen& other);

    // Movable
    Gen(Gen&& other) noexcept;
    Gen& operator=(Gen&& other) noexcept;

    // String conversion
    std::string to_string() const;

    // Type information
    int type() const;
    int32_t subtype() const;
    std::string type_name() const;

    // Typed accessors (caller must verify type() first!)
    // _INT_ type
    int64_t to_int64() const;
    int32_t to_int32() const;

    // _DOUBLE_ type
    double to_double() const;

    // _ZINT type (big integers)
    std::string zint_to_string() const;
    int zint_sign() const;
    std::vector<uint8_t> zint_to_bytes() const;

    // _CPLX type
    Gen cplx_re() const;
    Gen cplx_im() const;

    // _FRAC type
    Gen frac_num() const;
    Gen frac_den() const;

    // _VECT type
    int32_t vect_size() const;
    Gen vect_at(int32_t i) const;  // throws std::out_of_range if i out of bounds

    // _SYMB type
    std::string symb_sommet_name() const;
    Gen symb_feuille() const;

    // _IDNT type
    std::string idnt_name() const;

    // _STRNG type
    std::string strng_value() const;

    // _MAP type
    int32_t map_size() const;
    Gen map_keys() const;
    Gen map_values() const;

    // Predicates
    bool is_zero() const;
    bool is_one() const;
    bool is_integer() const;
    bool is_approx() const;

    // Type predicates (Feature 004: REQ-C10..C17)
    bool is_numeric() const;      // REQ-C11: _INT_, _DOUBLE_, _ZINT_, or _REAL_
    bool is_vector() const;       // REQ-C12: _VECT_
    bool is_symbolic() const;     // REQ-C13: _SYMB_
    bool is_identifier() const;   // REQ-C14: _IDNT_
    bool is_fraction() const;     // REQ-C15: _FRAC_
    bool is_complex() const;      // REQ-C16: _CPLX_
    bool is_string() const;       // REQ-C17: _STRNG_

    // Operations
    Gen eval() const;
    Gen simplify() const;
    Gen expand() const;
    Gen factor() const;

    // Arithmetic operators
    Gen operator+(const Gen& other) const;
    Gen operator-(const Gen& other) const;
    Gen operator*(const Gen& other) const;
    Gen operator/(const Gen& other) const;
    Gen operator-() const;

    // Comparison
    bool operator==(const Gen& other) const;
    bool operator!=(const Gen& other) const;

    // Internal use
    void* get_impl() const;
    static Gen from_impl(void* impl);

private:
    std::unique_ptr<GenImpl> impl_;
    explicit Gen(std::unique_ptr<GenImpl> impl);

    // Friend functions that need access to private constructor
    friend Gen giac_eval(const std::string& expr);
    friend Gen apply_func0(const std::string& name);
    friend Gen apply_func1(const std::string& name, const Gen& arg);
    friend Gen apply_func2(const std::string& name, const Gen& arg1, const Gen& arg2);
    friend Gen apply_func3(const std::string& name, const Gen& arg1, const Gen& arg2, const Gen& arg3);
    friend Gen apply_funcN(const std::string& name, const std::vector<Gen>& args);

    // Tier 1 direct wrapper friends (single-arg)
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

    // Tier 1 direct wrapper friends (multi-arg)
    friend Gen giac_diff(const Gen& expr, const Gen& var);
    friend Gen giac_integrate(const Gen& expr, const Gen& var);
    friend Gen giac_subst(const Gen& expr, const Gen& var, const Gen& val);
    friend Gen giac_solve(const Gen& expr, const Gen& var);
    friend Gen giac_limit(const Gen& expr, const Gen& var, const Gen& val);
    friend Gen giac_series(const Gen& expr, const Gen& var, const Gen& order);
    friend Gen giac_gcd(const Gen& a, const Gen& b);
    friend Gen giac_lcm(const Gen& a, const Gen& b);
    friend Gen giac_pow(const Gen& base, const Gen& exp);
};

} // namespace giac_julia

#endif // GIAC_IMPL_H
