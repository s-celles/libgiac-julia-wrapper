# test_introspection.jl
# Tests for Gen type introspection (Feature 004, User Story 2)
#
# Tests for REQ-C01..C17: Type introspection and predicates

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

@testset "Type Introspection Tests" begin
    @testset "Type Constants" begin
        # Verify type constants are accessible
        @test GENTYPE_INT == 0
        @test GENTYPE_DOUBLE == 1
        @test GENTYPE_ZINT == 2
        @test GENTYPE_CPLX == 4
        @test GENTYPE_VECT == 7
        @test GENTYPE_SYMB == 8
        @test GENTYPE_FRAC == 10
        @test GENTYPE_STRNG == 12
    end

    @testset "Vector Subtype Constants" begin
        @test VECTSUBTYPE_LIST == 0
        @test VECTSUBTYPE_SET == 2
        @test VECTSUBTYPE_MATRIX == 11
    end

    @testset "type() method" begin
        # Integer
        g_int = giac_eval("42")
        @test type(g_int) == GENTYPE_INT

        # Double
        g_double = giac_eval("3.14")
        @test type(g_double) == GENTYPE_DOUBLE

        # Fraction
        g_frac = giac_eval("3/7")
        @test type(g_frac) == GENTYPE_FRAC

        # Vector
        g_vect = giac_eval("[1, 2, 3]")
        @test type(g_vect) == GENTYPE_VECT

        # Complex
        g_cplx = giac_eval("2+3*i")
        @test type(g_cplx) == GENTYPE_CPLX

        # Symbolic
        g_symb = giac_eval("sin(x)")
        @test type(g_symb) == GENTYPE_SYMB
    end

    @testset "is_integer()" begin
        @test is_integer(giac_eval("42")) == true
        @test is_integer(giac_eval("factorial(100)")) == true  # BigInt
        @test is_integer(giac_eval("3.14")) == false
        @test is_integer(giac_eval("3/7")) == false
    end

    @testset "is_numeric()" begin
        @test is_numeric(giac_eval("42")) == true
        @test is_numeric(giac_eval("3.14")) == true
        @test is_numeric(giac_eval("factorial(100)")) == true  # BigInt
        @test is_numeric(giac_eval("sin(x)")) == false
        @test is_numeric(giac_eval("3/7")) == false  # Fraction is _FRAC_, not numeric
    end

    @testset "is_vector()" begin
        @test is_vector(giac_eval("[1, 2, 3]")) == true
        @test is_vector(giac_eval("[[1,2],[3,4]]")) == true  # Matrix is also a vector
        @test is_vector(giac_eval("42")) == false
        @test is_vector(giac_eval("sin(x)")) == false
    end

    @testset "is_symbolic()" begin
        @test is_symbolic(giac_eval("sin(x)")) == true
        @test is_symbolic(giac_eval("x + 1")) == true
        @test is_symbolic(giac_eval("42")) == false
        @test is_symbolic(giac_eval("x")) == false  # Identifier, not symbolic
    end

    @testset "is_identifier()" begin
        @test is_identifier(giac_eval("x")) == true
        @test is_identifier(giac_eval("abc")) == true
        @test is_identifier(giac_eval("42")) == false
        @test is_identifier(giac_eval("x + 1")) == false
    end

    @testset "is_fraction()" begin
        @test is_fraction(giac_eval("3/7")) == true
        @test is_fraction(giac_eval("42")) == false
        @test is_fraction(giac_eval("6/2")) == false  # Reduces to 3
    end

    @testset "is_complex()" begin
        @test is_complex(giac_eval("2+3*i")) == true
        @test is_complex(giac_eval("5*i")) == true
        @test is_complex(giac_eval("42")) == false
        @test is_complex(giac_eval("3.14")) == false
    end

    @testset "is_string()" begin
        @test is_string(giac_eval("\"hello\"")) == true
        @test is_string(giac_eval("42")) == false
    end

    @testset "subtype()" begin
        # List subtype (default for nested vectors)
        g_nested = giac_eval("[[1,2],[3,4]]")
        @test subtype(g_nested) == VECTSUBTYPE_LIST

        # List subtype
        g_list = giac_eval("[1, 2, 3]")
        @test subtype(g_list) == VECTSUBTYPE_LIST
    end

    @testset "type_name()" begin
        @test type_name(giac_eval("42")) == "integer"
        @test type_name(giac_eval("3.14")) == "double"
        @test type_name(giac_eval("3/7")) == "fraction"
        @test type_name(giac_eval("[1,2,3]")) == "vector"
        @test type_name(giac_eval("sin(x)")) == "symbolic"
    end
end
