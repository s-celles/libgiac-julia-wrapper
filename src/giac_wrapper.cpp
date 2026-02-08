/**
 * @file giac_wrapper.cpp
 * @brief Main wrapper implementation and JLCXX_MODULE definition
 */

#include "giac_wrapper.h"
#include <giac/giac.h>
#include <jlcxx/jlcxx.hpp>
#include <jlcxx/stl.hpp>
#include <mutex>

namespace giac_julia {

// ============================================================================
// GiacError Implementation (T008)
// ============================================================================

GiacError::GiacError(const std::string& message)
    : std::runtime_error(message), category_("GiacError") {}

std::string GiacError::category() const {
    return category_;
}

// ============================================================================
// Library Initialization (T009)
// ============================================================================

namespace {
    std::once_flag giac_init_flag;
    bool giac_initialized = false;

    void initialize_giac_library() {
        std::call_once(giac_init_flag, []() {
            // GIAC library initialization
            // The context constructor handles per-context init
            giac_initialized = true;
        });
    }
}

// ============================================================================
// Free Functions (T010, T011, T012)
// ============================================================================

std::string giac_version() {
    return GIAC_VERSION;
}

std::string wrapper_version() {
    return "0.1.0";
}

bool is_giac_available() {
    initialize_giac_library();
    return giac_initialized;
}

// ============================================================================
// JLCXX_MODULE Definition (T013)
// ============================================================================

} // namespace giac_julia

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
    using namespace giac_julia;

    // Ensure GIAC is initialized
    initialize_giac_library();

    // Register version functions
    mod.method("giac_version", &giac_version);
    mod.method("wrapper_version", &wrapper_version);
    mod.method("is_giac_available", &is_giac_available);

    // Register GiacContext type (T025)
    mod.add_type<GiacContext>("GiacContext")
        .constructor<>()
        // Register eval method (T026)
        .method("eval", &GiacContext::eval)
        .method("eval_to_gen", &GiacContext::eval_to_gen)
        // Context management methods
        .method("set_variable", &GiacContext::set_variable)
        .method("get_variable", &GiacContext::get_variable)
        .method("set_timeout", &GiacContext::set_timeout)
        .method("get_timeout", &GiacContext::get_timeout)
        .method("set_warning_handler", &GiacContext::set_warning_handler)
        .method("clear_warning_handler", &GiacContext::clear_warning_handler)
        .method("set_precision", &GiacContext::set_precision)
        .method("get_precision", &GiacContext::get_precision)
        .method("is_complex_mode", &GiacContext::is_complex_mode)
        .method("set_complex_mode", &GiacContext::set_complex_mode);

    // Register Gen type (T060)
    mod.add_type<Gen>("Gen")
        // Register Gen methods (T061)
        .method("to_string", &Gen::to_string)
        .method("type", &Gen::type)
        .method("type_name", &Gen::type_name)
        .method("eval", &Gen::eval)
        .method("simplify", &Gen::simplify)
        .method("expand", &Gen::expand)
        .method("factor", &Gen::factor);

    // Register Gen operators
    mod.set_override_module(jl_base_module);
    mod.method("+", [](const Gen& a, const Gen& b) { return a + b; });
    mod.method("-", [](const Gen& a, const Gen& b) { return a - b; });
    mod.method("*", [](const Gen& a, const Gen& b) { return a * b; });
    mod.method("/", [](const Gen& a, const Gen& b) { return a / b; });
    mod.method("-", [](const Gen& a) { return -a; });
    mod.method("==", [](const Gen& a, const Gen& b) { return a == b; });
    mod.method("!=", [](const Gen& a, const Gen& b) { return a != b; });
    mod.unset_override_module();
}
