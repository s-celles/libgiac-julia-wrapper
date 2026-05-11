# test_eval.jl
# Tests for expression evaluation (User Story 1)

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

@testset "GIAC Wrapper Eval Tests" begin
    @testset "Version Functions" begin
        @test giac_version() isa String
        @test !isempty(giac_version())
        # Issue #2: wrapper_version() now reads from meson.project_version()
        # at build time, so it never falls out of sync with meson.build.
        # Accept any SemVer-shaped string instead of a hardcoded literal.
        @test occursin(r"^\d+\.\d+\.\d+", wrapper_version())
    end

    @testset "GIAC Availability" begin
        @test is_giac_available() == true
    end

    @testset "Basic Eval" begin
        ctx = GiacContext()
        @test eval(ctx, "1+1") == "2"
        @test eval(ctx, "2*3") == "6"
    end

    @testset "Factor Operation" begin
        ctx = GiacContext()
        result = eval(ctx, "factor(x^2-1)")
        @test occursin("x-1", result) || occursin("x+1", result)
    end

    @testset "Error Handling" begin
        ctx = GiacContext()
        @test_throws Exception eval(ctx, "invalid(((")
    end
end
