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

    # Issue #3: the new free-function `giac_eval(expr, ctx)` returns a Gen
    # and preserves per-context isolation of `:=` bindings.
    @testset "giac_eval(expr, ctx) — returns Gen and isolates bindings" begin
        ctx1 = GiacContext()
        ctx2 = GiacContext()

        # Bind via the new overload, exercised from Julia.
        r1 = giac_eval("ctx_iso_b := 11", ctx1)
        @test to_string(r1) == "11"

        # ctx1 sees the binding.
        @test to_string(giac_eval("ctx_iso_b", ctx1)) == "11"

        # ctx2 is independent — still the unbound symbol.
        @test to_string(giac_eval("ctx_iso_b", ctx2)) == "ctx_iso_b"
    end

    @testset "Timeout Configuration" begin
        ctx = GiacContext()

        # Default timeout should be 30 seconds
        @test get_timeout(ctx) == 30

        set_timeout(ctx, 60)
        @test get_timeout(ctx) == 60

        # Per-context isolation
        ctx_a = GiacContext()
        ctx_b = GiacContext()
        set_timeout(ctx_a, 60)
        set_timeout(ctx_b, 120)
        @test get_timeout(ctx_a) == 60
        @test get_timeout(ctx_b) == 120
        # A fresh context still reads the default — no leak from a or b.
        ctx_c = GiacContext()
        @test get_timeout(ctx_c) == 30
    end

    @testset "Precision Configuration" begin
        ctx = GiacContext()
        set_precision(ctx, 50)
        @test get_precision(ctx) == 50

        # Per-context isolation
        ctx_a = GiacContext()
        ctx_b = GiacContext()
        set_precision(ctx_a, 50)
        set_precision(ctx_b, 20)
        @test get_precision(ctx_a) == 50
        @test get_precision(ctx_b) == 20

        # Effect on evaluation: evalf(pi) at precision 50 → ≥45 digits after the dot
        ctx_50 = GiacContext()
        set_precision(ctx_50, 50)
        pi_str = to_string(giac_eval("evalf(pi)", ctx_50))
        dot = findfirst('.', pi_str)
        @test dot !== nothing
        digits_after = count(isdigit, pi_str[dot+1:end])
        @test digits_after >= 45
    end

    @testset "Complex Mode" begin
        ctx = GiacContext()

        set_complex_mode(ctx, true)
        @test is_complex_mode(ctx) == true

        set_complex_mode(ctx, false)
        @test is_complex_mode(ctx) == false

        # Per-context isolation
        ctx_a = GiacContext()
        ctx_b = GiacContext()
        set_complex_mode(ctx_a, true)
        set_complex_mode(ctx_b, false)
        @test is_complex_mode(ctx_a) == true
        @test is_complex_mode(ctx_b) == false
        set_complex_mode(ctx_a, false)
        @test is_complex_mode(ctx_b) == false  # ctx_b unaffected

        # Effect on evaluation: factor(x^2+1) splits over complex only in complex mode.
        # sqrt(-1) returns "i" regardless of complex_mode in this giac version,
        # so factor (or solve) is the reliable witness for the flag's evaluator effect.
        ctx_on = GiacContext()
        set_complex_mode(ctx_on, true)
        @test occursin("i", to_string(giac_eval("factor(x^2+1)", ctx_on)))

        ctx_off = GiacContext()
        set_complex_mode(ctx_off, false)
        @test !occursin("i", to_string(giac_eval("factor(x^2+1)", ctx_off)))
    end
end
