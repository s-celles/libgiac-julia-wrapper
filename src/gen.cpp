/**
 * @file gen.cpp
 * @brief Gen class implementation
 */

#include "giac_wrapper.h"
#include <giac/giac.h>

namespace giac_julia {

// ============================================================================
// Gen::Impl (Pimpl pattern) - T047
// ============================================================================

class Gen::Impl {
public:
    giac::gen value;
    giac::context* ctx_ptr;

    Impl(const giac::gen& g, giac::context* ctx)
        : value(g), ctx_ptr(ctx) {}

    Impl(const Impl& other)
        : value(other.value), ctx_ptr(other.ctx_ptr) {}

    ~Impl() = default;
};

// ============================================================================
// Gen Implementation (T047-T058 - Implemented in Phase 5)
// ============================================================================

Gen::Gen(const std::string& expression, GiacContext& context) {
    // This requires access to context's impl, will be properly implemented
    throw GiacError("Gen construction not yet fully implemented");
}

Gen::Gen(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}

Gen::Gen(const Gen& other) {
    if (other.impl_) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
}

Gen::Gen(Gen&& other) noexcept = default;

Gen::~Gen() = default;

Gen& Gen::operator=(const Gen& other) {
    if (this != &other && other.impl_) {
        impl_ = std::make_unique<Impl>(*other.impl_);
    }
    return *this;
}

Gen& Gen::operator=(Gen&& other) noexcept = default;

std::string Gen::to_string() const {
    if (!impl_) return "";
    return impl_->value.print(impl_->ctx_ptr);
}

int Gen::type() const {
    if (!impl_) return -1;
    return impl_->value.type;
}

std::string Gen::type_name() const {
    // Map GIAC type IDs to human-readable names
    switch (type()) {
        case 0: return "integer";
        case 1: return "double";
        case 2: return "rational";
        case 6: return "identifier";
        case 7: return "vector";
        case 8: return "symbolic";
        default: return "unknown";
    }
}

Gen Gen::eval() const {
    if (!impl_) throw GiacError("Invalid Gen object");
    auto result = giac::eval(impl_->value, impl_->ctx_ptr);
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::simplify() const {
    if (!impl_) throw GiacError("Invalid Gen object");
    auto result = giac::simplify(impl_->value, impl_->ctx_ptr);
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::expand() const {
    if (!impl_) throw GiacError("Invalid Gen object");
    auto result = giac::expand(impl_->value, impl_->ctx_ptr);
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::factor() const {
    if (!impl_) throw GiacError("Invalid Gen object");
    auto result = giac::factor(impl_->value, impl_->ctx_ptr);
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::operator+(const Gen& other) const {
    if (!impl_ || !other.impl_) throw GiacError("Invalid Gen object");
    auto result = impl_->value + other.impl_->value;
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::operator-(const Gen& other) const {
    if (!impl_ || !other.impl_) throw GiacError("Invalid Gen object");
    auto result = impl_->value - other.impl_->value;
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::operator*(const Gen& other) const {
    if (!impl_ || !other.impl_) throw GiacError("Invalid Gen object");
    auto result = impl_->value * other.impl_->value;
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::operator/(const Gen& other) const {
    if (!impl_ || !other.impl_) throw GiacError("Invalid Gen object");
    auto result = impl_->value / other.impl_->value;
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

Gen Gen::operator-() const {
    if (!impl_) throw GiacError("Invalid Gen object");
    auto result = -impl_->value;
    return Gen(std::make_unique<Impl>(result, impl_->ctx_ptr));
}

bool Gen::operator==(const Gen& other) const {
    if (!impl_ || !other.impl_) return false;
    return impl_->value == other.impl_->value;
}

bool Gen::operator!=(const Gen& other) const {
    return !(*this == other);
}

} // namespace giac_julia
