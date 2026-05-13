# libgiac-julia-wrapper

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/s-celles/libgiac-julia-wrapper)
[![CI](https://github.com/s-celles/libgiac-julia-wrapper/actions/workflows/ci.yml/badge.svg)](https://github.com/s-celles/libgiac-julia-wrapper/actions/workflows/ci.yml)

C++ wrapper exposing the [GIAC](https://xcas.univ-grenoble-alpes.fr/) computer algebra system to Julia via [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl). Normally consumed through [Giac.jl](https://github.com/s-celles/Giac.jl), which pulls a pre-built binary of this wrapper from [`libgiac_julia_jll`](https://github.com/JuliaPackaging/Yggdrasil/tree/master/L/libgiac_julia) (built by BinaryBuilder against `GIAC_jll`'s runtime). This repository is what you build locally if you are contributing to the wrapper itself.

## Features

### Evaluation

- String-based evaluation through a free function `giac_eval(expr)` that routes through a process-wide thread-local default context.
- Per-context evaluation via `giac_eval(expr, ctx)` so distinct `GiacContext` instances isolate `:=` bindings and per-context configuration ([#3](https://github.com/s-celles/libgiac-julia-wrapper/issues/3)).
- Pre-parsed evaluation through `Gen::eval()`, `Gen::simplify()`, `Gen::expand()`, `Gen::factor()`.

### Function dispatch

- **Tier 1 direct wrappers** for ~25 common operations (skip the name-lookup step): `giac_sin/cos/tan/asin/acos/atan`, `giac_exp/ln/log10/sqrt`, `giac_abs/sign/floor/ceil`, `giac_re/im/conj`, `giac_normal/evalf`, `giac_diff/integrate/subst/solve/limit/series`, `giac_gcd/lcm/pow`.
- **Tier 2 generic dispatch** by giac function name: `apply_func0/1/2/3/N` — calls any giac builtin or user-registered function with 0/1/2/3/N arguments.

### Gen — opaque `giac::gen` wrapper

- Construction helpers: `Gen(string)`, `Gen(Int64)`, `Gen(Float64)`, `make_identifier`, `make_complex`, `make_fraction`, `make_vect`, `make_zint_from_bytes`, `make_symbolic_unevaluated`.
- Typed accessors: `to_int64/int32/double`, `zint_to_bytes/sign/string`, `cplx_re/im`, `frac_num/den`, `vect_size/at`, `symb_sommet_name/feuille`, `idnt_name`, `strng_value`, `map_size/keys/values`, `type/subtype/type_name`.
- Value predicates: `is_zero`, `is_one`, `is_integer`, `is_approx`. Type predicates: `is_numeric`, `is_vector`, `is_symbolic`, `is_identifier`, `is_fraction`, `is_complex`, `is_string`.
- Operators: `+ - * /` and unary `-` with mixed-type overloads against Julia `Int64` and `Float64`; `==` / `!=`.
- Direct pointer plumbing for zero-copy interop with Julia: `gen_to_heap_ptr`, `gen_from_heap_ptr`, `free_gen_ptr`, `gen_ptr_to_string`, `gen_ptr_type`.

### Help / introspection

- Pre-loaded command database with `init_help(path_to_aide_cas)` so giac never falls back to filesystem-search paths.
- `list_commands()`, `help_count()`, `list_builtin_functions()`, `builtin_function_count()`, `list_all_functions()` for command-table inspection.

### Other

- Warning handler hooks: `set_warning_handler`, `clear_warning_handler` for routing giac warnings into custom callbacks.
- Config: `set_xcasroot/get_xcasroot`. Per-context `set_variable/get_variable` are implemented; `set_timeout/set_precision/set_complex_mode` are accepted but currently stubs (see TODOs in `src/giac_impl.cpp`).
- **Linux and macOS** in CI (Ubuntu + macOS) are fully green. **Windows has a known ABI issue** that affects production Julia usage, not just CI:
  - In CI, Windows builds the wrapper locally under MSYS2 (currently GCC 15.2) and links against `GIAC_jll` (BinaryBuilder GCC 8). The mismatch with the artifact's runtime DLLs makes the test step fail, so it runs as `continue-on-error`.
  - For end users on Windows, the production path is `libgiac_julia_jll` (BinaryBuilder GCC 10) + `GIAC_jll` (BinaryBuilder GCC 8). Even though both come from BinaryBuilder, a bitfield layout difference in giac's `gen` struct still leaks across the boundary: MPFR reals come back tagged as `_DOUBLE_` instead of `_REAL_`. Surfaced while reviewing [Giac.jl#22](https://github.com/s-celles/Giac.jl/pull/22); a string-length heuristic in Giac.jl is the current workaround. A proper fix likely requires rebuilding `GIAC_jll` with the same `preferred_gcc_version` as `libgiac_julia_jll`.

## Requirements

- **Julia 1.10+** (LTS supported; CI runs on 1.10).
- **Meson 1.2+** and **Ninja** as the build system.
- **C++17 compiler** — GCC 7+ or Clang 5+ (required by libcxxwrap-julia).
- **CMake** — used by Meson to discover JlCxx (libcxxwrap-julia).
- **GMP** and **MPFR** development headers (`libgmp-dev libmpfr-dev` on Debian/Ubuntu, `gmp mpfr` on Homebrew, `mingw-w64-x86_64-{gmp,mpfr}` on MSYS2).
- **gettext / libintl** development headers on macOS (`brew install gettext`); already part of glibc on Linux.
- **GIAC** itself — one of:
  - System install: `libgiac-dev` on Debian/Ubuntu, or build giac from source. Default include path is `/usr/include/giac`.
  - **`GIAC_jll`** (Julia binary artifact, recommended — this is what CI uses): `julia -e 'using Pkg; Pkg.add("GIAC_jll")'`, then pass the artifact path via `-Dgiac_include_dir=$(julia -e 'using GIAC_jll; print(GIAC_jll.artifact_dir)')/include/giac` to Meson.

The wrapper is tested against giac 2.0.x. The version reported at runtime by `wrapper_version()` is driven from `meson.project_version()` so it never drifts from the build ([#2](https://github.com/s-celles/libgiac-julia-wrapper/issues/2)).

## Building

### Using GIAC_jll (recommended — matches CI)

```bash
julia -e 'using Pkg; Pkg.add(["CxxWrap", "GIAC_jll"])'
CXXWRAP_PREFIX=$(julia -e 'using CxxWrap; print(CxxWrap.prefix_path())')
GIAC_ARTIFACT=$(julia -e 'using GIAC_jll; print(GIAC_jll.artifact_dir)')
meson setup builddir \
  --cmake-prefix-path="$CXXWRAP_PREFIX" \
  -Dgiac_include_dir="$GIAC_ARTIFACT/include/giac"
meson compile -C builddir
```

### Using a system giac install

```bash
julia -e 'using Pkg; Pkg.add("CxxWrap")'
meson setup builddir \
  --cmake-prefix-path="$(julia -e 'using CxxWrap; print(CxxWrap.prefix_path())')"
meson compile -C builddir
```

(Defaults to `/usr/include/giac`; override with `-Dgiac_include_dir=...` if needed.)

### Using `just`

```bash
julia -e 'using Pkg; Pkg.add("CxxWrap")'
just setup
just build
```

The `justfile` runs the system-giac path above; for the GIAC_jll path, use the explicit `meson setup` command.

## Testing

```bash
meson test -C builddir
```

Or `just test`. This runs the C++ test suites (`test_eval`, `test_context`, `test_gen`, `test_extraction`, `test_predicates`, `test_warnings`) — 6 suites total, all green on Linux and macOS. The `tests/julia/` directory contains standalone Julia integration scripts that are not currently wired into `meson test`; downstream coverage from Julia lives in [Giac.jl](https://github.com/s-celles/Giac.jl).

## Usage from Julia (direct)

Most consumers should use [Giac.jl](https://github.com/s-celles/Giac.jl) for an idiomatic Julia API. To talk to the wrapper directly:

```julia
# Run from the repository root after `meson compile -C builddir`.
using CxxWrap
const libgiac = joinpath(pwd(), "builddir", "src", "libgiac_wrapper")
@wrapmodule(() -> libgiac)
@initcxx

# String-based eval against the default context
r = giac_eval("sin(x)+1")
println(to_string(r))                 # "sin(x)+1"

# Per-context isolation
ctx1 = GiacContext()
ctx2 = GiacContext()
giac_eval("a := 5", ctx1)
to_string(giac_eval("a", ctx1))       # "5"
to_string(giac_eval("a", ctx2))       # "a" — ctx2 never saw the binding
```

## Related projects

- [Giac.jl](https://github.com/s-celles/Giac.jl) — Julia package providing the idiomatic API on top of this wrapper.
- [GIAC](https://xcas.univ-grenoble-alpes.fr/) / [Xcas](https://en.wikipedia.org/wiki/Xcas) — the underlying CAS.
- [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl) / [libcxxwrap-julia](https://github.com/JuliaInterop/libcxxwrap-julia) — the C++/Julia interop layer.

## License

GPL-3.0-or-later. See [LICENSE](LICENSE) for the full text.
