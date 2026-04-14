# libgiac-julia-wrapper

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/s-celles/libgiac-julia-wrapper)
[![CI](https://github.com/s-celles/libgiac-julia-wrapper/actions/workflows/ci.yml/badge.svg)](https://github.com/s-celles/libgiac-julia-wrapper/actions/workflows/ci.yml)

C++ wrapper for [GIAC](https://xcas.univ-grenoble-alpes.fr/) computer algebra system, designed for Julia integration via [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl).

## Features

- String-based expression evaluation
- Context management for variable persistence
- Native Gen object manipulation
- Support for Linux, macOS, and Windows

## Requirements

- GIAC library (libgiac-dev)
- Julia 1.10+
- Meson 1.2+
- Ninja
- C++17 compiler

## Building

Using [just](https://github.com/casey/just):

```bash
julia -e 'using Pkg; Pkg.add("CxxWrap")'
just setup
just build
```

Or manually with Meson:

```bash
julia -e 'using Pkg; Pkg.add("CxxWrap")'
meson setup builddir --cmake-prefix-path="$(julia -e 'using CxxWrap; print(CxxWrap.prefix_path())')"
meson compile -C builddir
```

## Testing

```bash
just test
```

Or manually:

```bash
meson test -C builddir
```

## License

See LICENSE file.
