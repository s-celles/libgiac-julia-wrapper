# runtests.jl
# Main test runner for GIAC Julia wrapper

using Test

@testset "GIAC Julia Wrapper" begin
    include("test_eval.jl")
    include("test_context.jl")
    include("test_gen.jl")
    include("test_warnings.jl")
end
