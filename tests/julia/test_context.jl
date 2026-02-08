# test_context.jl
# Tests for context management (User Story 2)

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

@testset "GIAC Wrapper Context Tests" begin
    @testset "Variable Assignment" begin
        ctx = GiacContext()
        set_variable(ctx, "a", "5")
        result = eval(ctx, "a+3")
        @test result == "8"
    end

    @testset "Context Isolation" begin
        ctx1 = GiacContext()
        ctx2 = GiacContext()

        set_variable(ctx1, "x", "10")
        set_variable(ctx2, "x", "20")

        @test get_variable(ctx1, "x") == "10"
        @test get_variable(ctx2, "x") == "20"
    end

    @testset "Timeout Configuration" begin
        ctx = GiacContext()

        # Default timeout should be 30 seconds
        @test get_timeout(ctx) == 30

        set_timeout(ctx, 60)
        @test get_timeout(ctx) == 60
    end

    @testset "Precision Configuration" begin
        ctx = GiacContext()
        set_precision(ctx, 50)
        @test get_precision(ctx) == 50
    end

    @testset "Complex Mode" begin
        ctx = GiacContext()

        set_complex_mode(ctx, true)
        @test is_complex_mode(ctx) == true

        set_complex_mode(ctx, false)
        @test is_complex_mode(ctx) == false
    end
end
