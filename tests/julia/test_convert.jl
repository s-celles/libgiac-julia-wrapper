# test_convert.jl
# Tests for Julia type conversions (Feature 004, Phase 12)
#
# Tests for REQ-J40..J43: Base.convert implementations

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
# Collection interface (needed for to_julia)
# ============================================================================

import Base: getindex, length, lastindex, iterate, eltype, convert

function Base.getindex(g::Gen, i::Integer)
    if !is_vector(g)
        throw(ErrorException("GiacExpr is not a vector/list"))
    end
    n = vect_size(g)
    if i < 1 || i > n
        throw(BoundsError(g, i))
    end
    return vect_at(g, Int32(i - 1))
end

function Base.length(g::Gen)
    if is_vector(g)
        return Int(vect_size(g))
    else
        return 1
    end
end

Base.lastindex(g::Gen) = length(g)

function Base.iterate(g::Gen)
    if !is_vector(g)
        return nothing
    end
    n = vect_size(g)
    if n == 0
        return nothing
    end
    return (vect_at(g, Int32(0)), 1)
end

function Base.iterate(g::Gen, state::Int)
    n = vect_size(g)
    if state >= n
        return nothing
    end
    return (vect_at(g, Int32(state)), state + 1)
end

Base.eltype(::Type{Gen}) = Gen

# ============================================================================
# to_julia implementation (from test_to_julia.jl)
# ============================================================================

function to_julia(g::Gen)
    t = type(g)

    if t == GENTYPE_INT
        return to_int64(g)
    elseif t == GENTYPE_DOUBLE
        return to_double(g)
    elseif t == GENTYPE_ZINT
        return parse(BigInt, zint_to_string(g))
    elseif t == GENTYPE_FRAC
        num = to_julia(frac_num(g))
        den = to_julia(frac_den(g))
        if num isa Integer && den isa Integer
            return num // den
        else
            return num / den
        end
    elseif t == GENTYPE_CPLX
        re = to_julia(cplx_re(g))
        im_val = to_julia(cplx_im(g))
        return Complex(re, im_val)
    elseif t == GENTYPE_VECT
        return _to_julia_vector(g)
    elseif t == GENTYPE_STRNG
        return String(strng_value(g))
    elseif t == GENTYPE_IDNT
        return Symbol(idnt_name(g))
    elseif t in (GENTYPE_SYMB, GENTYPE_FUNC)
        return g
    else
        return g
    end
end

function _to_julia_vector(g::Gen)
    n = vect_size(g)
    elements = [to_julia(g[i]) for i in 1:n]
    isempty(elements) && return Any[]
    return identity.(elements)
end

# ============================================================================
# giac_convert implementations (REQ-J40..J43)
# ============================================================================

# Note: We use giac_convert instead of Base.convert to avoid dispatch conflicts
# with CxxWrap's internal convert methods. Use predicates like is_integer()
# instead of type() comparisons to avoid any hidden dispatch issues.

"""
    giac_convert(::Type{Int64}, g::Gen) -> Int64

Convert a Gen of integer type to Int64 (REQ-J40).
Throws InexactError if not a machine integer.
"""
function giac_convert(::Type{Int64}, g::Gen)
    # Use is_integer() predicate and try to extract value
    # This avoids issues with type() dispatch
    try
        val = to_int64(g)
        return val
    catch
        throw(InexactError(:convert, Int64, g))
    end
end

"""
    giac_convert(::Type{Float64}, g::Gen) -> Float64

Convert a Gen of numeric type to Float64 (REQ-J41).
Supports _INT_ and _DOUBLE_ types.
Throws InexactError if not numeric.
"""
function giac_convert(::Type{Float64}, g::Gen)
    # Try double extraction first, then integer
    try
        return to_double(g)
    catch
        throw(InexactError(:convert, Float64, g))
    end
end

"""
    giac_convert(::Type{Vector}, g::Gen) -> Vector

Convert a Gen of vector type to Julia Vector (REQ-J42).
Uses to_julia for recursive conversion.
Throws MethodError if not a vector.
"""
function giac_convert(::Type{Vector}, g::Gen)
    if !is_vector(g)
        throw(MethodError(giac_convert, (Vector, g)))
    end
    return to_julia(g)
end

"""
    giac_convert(::Type{Vector{T}}, g::Gen) -> Vector{T}

Convert a Gen of vector type to typed Julia Vector.
"""
function giac_convert(::Type{Vector{T}}, g::Gen) where T
    if !is_vector(g)
        throw(MethodError(giac_convert, (Vector{T}, g)))
    end
    result = to_julia(g)
    return Base.convert(Vector{T}, result)
end

"""
    giac_convert(::Type{String}, g::Gen) -> String

Convert a Gen of string type to Julia String.
"""
function giac_convert(::Type{String}, g::Gen)
    if is_string(g)
        return String(strng_value(g))
    else
        # Fallback: convert any Gen to string representation
        return to_string(g)
    end
end

"""
    giac_convert(::Type{BigInt}, g::Gen) -> BigInt

Convert a Gen of BigInt type to Julia BigInt.
"""
function giac_convert(::Type{BigInt}, g::Gen)
    # Check if it's a big integer (ZINT) first
    try
        s = zint_to_string(g)
        return parse(BigInt, s)
    catch
        # Maybe it's a regular integer
        try
            return BigInt(to_int64(g))
        catch
            throw(InexactError(:convert, BigInt, g))
        end
    end
end

"""
    giac_convert(::Type{Rational}, g::Gen) -> Rational

Convert a Gen of fraction type to Julia Rational.
"""
function giac_convert(::Type{Rational}, g::Gen)
    if is_fraction(g)
        num = to_julia(frac_num(g))
        den = to_julia(frac_den(g))
        if num isa Integer && den isa Integer
            return num // den
        else
            throw(InexactError(:convert, Rational, g))
        end
    elseif is_integer(g)
        return Rational(to_int64(g))
    else
        throw(InexactError(:convert, Rational, g))
    end
end

"""
    giac_convert(::Type{Complex}, g::Gen) -> Complex

Convert a Gen of complex type to Julia Complex.
"""
function giac_convert(::Type{Complex}, g::Gen)
    if is_complex(g)
        re = to_julia(cplx_re(g))
        im_val = to_julia(cplx_im(g))
        return Complex(re, im_val)
    elseif is_numeric(g)
        return Complex(to_julia(g))
    else
        throw(InexactError(:convert, Complex, g))
    end
end

# ============================================================================
# Tests
# ============================================================================

@testset "Julia Type Conversion Tests" begin
    @testset "giac_convert(Int64, g) - REQ-J40" begin
        # Valid integer conversion
        g = giac_eval("42")
        @test giac_convert(Int64, g) === Int64(42)
        @test giac_convert(Int64, g) isa Int64

        # Negative integer
        g_neg = giac_eval("-123")
        @test giac_convert(Int64, g_neg) === Int64(-123)

        # Int alias
        @test giac_convert(Int, giac_eval("99")) === 99
    end

    @testset "giac_convert(Int64, g) throws on non-integer - REQ-J43" begin
        g_double = giac_eval("3.14")
        @test_throws InexactError giac_convert(Int64, g_double)

        g_frac = giac_eval("3/7")
        @test_throws InexactError giac_convert(Int64, g_frac)

        g_symb = giac_eval("sin(x)")
        @test_throws InexactError giac_convert(Int64, g_symb)

        g_vect = giac_eval("[1, 2, 3]")
        @test_throws InexactError giac_convert(Int64, g_vect)
    end

    @testset "giac_convert(Float64, g) - REQ-J41" begin
        # From double
        g_double = giac_eval("3.14")
        result = giac_convert(Float64, g_double)
        @test result isa Float64
        @test abs(result - 3.14) < 0.001

        # From integer (implicit promotion)
        g_int = giac_eval("42")
        @test giac_convert(Float64, g_int) === 42.0
    end

    @testset "giac_convert(Float64, g) throws on non-numeric - REQ-J43" begin
        g_symb = giac_eval("sin(x)")
        @test_throws InexactError giac_convert(Float64, g_symb)

        g_vect = giac_eval("[1, 2, 3]")
        @test_throws InexactError giac_convert(Float64, g_vect)

        g_str = giac_eval("\"hello\"")
        @test_throws InexactError giac_convert(Float64, g_str)
    end

    @testset "giac_convert(Vector, g) - REQ-J42" begin
        g = giac_eval("[1, 2, 3, 4, 5]")
        result = giac_convert(Vector, g)
        @test result isa Vector
        @test result == [1, 2, 3, 4, 5]

        # Empty vector
        g_empty = giac_eval("[]")
        @test giac_convert(Vector, g_empty) == Any[]

        # Nested vector
        g_nested = giac_eval("[[1, 2], [3, 4]]")
        result_nested = giac_convert(Vector, g_nested)
        @test result_nested isa Vector
        @test length(result_nested) == 2
    end

    @testset "giac_convert(Vector{T}, g) - typed conversion" begin
        g = giac_eval("[1, 2, 3]")
        result = giac_convert(Vector{Int64}, g)
        @test result isa Vector{Int64}
        @test result == [1, 2, 3]
    end

    @testset "giac_convert(Vector, g) throws on non-vector - REQ-J43" begin
        g_int = giac_eval("42")
        @test_throws MethodError giac_convert(Vector, g_int)

        g_symb = giac_eval("sin(x)")
        @test_throws MethodError giac_convert(Vector, g_symb)
    end

    @testset "giac_convert(String, g)" begin
        # String type
        g_str = giac_eval("\"hello world\"")
        @test giac_convert(String, g_str) == "hello world"

        # Fallback to string representation
        g_int = giac_eval("42")
        @test giac_convert(String, g_int) == "42"

        g_symb = giac_eval("sin(x)")
        @test giac_convert(String, g_symb) == "sin(x)"
    end

    @testset "giac_convert(BigInt, g)" begin
        g = giac_eval("factorial(100)")
        result = giac_convert(BigInt, g)
        @test result isa BigInt
        @test result == factorial(big(100))

        # From integer
        g_int = giac_eval("42")
        @test giac_convert(BigInt, g_int) == big(42)
    end

    @testset "giac_convert(BigInt, g) throws on incompatible" begin
        g_double = giac_eval("3.14")
        @test_throws InexactError giac_convert(BigInt, g_double)

        g_symb = giac_eval("sin(x)")
        @test_throws InexactError giac_convert(BigInt, g_symb)
    end

    @testset "giac_convert(Rational, g)" begin
        g = giac_eval("3/7")
        result = giac_convert(Rational, g)
        @test result isa Rational
        @test result == 3//7

        # From integer
        g_int = giac_eval("5")
        @test giac_convert(Rational, g_int) == 5//1
    end

    @testset "giac_convert(Rational, g) throws on incompatible" begin
        g_double = giac_eval("3.14")
        @test_throws InexactError giac_convert(Rational, g_double)

        g_symb = giac_eval("sin(x)")
        @test_throws InexactError giac_convert(Rational, g_symb)
    end

    @testset "giac_convert(Complex, g)" begin
        g = giac_eval("2+3*i")
        result = giac_convert(Complex, g)
        @test result isa Complex
        @test result == 2 + 3im

        # From real number
        g_int = giac_eval("5")
        @test giac_convert(Complex, g_int) == 5 + 0im

        g_double = giac_eval("3.14")
        result_d = giac_convert(Complex, g_double)
        @test real(result_d) ≈ 3.14
        @test imag(result_d) == 0
    end

    @testset "giac_convert(Complex, g) throws on incompatible" begin
        g_symb = giac_eval("sin(x)")
        @test_throws InexactError giac_convert(Complex, g_symb)

        g_vect = giac_eval("[1, 2]")
        @test_throws InexactError giac_convert(Complex, g_vect)
    end

    @testset "Function call with conversion" begin
        # Test that giac_convert enables type-safe function calls
        function takes_int(x::Int64)
            return x * 2
        end

        g = giac_eval("21")
        @test takes_int(giac_convert(Int64, g)) == 42
    end

    @testset "Type stability" begin
        g_int = giac_eval("42")
        g_double = giac_eval("3.14")
        g_vect = giac_eval("[1, 2, 3]")

        # Verify return types are stable
        @test giac_convert(Int64, g_int) isa Int64
        @test giac_convert(Float64, g_double) isa Float64
        @test giac_convert(Vector, g_vect) isa Vector
    end

    @testset "to_julia vs giac_convert consistency" begin
        # Verify that giac_convert produces same results as to_julia for compatible types
        g_int = giac_eval("42")
        @test giac_convert(Int64, g_int) == to_julia(g_int)

        g_vect = giac_eval("[1, 2, 3]")
        @test giac_convert(Vector, g_vect) == to_julia(g_vect)

        g_frac = giac_eval("3/7")
        @test giac_convert(Rational, g_frac) == to_julia(g_frac)
    end
end
