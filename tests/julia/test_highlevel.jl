# test_highlevel.jl
# Tests for high-level function wrappers (Feature 004, User Story 8)
#
# Tests for REQ-J60..J63: Convenience functions with automatic conversion

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

import Base: getindex, length, lastindex, iterate, eltype

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
# High-Level Wrapper Module (REQ-J60..J63)
# ============================================================================

"""
High-level wrapper functions for common Giac operations.
These provide a more Julia-friendly interface with automatic type conversion.
"""
module Giac

using ..Main: Gen, giac_eval, to_julia, to_string, is_vector

"""
    zeros(expr, var) -> Vector

Find zeros of an expression with respect to a variable.
Returns Julia Vector via automatic conversion.

# Example
```julia
zeros("x^2-1", "x")  # Returns [-1, 1]
```
"""
function zeros(expr::AbstractString, var::AbstractString)
    cmd = "zeros($expr, $var)"
    result = giac_eval(cmd)
    return to_julia(result)
end

function zeros(expr::Gen, var::AbstractString)
    zeros(to_string(expr), var)
end

function zeros(expr::Gen, var::Gen)
    zeros(to_string(expr), to_string(var))
end

"""
    solve(expr, var) -> Vector

Solve an equation or expression for a variable.
Returns Julia Vector via automatic conversion.

# Example
```julia
solve("x^2-4=0", "x")  # Returns [-2, 2]
solve("2*x+1", "x")    # Returns [-1//2]
```
"""
function solve(expr::AbstractString, var::AbstractString)
    cmd = "solve($expr, $var)"
    result = giac_eval(cmd)
    return to_julia(result)
end

function solve(expr::Gen, var::AbstractString)
    solve(to_string(expr), var)
end

function solve(expr::Gen, var::Gen)
    solve(to_string(expr), to_string(var))
end

"""
    factor(expr) -> Gen

Factor an expression. Returns Gen (symbolic result).

# Example
```julia
factor("x^2-1")  # Returns (x-1)*(x+1)
```
"""
function factor(expr::AbstractString)
    cmd = "factor($expr)"
    return giac_eval(cmd)
end

function factor(expr::Gen)
    factor(to_string(expr))
end

"""
    simplify(expr) -> Gen

Simplify an expression. Returns Gen (symbolic result).

# Example
```julia
simplify("(x^2-1)/(x-1)")  # Returns x+1
```
"""
function simplify(expr::AbstractString)
    cmd = "simplify($expr)"
    return giac_eval(cmd)
end

function simplify(expr::Gen)
    simplify(to_string(expr))
end

"""
    expand(expr) -> Gen

Expand an expression. Returns Gen (symbolic result).

# Example
```julia
expand("(x+1)^2")  # Returns x^2+2*x+1
```
"""
function expand(expr::AbstractString)
    cmd = "expand($expr)"
    return giac_eval(cmd)
end

function expand(expr::Gen)
    expand(to_string(expr))
end

"""
    diff(expr, var) -> Gen

Differentiate an expression with respect to a variable.
Returns Gen (symbolic result).

# Example
```julia
diff("x^3", "x")  # Returns 3*x^2
```
"""
function diff(expr::AbstractString, var::AbstractString)
    cmd = "diff($expr, $var)"
    return giac_eval(cmd)
end

function diff(expr::Gen, var::AbstractString)
    diff(to_string(expr), var)
end

function diff(expr::Gen, var::Gen)
    diff(to_string(expr), to_string(var))
end

"""
    integrate(expr, var) -> Gen

Integrate an expression with respect to a variable (indefinite integral).
Returns Gen (symbolic result).

# Example
```julia
integrate("x^2", "x")  # Returns x^3/3
```
"""
function integrate(expr::AbstractString, var::AbstractString)
    cmd = "integrate($expr, $var)"
    return giac_eval(cmd)
end

function integrate(expr::Gen, var::AbstractString)
    integrate(to_string(expr), var)
end

function integrate(expr::Gen, var::Gen)
    integrate(to_string(expr), to_string(var))
end

"""
    limit(expr, var, val) -> Any

Compute the limit of an expression as variable approaches a value.
Returns Julia type via automatic conversion when possible.

# Example
```julia
limit("sin(x)/x", "x", "0")  # Returns 1
```
"""
function limit(expr::AbstractString, var::AbstractString, val::AbstractString)
    cmd = "limit($expr, $var, $val)"
    result = giac_eval(cmd)
    return to_julia(result)
end

function limit(expr::AbstractString, var::AbstractString, val::Number)
    limit(expr, var, string(val))
end

function limit(expr::Gen, var::AbstractString, val)
    limit(to_string(expr), var, string(val))
end

"""
    series(expr, var, point, order) -> Gen

Compute Taylor series expansion of an expression.
Returns Gen (symbolic result).

# Example
```julia
series("sin(x)", "x", "0", 5)  # Returns x-x^3/6+x^5/120+...
```
"""
function series(expr::AbstractString, var::AbstractString, point::AbstractString, order::Integer)
    cmd = "series($expr, $var=$point, $order)"
    return giac_eval(cmd)
end

function series(expr::AbstractString, var::AbstractString, point::Number, order::Integer)
    series(expr, var, string(point), order)
end

function series(expr::Gen, var::AbstractString, point, order::Integer)
    series(to_string(expr), var, string(point), order)
end

end  # module Giac

# ============================================================================
# Tests
# ============================================================================

@testset "High-Level Wrapper Tests" begin
    @testset "zeros() - Find roots (REQ-J60)" begin
        # Simple polynomial zeros
        result = Giac.zeros("x^2-1", "x")
        @test result isa Vector
        @test length(result) == 2
        @test -1 in result && 1 in result

        # Quadratic
        result2 = Giac.zeros("x^2-4", "x")
        @test length(result2) == 2
        @test -2 in result2 && 2 in result2

        # Linear
        result3 = Giac.zeros("2*x+4", "x")
        @test length(result3) == 1
        @test result3[1] == -2
    end

    @testset "solve() - Solve equations (REQ-J61)" begin
        # Simple linear
        result = Giac.solve("2*x+1", "x")
        @test result isa Vector
        @test length(result) == 1
        @test result[1] == -1//2 || result[1] == -0.5

        # Quadratic
        result2 = Giac.solve("x^2-9", "x")
        @test length(result2) == 2
        @test -3 in result2 && 3 in result2
    end

    @testset "factor() - Factorization (REQ-J62)" begin
        result = Giac.factor("x^2-1")
        @test result isa Gen
        str = to_string(result)
        @test contains(str, "x-1") || contains(str, "x+1")

        result2 = Giac.factor("x^2+2*x+1")
        str2 = to_string(result2)
        @test contains(str2, "x+1")
    end

    @testset "simplify() (REQ-J63)" begin
        result = Giac.simplify("(x^2-1)/(x-1)")
        @test result isa Gen
        str = to_string(result)
        @test str == "x+1"
    end

    @testset "expand() (REQ-J63)" begin
        result = Giac.expand("(x+1)^2")
        @test result isa Gen
        str = to_string(result)
        @test contains(str, "x^2") || contains(str, "x²")
        @test contains(str, "2*x") || contains(str, "2x")
        @test contains(str, "1")
    end

    @testset "diff() - Differentiation (REQ-J63)" begin
        # Power rule
        result = Giac.diff("x^3", "x")
        @test result isa Gen
        str = to_string(result)
        @test contains(str, "3") && contains(str, "x")

        # Trigonometric
        result2 = Giac.diff("sin(x)", "x")
        str2 = to_string(result2)
        @test contains(str2, "cos")
    end

    @testset "integrate() - Integration (REQ-J63)" begin
        # Power rule
        result = Giac.integrate("x^2", "x")
        @test result isa Gen
        str = to_string(result)
        @test contains(str, "x^3") || contains(str, "x³")
        @test contains(str, "3")

        # Trigonometric
        result2 = Giac.integrate("cos(x)", "x")
        str2 = to_string(result2)
        @test contains(str2, "sin")
    end

    @testset "limit() (REQ-J63)" begin
        # Classic limit
        result = Giac.limit("sin(x)/x", "x", "0")
        @test result == 1

        # Limit at infinity
        result2 = Giac.limit("1/x", "x", "+infinity")
        @test result2 == 0
    end

    @testset "series() - Taylor expansion (REQ-J63)" begin
        result = Giac.series("exp(x)", "x", "0", 4)
        @test result isa Gen
        str = to_string(result)
        # Should contain terms like 1, x, x^2/2, x^3/6
        @test contains(str, "x")
    end

    @testset "Gen input support" begin
        # Test that functions accept Gen as input
        x_expr = giac_eval("x^2-1")

        result = Giac.factor(x_expr)
        @test result isa Gen

        # Test zeros with Gen input
        expr = giac_eval("x^2-4")
        result2 = Giac.zeros(expr, "x")
        @test result2 isa Vector
    end

    @testset "MVP validation: to_julia(zeros(x^2-1))" begin
        # This is the MVP goal from the spec
        result = Giac.zeros("x^2-1", "x")
        @test result isa Vector{Int64}
        @test result == [-1, 1] || result == [1, -1]
    end
end
