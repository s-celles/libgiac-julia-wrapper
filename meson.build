project('giac_wrapper', 'cpp',
  version: '0.5.0',
  meson_version: '>= 1.2.0',
  license: 'GPL-3.0-or-later',
  default_options: [
    'cpp_std=c++17',
    'warning_level=2',
    'buildtype=release',
  ],
)

# Find dependencies
# JlCxx (libcxxwrap-julia) ships CMake config files, use Meson's cmake module
jlcxx_dep = dependency('JlCxx',
  method: 'cmake',
  modules: ['JlCxx::cxxwrap_julia', 'JlCxx::cxxwrap_julia_stl'],
  required: true,
)

# GIAC library
giac_dep = dependency('giac', required: false)
if not giac_dep.found()
  # Fallback: manual find
  cpp = meson.get_compiler('cpp')
  giac_lib = cpp.find_library('giac', required: true)
  giac_inc = include_directories(
    get_option('giac_include_dir'),
    is_system: true,
  )
  giac_dep = declare_dependency(
    dependencies: giac_lib,
    include_directories: giac_inc,
  )
endif

# GMP library (required for direct mpz_* calls in ZINT handling)
gmp_dep = dependency('gmp', required: false)
if not gmp_dep.found()
  cpp = meson.get_compiler('cpp')
  gmp_dep = cpp.find_library('gmp', required: true)
endif

subdir('src')
subdir('tests/cpp')

# Print configuration summary
summary({
  'Version': meson.project_version(),
  'Build type': get_option('buildtype'),
  'C++ standard': get_option('cpp_std'),
}, section: 'Configuration')
