# load_wrapper.jl
#
# Idempotent loader for the libgiac-julia-wrapper CxxWrap module. Each
# `test_*.jl` file `include`s this script as its first line. The guard
# prevents the second-and-subsequent inclusions from re-running
# `@wrapmodule`, which would otherwise abort with
# `Double registration for method (:function, :giac_version, …)` when the
# whole test suite is driven through `runtests.jl`.

# `using` lives outside the guard so that the `@wrapmodule` / `@initcxx`
# macros are resolvable at parse time. The `using` statements themselves
# are idempotent — re-doing them on the second include is harmless.
using Test
using CxxWrap
using Libdl

if !isdefined(@__MODULE__, :__libgiac_wrapper_loaded__)
    # Meson outputs the shared library to `builddir/src/libgiac_wrapper.so`
    # (the `justfile` runs `meson setup builddir`). Resolve relative to this
    # file so the path works regardless of the caller's cwd.
    const libgiac_wrapper = joinpath(@__DIR__, "..", "..", "builddir", "src", "libgiac_wrapper")
    @wrapmodule(() -> libgiac_wrapper)

    function __init__()
        @initcxx
    end
    # Julia auto-calls __init__ only for Module load events, not for scripts
    # included at top level — so call it explicitly here.
    __init__()

    const __libgiac_wrapper_loaded__ = true
end
