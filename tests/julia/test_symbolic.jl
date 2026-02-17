# test_symbolic.jl
# Tests for symbolic expression access (Feature 004, User Story 7)
#
# Tests for REQ-C60..C62: Symbolic expression structure introspection

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

@testset "Symbolic Expression Access Tests" begin
    @testset "symb_sommet_name() - Function name extraction" begin
        # Basic function names
        g_sin = giac_eval("sin(x)")
        @test contains(symb_sommet_name(g_sin), "sin")

        g_cos = giac_eval("cos(x)")
        @test contains(symb_sommet_name(g_cos), "cos")

        g_exp = giac_eval("exp(x)")
        @test contains(symb_sommet_name(g_exp), "exp")

        g_ln = giac_eval("ln(x)")
        @test contains(symb_sommet_name(g_ln), "ln")
    end

    @testset "symb_sommet_name() - Operator names" begin
        # Addition
        g_add = giac_eval("x + y")
        name_add = symb_sommet_name(g_add)
        @test contains(name_add, "+") || contains(name_add, "plus")

        # Multiplication
        g_mul = giac_eval("x * y")
        name_mul = symb_sommet_name(g_mul)
        @test contains(name_mul, "*") || contains(name_mul, "prod")

        # Power
        g_pow = giac_eval("x^2")
        name_pow = symb_sommet_name(g_pow)
        @test contains(name_pow, "^") || contains(name_pow, "pow")
    end

    @testset "symb_feuille() - Argument extraction" begin
        # Single argument function
        g_sin = giac_eval("sin(x)")
        arg = symb_feuille(g_sin)
        @test to_string(arg) == "x"

        # Nested argument
        g_nested = giac_eval("sin(x^2)")
        arg_nested = symb_feuille(g_nested)
        @test to_string(arg_nested) == "x^2" || to_string(arg_nested) == "x²"
    end

    @testset "symb_feuille() - Multiple arguments" begin
        # Addition has multiple arguments (as a vector)
        g_add = giac_eval("x + y + z")
        args = symb_feuille(g_add)
        # Arguments are typically returned as a vector for multi-arg ops
        @test is_vector(args) || type(args) == GENTYPE_IDNT
    end

    @testset "symb_sommet_name() throws on non-symbolic" begin
        g_int = giac_eval("42")
        @test_throws Exception symb_sommet_name(g_int)

        g_frac = giac_eval("3/7")
        @test_throws Exception symb_sommet_name(g_frac)

        g_vect = giac_eval("[1, 2, 3]")
        @test_throws Exception symb_sommet_name(g_vect)

        g_idnt = giac_eval("x")
        @test_throws Exception symb_sommet_name(g_idnt)
    end

    @testset "symb_feuille() throws on non-symbolic" begin
        g_int = giac_eval("42")
        @test_throws Exception symb_feuille(g_int)

        g_double = giac_eval("3.14")
        @test_throws Exception symb_feuille(g_double)

        g_str = giac_eval("\"hello\"")
        @test_throws Exception symb_feuille(g_str)
    end

    @testset "Nested symbolic expressions" begin
        # sin(cos(x))
        g = giac_eval("sin(cos(x))")
        @test contains(symb_sommet_name(g), "sin")

        inner = symb_feuille(g)
        @test is_symbolic(inner)
        @test contains(symb_sommet_name(inner), "cos")

        innermost = symb_feuille(inner)
        @test to_string(innermost) == "x"
    end

    @testset "Complex expressions" begin
        # x^2 + 2*x + 1
        g = giac_eval("x^2 + 2*x + 1")
        @test is_symbolic(g)

        # The top-level is addition
        name = symb_sommet_name(g)
        @test contains(name, "+") || contains(name, "plus")
    end

    @testset "is_symbolic() predicate" begin
        @test is_symbolic(giac_eval("sin(x)"))
        @test is_symbolic(giac_eval("x + 1"))
        @test is_symbolic(giac_eval("x^2"))

        @test !is_symbolic(giac_eval("42"))
        @test !is_symbolic(giac_eval("x"))  # identifier, not symbolic
        @test !is_symbolic(giac_eval("[1, 2]"))
        @test !is_symbolic(giac_eval("3/7"))
    end

    @testset "Symbolic tree traversal" begin
        # Build and traverse: (x + 1) * (x - 1)
        g = giac_eval("(x + 1) * (x - 1)")

        # Top level is multiplication
        @test is_symbolic(g)
        top_name = symb_sommet_name(g)
        @test contains(top_name, "*") || contains(top_name, "prod")

        # Get the arguments (should be a vector of two terms)
        args = symb_feuille(g)
        if is_vector(args)
            @test vect_size(args) == 2
        end
    end
end
