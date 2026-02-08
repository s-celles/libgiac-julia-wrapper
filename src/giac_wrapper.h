/**
 * @file giac_wrapper.h
 * @brief C++ API Contract for GIAC Julia Wrapper
 *
 * This header defines the public C++ API exposed to Julia via CxxWrap.jl.
 * Implementation details are hidden; this serves as the contract between
 * the C++ wrapper and the Julia binding layer.
 *
 * @version 0.1.0
 * @date 2026-02-08
 *
 * @section thread_safety Thread Safety (FR-007)
 *
 * This library is NOT thread-safe. The following guidelines apply:
 *
 * - Each GiacContext instance must be accessed from a single thread only
 * - Gen objects are associated with their originating context
 * - For concurrent computations, create separate GiacContext instances per thread
 * - The library initialization (via is_giac_available()) uses std::call_once
 *   and is thread-safe
 * - Warning callbacks are stored per-context (not thread-local)
 *
 * @par Example for multi-threaded usage:
 * @code
 * // Create one context per thread
 * std::vector<std::thread> threads;
 * for (int i = 0; i < num_threads; ++i) {
 *     threads.emplace_back([i]() {
 *         GiacContext ctx;  // Thread-local context
 *         std::string result = ctx.eval("factor(x^2-1)");
 *     });
 * }
 * for (auto& t : threads) t.join();
 * @endcode
 */

#ifndef GIAC_WRAPPER_H
#define GIAC_WRAPPER_H

#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

namespace giac_julia {

// Forward declarations
class Gen;

// Type aliases
using WarningCallback = std::function<void(const std::string&)>;

// Constants
constexpr int DEFAULT_TIMEOUT_SECONDS = 30;

/**
 * @brief Exception thrown for GIAC computation errors
 */
class GiacError : public std::runtime_error {
public:
    explicit GiacError(const std::string& message);

    /**
     * @brief Get the error category
     * @return Error category string (e.g., "ParseError", "TypeError")
     */
    std::string category() const;

private:
    std::string category_;
};

/**
 * @brief Manages GIAC computation context and state
 *
 * A context maintains variable bindings, computation settings, and mode
 * configurations. Each context is independent; variables assigned in one
 * context do not affect others.
 *
 * @note Not thread-safe. Use separate contexts for concurrent computations.
 *
 * @par Example:
 * @code
 * GiacContext ctx;
 * std::string result = ctx.eval("factor(x^2-1)");
 * // result == "(x-1)*(x+1)"
 * @endcode
 */
class GiacContext {
public:
    /**
     * @brief Construct a new GIAC context
     *
     * Initializes the GIAC library if not already initialized.
     * Creates an independent computation environment.
     *
     * @throws GiacError if library initialization fails
     */
    GiacContext();

    /**
     * @brief Destroy the context and release resources
     */
    ~GiacContext();

    // Non-copyable (context state is not trivially copyable)
    GiacContext(const GiacContext&) = delete;
    GiacContext& operator=(const GiacContext&) = delete;

    // Movable
    GiacContext(GiacContext&&) noexcept;
    GiacContext& operator=(GiacContext&&) noexcept;

    /**
     * @brief Evaluate a mathematical expression
     *
     * Parses and evaluates the given expression string, returning the
     * result as a string representation.
     *
     * @param expression Mathematical expression in GIAC syntax
     * @return Result as a string
     * @throws GiacError on parse error or evaluation failure
     *
     * @par Example:
     * @code
     * ctx.eval("1+1")           // returns "2"
     * ctx.eval("factor(x^2-1)") // returns "(x-1)*(x+1)"
     * ctx.eval("diff(x^3,x)")   // returns "3*x^2"
     * @endcode
     */
    std::string eval(const std::string& expression);

    /**
     * @brief Evaluate expression and return as Gen object
     *
     * Similar to eval() but returns a Gen object for further manipulation.
     *
     * @param expression Mathematical expression in GIAC syntax
     * @return Gen object representing the result
     * @throws GiacError on parse error or evaluation failure
     */
    Gen eval_to_gen(const std::string& expression);

    /**
     * @brief Assign a value to a variable
     *
     * Creates or updates a variable in this context's environment.
     *
     * @param name Variable name (must be valid identifier)
     * @param value Value to assign (expression string)
     * @throws GiacError if name is invalid or value cannot be parsed
     *
     * @par Example:
     * @code
     * ctx.set_variable("a", "5");
     * ctx.eval("a + 3");  // returns "8"
     * @endcode
     */
    void set_variable(const std::string& name, const std::string& value);

    /**
     * @brief Get the value of a variable
     *
     * @param name Variable name
     * @return Value as string, or empty string if undefined
     */
    std::string get_variable(const std::string& name);

    /**
     * @brief Set computation timeout
     *
     * @param seconds Maximum computation time (0 = no limit, default = 30)
     */
    void set_timeout(int seconds = DEFAULT_TIMEOUT_SECONDS);

    /**
     * @brief Get computation timeout
     * @return Current timeout in seconds (default: 30)
     */
    int get_timeout() const;

    /**
     * @brief Set warning message handler (FR-011)
     *
     * Registers a callback function to receive GIAC warning messages.
     * Warnings are non-fatal diagnostic messages that don't interrupt computation.
     *
     * @param handler Callback function receiving warning message string,
     *                or nullptr to disable warning capture
     *
     * @par Example:
     * @code
     * ctx.set_warning_handler([](const std::string& msg) {
     *     std::cerr << "GIAC warning: " << msg << std::endl;
     * });
     * @endcode
     */
    void set_warning_handler(WarningCallback handler);

    /**
     * @brief Clear the warning handler
     *
     * Equivalent to set_warning_handler(nullptr)
     */
    void clear_warning_handler();

    /**
     * @brief Set numeric precision for floating-point operations
     *
     * @param digits Number of significant digits
     */
    void set_precision(int digits);

    /**
     * @brief Get numeric precision
     * @return Current precision in digits
     */
    int get_precision() const;

    /**
     * @brief Check if context is in complex mode
     * @return true if complex numbers are enabled
     */
    bool is_complex_mode() const;

    /**
     * @brief Enable or disable complex mode
     * @param enable true to enable complex numbers
     */
    void set_complex_mode(bool enable);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Represents a GIAC mathematical expression
 *
 * Gen objects hold mathematical expressions that can be manipulated,
 * combined, and converted to strings. They maintain a reference to
 * their originating context.
 *
 * @note Not thread-safe. Associated context must remain valid.
 */
class Gen {
public:
    /**
     * @brief Construct Gen from string expression
     *
     * @param expression Expression to parse
     * @param context Context for parsing and evaluation
     * @throws GiacError on parse error
     */
    Gen(const std::string& expression, GiacContext& context);

    /**
     * @brief Copy constructor
     */
    Gen(const Gen& other);

    /**
     * @brief Move constructor
     */
    Gen(Gen&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~Gen();

    /**
     * @brief Copy assignment
     */
    Gen& operator=(const Gen& other);

    /**
     * @brief Move assignment
     */
    Gen& operator=(Gen&& other) noexcept;

    /**
     * @brief Convert to string representation
     * @return String representation of the expression
     */
    std::string to_string() const;

    /**
     * @brief Get the GIAC type identifier
     *
     * @return Type ID (see GIAC documentation for constants)
     *         Common values:
     *         - 0: Integer
     *         - 1: Double
     *         - 6: Identifier
     *         - 7: Vector
     *         - 8: Symbolic
     */
    int type() const;

    /**
     * @brief Get type as human-readable string
     * @return Type name (e.g., "integer", "polynomial", "symbolic")
     */
    std::string type_name() const;

    /**
     * @brief Evaluate the expression
     * @return New Gen with evaluated result
     */
    Gen eval() const;

    /**
     * @brief Simplify the expression
     * @return New Gen with simplified form
     */
    Gen simplify() const;

    /**
     * @brief Expand the expression
     * @return New Gen with expanded form
     */
    Gen expand() const;

    /**
     * @brief Factor the expression
     * @return New Gen with factored form
     */
    Gen factor() const;

    // Arithmetic operators (return new Gen objects)
    Gen operator+(const Gen& other) const;
    Gen operator-(const Gen& other) const;
    Gen operator*(const Gen& other) const;
    Gen operator/(const Gen& other) const;
    Gen operator-() const;  // Unary minus

    // Comparison (structural equality of expressions)
    bool operator==(const Gen& other) const;
    bool operator!=(const Gen& other) const;

private:
    friend class GiacContext;
    class Impl;
    std::unique_ptr<Impl> impl_;

    // Private constructor for internal use
    Gen(std::unique_ptr<Impl> impl);
};

// ============================================================================
// Free Functions (module-level operations)
// ============================================================================

/**
 * @brief Get GIAC library version
 * @return Version string (e.g., "1.9.0")
 */
std::string giac_version();

/**
 * @brief Get wrapper library version
 * @return Version string (e.g., "0.1.0")
 */
std::string wrapper_version();

/**
 * @brief Check if GIAC library is available
 * @return true if library was loaded successfully
 */
bool is_giac_available();

} // namespace giac_julia

#endif // GIAC_WRAPPER_H
