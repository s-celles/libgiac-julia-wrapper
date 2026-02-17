# test_indexing.jl
# Tests for Gen vector indexing (Feature 004, User Story 3)
#
# Tests for REQ-J30..J34: 1-based indexing and length

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
import Base: getindex, length, lastindex

function Base.getindex(g::Gen, i::Integer)
    # REQ-J32: Throws on non-vector
    if !is_vector(g)
        throw(ErrorException("GiacExpr is not a vector/list"))
    end
    n = vect_size(g)
    # REQ-J31: Throws on invalid index
    if i < 1 || i > n
        throw(BoundsError(g, i))
    end
    # REQ-J30: 1-based indexing (convert to 0-based for C++)
    return vect_at(g, Int32(i - 1))
end

function Base.length(g::Gen)
    if is_vector(g)
        # REQ-J33: Returns vector size
        return Int(vect_size(g))
    else
        # REQ-J34: Returns 1 for non-vectors
        return 1
    end
end

function Base.lastindex(g::Gen)
    return length(g)
end

@testset "Vector Indexing Tests" begin
    @testset "Basic 1-based indexing" begin
        g = giac_eval("[10, 20, 30, 40, 50]")

        # REQ-J30: 1-based indexing
        @test to_string(g[1]) == "10"
        @test to_string(g[2]) == "20"
        @test to_string(g[3]) == "30"
        @test to_string(g[5]) == "50"
    end

    @testset "length() for vectors" begin
        # REQ-J33: Vector size
        @test length(giac_eval("[1, 2, 3]")) == 3
        @test length(giac_eval("[]")) == 0
        @test length(giac_eval("[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]")) == 10
    end

    @testset "length() for non-vectors" begin
        # REQ-J34: Returns 1 for non-vectors
        @test length(giac_eval("42")) == 1
        @test length(giac_eval("3.14")) == 1
        @test length(giac_eval("sin(x)")) == 1
    end

    @testset "BoundsError on invalid index" begin
        g = giac_eval("[1, 2, 3]")

        # REQ-J31: Throws BoundsError
        @test_throws BoundsError g[0]
        @test_throws BoundsError g[4]
        @test_throws BoundsError g[10]
        @test_throws BoundsError g[-1]
    end

    @testset "ErrorException on non-vector indexing" begin
        g_int = giac_eval("42")
        g_symb = giac_eval("sin(x)")

        # REQ-J32: Throws on non-vector
        @test_throws ErrorException g_int[1]
        @test_throws ErrorException g_symb[1]
    end

    @testset "Nested vector indexing" begin
        g = giac_eval("[[1, 2], [3, 4]]")

        # Access first row
        row1 = g[1]
        @test is_vector(row1)
        @test to_string(row1[1]) == "1"
        @test to_string(row1[2]) == "2"

        # Access second row
        row2 = g[2]
        @test to_string(row2[1]) == "3"
        @test to_string(row2[2]) == "4"
    end

    @testset "lastindex() support" begin
        g = giac_eval("[10, 20, 30]")
        @test to_string(g[end]) == "30"
    end
end
