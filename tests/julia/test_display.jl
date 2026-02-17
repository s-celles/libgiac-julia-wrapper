# test_display.jl
# Tests for Gen REPL display (Feature 004, User Story 4)
#
# Tests for REQ-J50..J52: Human-readable REPL display

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

# ============================================================================
# Base.show implementations (REQ-J50..J52)
# ============================================================================

import Base: show

"""
Single-line display for Gen (used in arrays, etc.)
"""
function Base.show(io::IO, g::Gen)
    print(io, to_string(g))
end

"""
Multi-line REPL display with type information
"""
function Base.show(io::IO, ::MIME"text/plain", g::Gen)
    t = type(g)
    type_str = _type_name(t)

    if t == GENTYPE_VECT
        # REQ-J51: Vector display with brackets
        n = vect_size(g)
        print(io, "Gen{", type_str, "}([")
        for i in 0:(n-1)
            i > 0 && print(io, ", ")
            print(io, to_string(vect_at(g, Int32(i))))
        end
        print(io, "])")
    elseif t == GENTYPE_INT || t == GENTYPE_DOUBLE
        # REQ-J52: Numeric display directly
        print(io, to_string(g))
    else
        # REQ-J50: Type and textual representation
        print(io, "Gen{", type_str, "}(", to_string(g), ")")
    end
end

"""
Helper: Convert type constant to readable name
"""
function _type_name(t::Int32)
    if t == GENTYPE_INT
        return "Int"
    elseif t == GENTYPE_DOUBLE
        return "Double"
    elseif t == GENTYPE_ZINT
        return "BigInt"
    elseif t == GENTYPE_FRAC
        return "Fraction"
    elseif t == GENTYPE_CPLX
        return "Complex"
    elseif t == GENTYPE_VECT
        return "Vector"
    elseif t == GENTYPE_SYMB
        return "Symbolic"
    elseif t == GENTYPE_IDNT
        return "Identifier"
    elseif t == GENTYPE_STRNG
        return "String"
    elseif t == GENTYPE_FUNC
        return "Function"
    else
        return "Unknown"
    end
end

# ============================================================================
# Tests
# ============================================================================

@testset "REPL Display Tests" begin
    @testset "Single-line display (Base.show)" begin
        # Test that show(io, g) outputs the string representation
        g_int = giac_eval("42")
        @test sprint(show, g_int) == "42"

        g_symb = giac_eval("sin(x)")
        @test sprint(show, g_symb) == "sin(x)"

        g_vect = giac_eval("[1, 2, 3]")
        @test sprint(show, g_vect) == "[1,2,3]"
    end

    @testset "Multi-line display for integers (REQ-J52)" begin
        g = giac_eval("42")
        output = sprint(show, MIME("text/plain"), g)
        # Numeric values displayed directly
        @test output == "42"
    end

    @testset "Multi-line display for floats (REQ-J52)" begin
        g = giac_eval("3.14")
        output = sprint(show, MIME("text/plain"), g)
        # Numeric values displayed directly
        @test contains(output, "3.14")
    end

    @testset "Multi-line display for vectors (REQ-J51)" begin
        g = giac_eval("[1, 2, 3]")
        output = sprint(show, MIME("text/plain"), g)
        # Vector display with brackets
        @test contains(output, "Gen{Vector}")
        @test contains(output, "[1, 2, 3]")
    end

    @testset "Multi-line display for symbolic (REQ-J50)" begin
        g = giac_eval("x^2 + 2*x + 1")
        output = sprint(show, MIME("text/plain"), g)
        # Type and textual representation
        @test contains(output, "Gen{Symbolic}")
        @test contains(output, "x^2+2*x+1") || contains(output, "x²+2x+1")
    end

    @testset "Multi-line display for fractions (REQ-J50)" begin
        g = giac_eval("3/7")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{Fraction}")
        @test contains(output, "3/7")
    end

    @testset "Multi-line display for complex (REQ-J50)" begin
        g = giac_eval("2+3*i")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{Complex}")
        @test contains(output, "2+3*i")
    end

    @testset "Multi-line display for BigInt (REQ-J50)" begin
        g = giac_eval("factorial(100)")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{BigInt}")
    end

    @testset "Multi-line display for identifier (REQ-J50)" begin
        g = giac_eval("x")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{Identifier}")
        @test contains(output, "x")
    end

    @testset "Multi-line display for string (REQ-J50)" begin
        g = giac_eval("\"hello\"")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{String}")
        @test contains(output, "hello")
    end

    @testset "Nested vector display" begin
        g = giac_eval("[[1, 2], [3, 4]]")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{Vector}")
        @test contains(output, "[1,2]")
        @test contains(output, "[3,4]")
    end

    @testset "Empty vector display" begin
        g = giac_eval("[]")
        output = sprint(show, MIME("text/plain"), g)
        @test contains(output, "Gen{Vector}")
        @test contains(output, "[]")
    end

    @testset "Display in array context" begin
        g1 = giac_eval("1")
        g2 = giac_eval("2")
        arr = [g1, g2]
        # When shown in array, uses single-line form
        output = sprint(show, arr)
        @test contains(output, "1")
        @test contains(output, "2")
    end
end
