# test_to_julia.jl
# Tests for to_julia() conversion (Feature 004, User Story 1)
#
# Tests for REQ-J01, REQ-J10..J18: Recursive conversion to native Julia types

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

# Implement Julia collection interface for Gen
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

function Base.lastindex(g::Gen)
    return length(g)
end

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
# to_julia() implementation (REQ-J01, REQ-J10..J18)
# ============================================================================

"""
    to_julia(g::Gen) -> Any

Recursively convert a Giac Gen expression to native Julia types.

# Conversion Rules
- `_INT_` → `Int64`
- `_DOUBLE_` → `Float64`
- `_ZINT_` → `BigInt`
- `_FRAC_` with integer parts → `Rational{Int64}`
- `_FRAC_` with non-integer parts → quotient
- `_CPLX_` → `Complex{T}`
- `_VECT_` (list) → `Vector{T}` with type narrowing
- `_VECT_` (matrix, subtype=11) → `Matrix{T}`
- `_VECT_` (set, subtype=2) → `Set{Any}`
- `_STRNG_` → `String`
- `_SYMB_`, `_IDNT_`, `_FUNC_` → unchanged (Gen)
"""
function to_julia(g::Gen)
    t = type(g)

    if t == GENTYPE_INT
        # REQ-J10: Integer → Int64
        return to_int64(g)
    elseif t == GENTYPE_DOUBLE
        # REQ-J11: Double → Float64
        return to_double(g)
    elseif t == GENTYPE_ZINT
        # REQ-J16: BigInt via string parsing
        return parse(BigInt, zint_to_string(g))
    elseif t == GENTYPE_FRAC
        # REQ-J12, J13: Fraction → Rational or quotient
        num = to_julia(frac_num(g))
        den = to_julia(frac_den(g))
        if num isa Integer && den isa Integer
            return num // den
        else
            return num / den
        end
    elseif t == GENTYPE_CPLX
        # REQ-J14: Complex → Complex{T}
        re = to_julia(cplx_re(g))
        im_val = to_julia(cplx_im(g))
        return Complex(re, im_val)
    elseif t == GENTYPE_VECT
        # REQ-J15: Vector → recursive conversion with type narrowing
        return _to_julia_vector(g)
    elseif t == GENTYPE_STRNG
        # REQ-J17: String → String (convert from C++ std::string)
        return String(strng_value(g))
    elseif t == GENTYPE_IDNT
        # Extension: Identifier → Symbol
        return Symbol(idnt_name(g))
    elseif t in (GENTYPE_SYMB, GENTYPE_FUNC)
        # REQ-J18: Symbolic/Function → unchanged
        return g
    else
        # Fallback: return Gen unchanged
        @warn "Unknown Gen type $(t), returning unchanged"
        return g
    end
end

"""
Internal: Convert vector Gen to Julia array with type narrowing.
Handles special subtypes: Matrix (11) and Set (2).
"""
function _to_julia_vector(g::Gen)
    n = vect_size(g)
    st = subtype(g)

    # Handle Set subtype (T064)
    if st == VECTSUBTYPE_SET
        elements = [to_julia(g[i]) for i in 1:n]
        return Set(elements)
    end

    # Handle Matrix subtype (T063)
    if st == VECTSUBTYPE_MATRIX
        return _to_julia_matrix(g)
    end

    # Default: regular vector/list
    # Collect elements
    elements = [to_julia(g[i]) for i in 1:n]

    # Handle empty vector
    isempty(elements) && return Any[]

    # Type narrowing using identity broadcast
    return _narrow_vector(elements)
end

"""
Internal: Convert matrix Gen to Julia Matrix{T}.
Assumes g is a vector of vectors (rows).
"""
function _to_julia_matrix(g::Gen)
    nrows = vect_size(g)
    nrows == 0 && return Matrix{Any}(undef, 0, 0)

    # Get first row to determine number of columns
    first_row = g[1]
    if !is_vector(first_row)
        # Single row case or malformed - fall back to vector
        elements = [to_julia(g[i]) for i in 1:nrows]
        return _narrow_vector(elements)
    end

    ncols = vect_size(first_row)

    # Collect all elements row by row
    rows = Vector{Vector{Any}}()
    for i in 1:nrows
        row = g[i]
        if is_vector(row)
            push!(rows, [to_julia(row[j]) for j in 1:vect_size(row)])
        else
            # Malformed matrix - fall back to vector
            elements = [to_julia(g[k]) for k in 1:nrows]
            return _narrow_vector(elements)
        end
    end

    # Check all rows have same length
    if !all(length(r) == ncols for r in rows)
        # Jagged array - return as vector of vectors
        return [_narrow_vector(r) for r in rows]
    end

    # Build matrix
    # First narrow each row to get element types
    narrowed_rows = [_narrow_vector(r) for r in rows]

    # Determine common element type
    T = promote_type([eltype(r) for r in narrowed_rows]...)

    # Create and fill matrix
    mat = Matrix{T}(undef, nrows, ncols)
    for i in 1:nrows
        for j in 1:ncols
            mat[i, j] = narrowed_rows[i][j]
        end
    end

    return mat
end

"""
Internal: Narrow Vector{Any} to tighter type if possible (REQ-J20..J22)
"""
function _narrow_vector(v::Vector)
    isempty(v) && return v

    # Try narrowing with identity broadcast
    narrowed = identity.(v)

    T = eltype(narrowed)
    if T === Any
        # REQ-J22: Mixed incompatible types
        return narrowed
    elseif T <: Number
        # REQ-J21: Promote numeric types
        return narrowed
    else
        # REQ-J20: Homogeneous type
        return narrowed
    end
end

# ============================================================================
# Tests
# ============================================================================

@testset "to_julia() Conversion Tests" begin
    @testset "Integer conversion (REQ-J10)" begin
        g = giac_eval("42")
        result = to_julia(g)
        @test result === Int64(42)
        @test result isa Int64

        g_neg = giac_eval("-123")
        @test to_julia(g_neg) === Int64(-123)
    end

    @testset "Float conversion (REQ-J11)" begin
        g = giac_eval("3.14")
        result = to_julia(g)
        @test result isa Float64
        @test abs(result - 3.14) < 0.001
    end

    @testset "BigInt conversion (REQ-J16)" begin
        g = giac_eval("factorial(100)")
        result = to_julia(g)
        @test result isa BigInt
        @test result == factorial(big(100))
    end

    @testset "Fraction conversion (REQ-J12)" begin
        g = giac_eval("3/7")
        result = to_julia(g)
        @test result isa Rational
        @test result == 3//7
    end

    @testset "Complex conversion (REQ-J14)" begin
        g = giac_eval("2+3*i")
        result = to_julia(g)
        @test result isa Complex
        @test result == 2 + 3im
    end

    @testset "Vector conversion (REQ-J15)" begin
        g = giac_eval("[1, 2, 3, 4, 5]")
        result = to_julia(g)
        @test result isa Vector
        @test result == [1, 2, 3, 4, 5]
    end

    @testset "Vector type narrowing (REQ-J20..J22)" begin
        # Homogeneous integer vector → Vector{Int64}
        g_int = giac_eval("[1, 2, 3]")
        @test to_julia(g_int) == [1, 2, 3]
        @test eltype(to_julia(g_int)) == Int64

        # Mixed int/float → Vector with promoted type
        g_mixed = giac_eval("[1, 2.5, 3]")
        result = to_julia(g_mixed)
        @test result isa Vector
        @test length(result) == 3

        # Empty vector
        g_empty = giac_eval("[]")
        @test to_julia(g_empty) == Any[]
    end

    @testset "String conversion (REQ-J17)" begin
        g = giac_eval("\"hello world\"")
        result = to_julia(g)
        @test result isa String
        @test result == "hello world"
    end

    @testset "Symbolic passthrough (REQ-J18)" begin
        g_symb = giac_eval("sin(x)")
        result = to_julia(g_symb)
        @test result isa Gen  # Unchanged
        @test to_string(result) == "sin(x)"
    end

    @testset "Identifier to Symbol" begin
        g = giac_eval("x")
        result = to_julia(g)
        @test result isa Symbol
        @test result == :x
    end

    @testset "Nested vector conversion" begin
        g = giac_eval("[[1, 2], [3, 4]]")
        result = to_julia(g)
        @test result isa Vector
        @test length(result) == 2
        @test result[1] == [1, 2]
        @test result[2] == [3, 4]
    end

    @testset "Complex vector" begin
        g = giac_eval("[1+2*i, 3+4*i]")
        result = to_julia(g)
        @test result isa Vector
        @test result[1] == 1 + 2im
        @test result[2] == 3 + 4im
    end

    @testset "Fraction vector" begin
        g = giac_eval("[1/2, 1/3, 1/4]")
        result = to_julia(g)
        @test result isa Vector
        @test result == [1//2, 1//3, 1//4]
    end

    @testset "Matrix conversion (T063)" begin
        # Explicit matrix with subtype VECTSUBTYPE_MATRIX
        g = giac_eval("matrix(2,2,[1,2,3,4])")
        result = to_julia(g)
        @test result isa Matrix
        @test size(result) == (2, 2)
        @test result[1, 1] == 1
        @test result[1, 2] == 2
        @test result[2, 1] == 3
        @test result[2, 2] == 4

        # 3x3 matrix
        g2 = giac_eval("matrix(3,3,[1,2,3,4,5,6,7,8,9])")
        result2 = to_julia(g2)
        @test result2 isa Matrix
        @test size(result2) == (3, 3)
        @test result2 == [1 2 3; 4 5 6; 7 8 9]

        # Matrix with fractions
        g3 = giac_eval("matrix(2,2,[1/2,1/3,1/4,1/5])")
        result3 = to_julia(g3)
        @test result3 isa Matrix
        @test result3[1, 1] == 1//2
        @test result3[2, 2] == 1//5
    end

    @testset "Set conversion (T064)" begin
        # Basic set
        g = giac_eval("set[1, 2, 3]")
        result = to_julia(g)
        @test result isa Set
        @test 1 in result
        @test 2 in result
        @test 3 in result
        @test length(result) == 3

        # Set with duplicates (should be deduplicated by GIAC)
        g2 = giac_eval("set[1, 2, 2, 3, 3, 3]")
        result2 = to_julia(g2)
        @test result2 isa Set
        @test length(result2) == 3

        # Empty set
        g3 = giac_eval("set[]")
        result3 = to_julia(g3)
        @test result3 isa Set
        @test isempty(result3)

        # Set with mixed types
        g4 = giac_eval("%{1, 2, 3%}")
        result4 = to_julia(g4)
        @test result4 isa Set
        @test 1 in result4
    end

    @testset "Regular nested vector (not matrix)" begin
        # Regular nested vector should still be Vector, not Matrix
        g = giac_eval("[[1, 2], [3, 4]]")
        result = to_julia(g)
        @test result isa Vector  # Not Matrix, because subtype is LIST (0)
        @test result[1] == [1, 2]
        @test result[2] == [3, 4]
    end
end
