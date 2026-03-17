# src/meson.build
# Build configuration for the GIAC wrapper library

giac_wrapper_sources = files(
  'giac_impl.cpp',
  'giac_wrapper.cpp',
)

giac_wrapper_lib = shared_library('giac_wrapper',
  giac_wrapper_sources,
  dependencies: [jlcxx_dep, giac_dep, gmp_dep],
  include_directories: include_directories('.'),
  install: true,
  version: meson.project_version(),
  soversion: '0',
)

# For tests and other subprojects to link against
giac_wrapper_dep = declare_dependency(
  link_with: giac_wrapper_lib,
  include_directories: include_directories('.'),
  dependencies: [giac_dep, gmp_dep],
)

# Install headers
install_headers('giac_impl.h', subdir: 'giac_julia')
