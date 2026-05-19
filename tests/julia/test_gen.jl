# test_gen.jl
# Tests for Gen objects (User Story 3)

include(joinpath(@__DIR__, "load_wrapper.jl"))

@testset "GIAC Wrapper Gen Tests" begin
    # The single-arg Gen(::String) constructor uses the wrapper's thread-local
    # default context. There is no Gen(::String, ::GiacContext) overload
    # registered with CxxWrap, so these tests do NOT pass a context.

    @testset "Gen Construction" begin
        g = Gen("x^2")
        @test to_string(g) isa AbstractString
    end

    @testset "Gen Type Query" begin
        g = Gen("42")
        @test type(g) == 0  # Integer type
        @test type_name(g) == "integer"
    end

    @testset "Gen Arithmetic" begin
        a = Gen("x")
        b = Gen("y")

        sum = a + b
        @test occursin("x", to_string(sum))
        @test occursin("y", to_string(sum))
    end

    @testset "Gen Operations" begin
        g = Gen("x^2 - 1")

        factored = factor(g)
        @test occursin("x-1", to_string(factored)) || occursin("x+1", to_string(factored))
    end
end
