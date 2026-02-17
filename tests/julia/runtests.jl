# runtests.jl
# Main test runner for GIAC Julia wrapper

using Test

@testset "GIAC Julia Wrapper" begin
    include("test_eval.jl")
    include("test_context.jl")
    include("test_gen.jl")
    include("test_warnings.jl")
    include("test_introspection.jl")
    include("test_indexing.jl")
    include("test_iteration.jl")
    include("test_to_julia.jl")
    include("test_display.jl")
    include("test_symbolic.jl")
end
