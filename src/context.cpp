/**
 * @file context.cpp
 * @brief GiacContext implementation
 */

#include "giac_wrapper.h"
#include <giac/giac.h>

namespace giac_julia {

// ============================================================================
// GiacContext::Impl (Pimpl pattern)
// ============================================================================

class GiacContext::Impl {
public:
    giac::context ctx;
    int timeout_seconds = DEFAULT_TIMEOUT_SECONDS;
    WarningCallback warning_handler;

    Impl() : ctx() {
        // Set default timeout
        // GIAC uses a global variable for timeout, we track per-context
    }

    ~Impl() = default;
};

// ============================================================================
// GiacContext Implementation (T020-T028 - Implemented in Phase 3)
// ============================================================================

GiacContext::GiacContext() : impl_(std::make_unique<Impl>()) {
    // Context created with GIAC default settings
}

GiacContext::~GiacContext() = default;

GiacContext::GiacContext(GiacContext&&) noexcept = default;
GiacContext& GiacContext::operator=(GiacContext&&) noexcept = default;

std::string GiacContext::eval(const std::string& expression) {
    try {
        giac::gen result = giac::eval(giac::gen(expression, &impl_->ctx), &impl_->ctx);
        return result.print(&impl_->ctx);
    } catch (const std::exception& e) {
        throw GiacError(e.what());
    }
}

Gen GiacContext::eval_to_gen(const std::string& expression) {
    // Will be fully implemented in Phase 5 (T059)
    throw GiacError("eval_to_gen not yet implemented");
}

void GiacContext::set_variable(const std::string& name, const std::string& value) {
    // Will be implemented in Phase 4 (T033)
    try {
        std::string assignment = name + ":=" + value;
        giac::eval(giac::gen(assignment, &impl_->ctx), &impl_->ctx);
    } catch (const std::exception& e) {
        throw GiacError(e.what());
    }
}

std::string GiacContext::get_variable(const std::string& name) {
    // Will be implemented in Phase 4 (T034)
    try {
        giac::gen result = giac::eval(giac::gen(name, &impl_->ctx), &impl_->ctx);
        return result.print(&impl_->ctx);
    } catch (const std::exception& e) {
        return "";
    }
}

void GiacContext::set_timeout(int seconds) {
    impl_->timeout_seconds = seconds;
    // Note: GIAC timeout handling may require additional setup
}

int GiacContext::get_timeout() const {
    return impl_->timeout_seconds;
}

void GiacContext::set_warning_handler(WarningCallback handler) {
    impl_->warning_handler = std::move(handler);
}

void GiacContext::clear_warning_handler() {
    impl_->warning_handler = nullptr;
}

void GiacContext::set_precision(int digits) {
    // GIAC precision setting
    impl_->ctx.globalptr->_decimal_digits_ = digits;
}

int GiacContext::get_precision() const {
    return impl_->ctx.globalptr->_decimal_digits_;
}

bool GiacContext::is_complex_mode() const {
    return impl_->ctx.globalptr->_complex_mode_;
}

void GiacContext::set_complex_mode(bool enable) {
    impl_->ctx.globalptr->_complex_mode_ = enable;
}

} // namespace giac_julia
