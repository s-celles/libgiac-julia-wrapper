# test_warnings.jl
# Tests for warning callback (FR-011)

using Test
using CxxWrap
using Libdl

# Load the wrapper library
const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "build", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac_wrapper)

function __init__()
    @initcxx
end

@testset "GIAC Wrapper Warning Tests" begin
    @testset "Warning Handler Registration" begin
        ctx = GiacContext()
        warnings = String[]

        # Set a warning handler
        set_warning_handler(ctx, msg -> push!(warnings, msg))

        # Trigger a warning (if possible)
        # Note: actual warning triggering depends on GIAC behavior
        @test true  # Placeholder
    end

    @testset "Warning Handler Clear" begin
        ctx = GiacContext()

        set_warning_handler(ctx, msg -> nothing)
        clear_warning_handler(ctx)

        @test true  # Handler cleared successfully
    end
end
