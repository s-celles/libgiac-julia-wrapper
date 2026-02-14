/**
 * @file giac_wrapper.cpp
 * @brief JLCXX module definition for Julia bindings
 *
 * This file only includes jlcxx headers and the opaque giac_impl.h
 * interface. It does NOT include GIAC headers directly to avoid
 * macro conflicts between GIAC and Julia.
 */

#include <jlcxx/jlcxx.hpp>
#include <jlcxx/stl.hpp>

#include "giac_impl.h"

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
    using namespace giac_julia;

    // Register version functions
    mod.method("giac_version", &get_giac_version);
    mod.method("wrapper_version", &get_wrapper_version);
    mod.method("is_giac_available", &check_giac_available);

    // Register configuration functions
    mod.method("set_xcasroot", &set_xcasroot);
    mod.method("get_xcasroot", &get_xcasroot);
    mod.method("init_help", &init_help);
    mod.method("list_commands", &list_commands);
    mod.method("help_count", &help_count);

    // Register GiacContext type
    mod.add_type<GiacContext>("GiacContext")
        .constructor<>()
        .method("giac_eval", &GiacContext::eval)
        .method("set_variable", &GiacContext::set_variable)
        .method("get_variable", &GiacContext::get_variable)
        .method("set_timeout", &GiacContext::set_timeout)
        .method("get_timeout", &GiacContext::get_timeout)
        .method("set_precision", &GiacContext::set_precision)
        .method("get_precision", &GiacContext::get_precision)
        .method("is_complex_mode", &GiacContext::is_complex_mode)
        .method("set_complex_mode", &GiacContext::set_complex_mode);

    // Register Gen type
    mod.add_type<Gen>("Gen")
        .constructor<>()
        .constructor<const std::string&>()
        .method("to_string", &Gen::to_string)
        .method("type", &Gen::type)
        .method("type_name", &Gen::type_name)
        .method("giac_eval", &Gen::eval)
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
