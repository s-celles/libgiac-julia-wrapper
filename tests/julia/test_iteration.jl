# test_iteration.jl
# Tests for Gen vector iteration (Feature 004, User Story 3)
#
# Tests for REQ-J35..J36: Julia iteration protocol

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
import Base: getindex, length, iterate, eltype, lastindex

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

# REQ-J35: Iteration protocol for vectors
function Base.iterate(g::Gen)
    if !is_vector(g)
        # REQ-J36: Returns nothing for non-vectors
        return nothing
    end
    n = vect_size(g)
    if n == 0
        return nothing
    end
    return (vect_at(g, Int32(0)), 1)  # (first element, next state)
end

function Base.iterate(g::Gen, state::Int)
    n = vect_size(g)
    if state >= n
        return nothing
    end
    return (vect_at(g, Int32(state)), state + 1)
end

Base.eltype(::Type{Gen}) = Gen

@testset "Vector Iteration Tests" begin
    @testset "Basic iteration" begin
        g = giac_eval("[10, 20, 30]")

        # Collect elements via iteration
        elements = Gen[]
        for elem in g
            push!(elements, elem)
        end

        @test length(elements) == 3
        @test to_string(elements[1]) == "10"
        @test to_string(elements[2]) == "20"
        @test to_string(elements[3]) == "30"
    end

    @testset "collect() support" begin
        g = giac_eval("[1, 2, 3, 4, 5]")
        collected = collect(g)

        @test length(collected) == 5
        @test all(x -> x isa Gen, collected)
        @test to_string(collected[1]) == "1"
        @test to_string(collected[5]) == "5"
    end

    @testset "for loop accumulation" begin
        g = giac_eval("[1, 2, 3, 4, 5]")

        # Sum elements as integers
        sum_val = 0
        for elem in g
            sum_val += to_int64(elem)
        end

        @test sum_val == 15
    end

    @testset "Non-vector iteration returns nothing" begin
        g_int = giac_eval("42")
        g_symb = giac_eval("sin(x)")

        # REQ-J36: Returns nothing for non-vectors
        @test iterate(g_int) === nothing
        @test iterate(g_symb) === nothing
    end

    @testset "Empty vector iteration" begin
        g = giac_eval("[]")

        elements = Gen[]
        for elem in g
            push!(elements, elem)
        end

        @test length(elements) == 0
    end

    @testset "Nested vector iteration" begin
        g = giac_eval("[[1, 2], [3, 4]]")

        rows = collect(g)
        @test length(rows) == 2

        # Each row should be iterable
        row1_elements = collect(rows[1])
        @test length(row1_elements) == 2
        @test to_string(row1_elements[1]) == "1"
        @test to_string(row1_elements[2]) == "2"
    end

    @testset "map() support" begin
        g = giac_eval("[1, 2, 3]")

        # Map each Gen to its string representation
        strings = map(to_string, g)

        @test length(strings) == 3
        @test strings[1] == "1"
        @test strings[2] == "2"
        @test strings[3] == "3"
    end

    @testset "filter() support" begin
        g = giac_eval("[1, 2, 3, 4, 5, 6]")

        # Filter even numbers
        evens = filter(x -> to_int64(x) % 2 == 0, collect(g))

        @test length(evens) == 3
        @test to_string(evens[1]) == "2"
        @test to_string(evens[2]) == "4"
        @test to_string(evens[3]) == "6"
    end

    @testset "enumerate() support" begin
        g = giac_eval("[10, 20, 30]")

        results = collect(enumerate(g))

        @test length(results) == 3
        @test results[1][1] == 1
        @test to_string(results[1][2]) == "10"
        @test results[3][1] == 3
        @test to_string(results[3][2]) == "30"
    end
end
