/**
 * @file giac_impl.cpp
 * @brief Implementation of GIAC wrapper, includes GIAC headers
 *
 * This translation unit includes GIAC headers and implements the
 * interface defined in giac_impl.h. It is kept separate from jlcxx
 * to avoid header conflicts.
 */

// Include GIAC headers first, before any potential conflicts
#include <giac/config.h>
#include <giac/giac.h>

#include "giac_impl.h"
#include <mutex>

namespace giac_julia {

// ============================================================================
// Implementation structs (hidden from header)
// ============================================================================

struct GiacContextImpl {
    giac::context ctx;
    std::function<void(const std::string&)> warning_handler;

    GiacContextImpl() : ctx() {}
};

struct GenImpl {
    giac::gen g;

    GenImpl() : g() {}
    explicit GenImpl(const giac::gen& gen) : g(gen) {}
    explicit GenImpl(giac::gen&& gen) : g(std::move(gen)) {}
};

// ============================================================================
// Library initialization
// ============================================================================

namespace {
    std::once_flag giac_init_flag;
    bool giac_initialized = false;

    void initialize_giac_library() {
        std::call_once(giac_init_flag, []() {
            giac_initialized = true;
        });
    }
}

// ============================================================================
// Version Functions
// ============================================================================

std::string get_giac_version() {
#ifdef GIAC_VERSION
    return GIAC_VERSION;
#elif defined(VERSION)
    return VERSION;
#else
    return "unknown";
#endif
}

std::string get_wrapper_version() {
    return "0.1.0";
}

bool check_giac_available() {
    initialize_giac_library();
    return giac_initialized;
}

// ============================================================================
// GiacContext Implementation
// ============================================================================

GiacContext::GiacContext() : impl_(std::make_unique<GiacContextImpl>()) {
    initialize_giac_library();
}

GiacContext::~GiacContext() = default;

GiacContext::GiacContext(GiacContext&& other) noexcept = default;
GiacContext& GiacContext::operator=(GiacContext&& other) noexcept = default;

std::string GiacContext::eval(const std::string& input) {
    try {
        giac::gen result = giac::eval(giac::gen(input, &impl_->ctx), &impl_->ctx);
        return result.print(&impl_->ctx);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("GIAC evaluation error: ") + e.what());
    }
}

void GiacContext::set_variable(const std::string& name, const std::string& value) {
    try {
        giac::gen val(value, &impl_->ctx);
        giac::sto(val, giac::gen(name, &impl_->ctx), &impl_->ctx);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to set variable: ") + e.what());
    }
}

std::string GiacContext::get_variable(const std::string& name) {
    try {
        giac::gen result = giac::eval(giac::gen(name, &impl_->ctx), &impl_->ctx);
        return result.print(&impl_->ctx);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to get variable: ") + e.what());
    }
}

void GiacContext::set_timeout(double seconds) {
    // Store timeout value (GIAC API varies by version)
    (void)seconds; // TODO: Implement when GIAC API is known
}

double GiacContext::get_timeout() const {
    return 0.0; // TODO: Implement when GIAC API is known
}

void GiacContext::set_precision(int digits) {
    // Store precision value (GIAC API varies by version)
    (void)digits; // TODO: Implement when GIAC API is known
}

int GiacContext::get_precision() const {
    return 15; // Default precision, TODO: Implement properly
}

bool GiacContext::is_complex_mode() const {
    return false; // TODO: Implement when GIAC API is known
}

void GiacContext::set_complex_mode(bool enable) {
    (void)enable; // TODO: Implement when GIAC API is known
}

void GiacContext::set_warning_handler(std::function<void(const std::string&)> handler) {
    impl_->warning_handler = std::move(handler);
}

void GiacContext::clear_warning_handler() {
    impl_->warning_handler = nullptr;
}

// ============================================================================
// Gen Implementation
// ============================================================================

Gen::Gen() : impl_(std::make_unique<GenImpl>()) {}

Gen::Gen(const std::string& expr) : impl_(std::make_unique<GenImpl>()) {
    giac::context ctx;
    impl_->g = giac::gen(expr, &ctx);
}

Gen::~Gen() = default;

Gen::Gen(const Gen& other) : impl_(std::make_unique<GenImpl>(other.impl_->g)) {}

Gen& Gen::operator=(const Gen& other) {
    if (this != &other) {
        impl_ = std::make_unique<GenImpl>(other.impl_->g);
    }
    return *this;
}

Gen::Gen(Gen&& other) noexcept = default;
Gen& Gen::operator=(Gen&& other) noexcept = default;

Gen::Gen(std::unique_ptr<GenImpl> impl) : impl_(std::move(impl)) {}

std::string Gen::to_string() const {
    giac::context ctx;
    return impl_->g.print(&ctx);
}

int Gen::type() const {
    return impl_->g.type;
}

std::string Gen::type_name() const {
    switch (impl_->g.type) {
        case giac::_INT_: return "integer";
        case giac::_DOUBLE_: return "double";
        case giac::_ZINT: return "bigint";
        case giac::_REAL: return "real";
        case giac::_CPLX: return "complex";
        case giac::_IDNT: return "identifier";
        case giac::_SYMB: return "symbolic";
        case giac::_VECT: return "vector";
        case giac::_POLY: return "polynomial";
        case giac::_FRAC: return "fraction";
        case giac::_STRNG: return "string";
        case giac::_FUNC: return "function";
        default: return "unknown";
    }
}

Gen Gen::eval() const {
    giac::context ctx;
    return Gen(std::make_unique<GenImpl>(giac::eval(impl_->g, &ctx)));
}

Gen Gen::simplify() const {
    giac::context ctx;
    return Gen(std::make_unique<GenImpl>(giac::simplify(impl_->g, &ctx)));
}

Gen Gen::expand() const {
    giac::context ctx;
    return Gen(std::make_unique<GenImpl>(giac::expand(impl_->g, &ctx)));
}

Gen Gen::factor() const {
    giac::context ctx;
    // Use _factor which is available in most GIAC versions
    giac::gen result = giac::eval(giac::symbolic(giac::at_factor, impl_->g), &ctx);
    return Gen(std::make_unique<GenImpl>(result));
}

Gen Gen::operator+(const Gen& other) const {
    return Gen(std::make_unique<GenImpl>(impl_->g + other.impl_->g));
}

Gen Gen::operator-(const Gen& other) const {
    return Gen(std::make_unique<GenImpl>(impl_->g - other.impl_->g));
}

Gen Gen::operator*(const Gen& other) const {
    return Gen(std::make_unique<GenImpl>(impl_->g * other.impl_->g));
}

Gen Gen::operator/(const Gen& other) const {
    return Gen(std::make_unique<GenImpl>(impl_->g / other.impl_->g));
}

Gen Gen::operator-() const {
    return Gen(std::make_unique<GenImpl>(-impl_->g));
}

bool Gen::operator==(const Gen& other) const {
    return impl_->g == other.impl_->g;
}

bool Gen::operator!=(const Gen& other) const {
    return impl_->g != other.impl_->g;
}

void* Gen::get_impl() const {
    return impl_.get();
}

Gen Gen::from_impl(void* impl) {
    auto* genImpl = static_cast<GenImpl*>(impl);
    return Gen(std::make_unique<GenImpl>(genImpl->g));
}

} // namespace giac_julia
