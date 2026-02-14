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

namespace giac_julia {

// Forward declaration of opaque types
struct GiacContextImpl;
struct GenImpl;

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
    std::string type_name() const;

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
};

} // namespace giac_julia

#endif // GIAC_IMPL_H
