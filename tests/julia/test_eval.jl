# test_eval.jl
# Tests for expression evaluation (User Story 1)

include(joinpath(@__DIR__, "load_wrapper.jl"))

@testset "GIAC Wrapper Eval Tests" begin
    @testset "Version Functions" begin
        # giac_version() returns CxxWrap.StdLib.StdStringAllocated, not a
        # Base.String; AbstractString accepts both.
        @test giac_version() isa AbstractString
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
        @test giac_eval(ctx, "1+1") == "2"
        @test giac_eval(ctx, "2*3") == "6"
    end

    @testset "Factor Operation" begin
        ctx = GiacContext()
        result = giac_eval(ctx, "factor(x^2-1)")
        @test occursin("x-1", result) || occursin("x+1", result)
    end

    @testset "Error Handling" begin
        # GIAC's parser is permissive: `invalid(((` triggers a warning and
        # returns `undef`, it does NOT throw. `factor()` with no argument
        # does throw, so use it as the exception witness.
        ctx = GiacContext()
        @test_throws Exception giac_eval(ctx, "factor()")
    end
end
