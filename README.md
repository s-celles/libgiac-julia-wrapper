# libgiac-julia-wrapper

[![CI](https://github.com/s-celles/libgiac-julia-wrapper/actions/workflows/ci.yml/badge.svg)](https://github.com/s-celles/libgiac-julia-wrapper/actions/workflows/ci.yml)

C++ wrapper for [GIAC](https://xcas.univ-grenoble-alpes.fr/) computer algebra system, designed for Julia integration via [CxxWrap.jl](https://github.com/JuliaInterop/CxxWrap.jl).

## Features

- String-based expression evaluation
- Context management for variable persistence
- Native Gen object manipulation
- Support for Linux and macOS

## Requirements

- GIAC library (libgiac-dev)
- Julia 1.10+
- CMake 3.15+
- C++17 compiler

## Building

```bash
julia -e 'using Pkg; Pkg.add("CxxWrap")'
cmake -B build -DCMAKE_PREFIX_PATH=$(julia -e 'using CxxWrap; print(CxxWrap.prefix_path())')
cmake --build build
```

## Testing

```bash
cd build && ctest --output-on-failure
```

## License

See LICENSE file.
