# test_warnings.jl
# Tests for warning callback (FR-011)

include(joinpath(@__DIR__, "load_wrapper.jl"))

@testset "GIAC Wrapper Warning Tests" begin
    # The C++ class has set_warning_handler / clear_warning_handler taking a
    # std::function<void(const std::string&)>, but those methods are NOT
    # currently registered with CxxWrap in src/giac_wrapper.cpp — CxxWrap
    # does not handle the std::function argument transparently. Until the
    # wrapper exposes a compatible callback registration mechanism, these
    # tests are marked broken.

    @testset "Warning Handler Registration" begin
        ctx = GiacContext()
        @test_broken isdefined(@__MODULE__, :set_warning_handler)
    end

    @testset "Warning Handler Clear" begin
        ctx = GiacContext()
        @test_broken isdefined(@__MODULE__, :clear_warning_handler)
    end
end
