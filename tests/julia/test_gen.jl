# test_gen.jl
# Tests for Gen objects (User Story 3)

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

@testset "GIAC Wrapper Gen Tests" begin
    @testset "Gen Construction" begin
        ctx = GiacContext()
        g = Gen("x^2", ctx)
        @test to_string(g) isa String
    end

    @testset "Gen Type Query" begin
        ctx = GiacContext()
        g = Gen("42", ctx)
        @test type(g) == 0  # Integer type
        @test type_name(g) == "integer"
    end

    @testset "Gen Arithmetic" begin
        ctx = GiacContext()
        a = Gen("x", ctx)
        b = Gen("y", ctx)

        sum = a + b
        @test occursin("x", to_string(sum))
        @test occursin("y", to_string(sum))
    end

    @testset "Gen Operations" begin
        ctx = GiacContext()
        g = Gen("x^2 - 1", ctx)

        factored = factor(g)
        @test occursin("x-1", to_string(factored)) || occursin("x+1", to_string(factored))
    end
end
