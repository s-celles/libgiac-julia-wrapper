# src/meson.build
# Build configuration for the GIAC wrapper library

giac_wrapper_sources = files(
  'giac_impl.cpp',
  'giac_wrapper.cpp',
)

# On Windows/MinGW, DLLs need --export-all-symbols for tests to link
extra_link_args = []
if host_machine.system() == 'windows'
  extra_link_args += ['-Wl,--export-all-symbols']
endif

giac_wrapper_lib = shared_library('giac_wrapper',
  giac_wrapper_sources,
  dependencies: [jlcxx_dep, giac_dep, gmp_dep, mpfr_dep, intl_dep],
  include_directories: include_directories('.'),
  install: true,
  version: meson.project_version(),
  soversion: '0',
  gnu_symbol_visibility: 'default',
  link_args: extra_link_args,
)

# For tests and other subprojects to link against
giac_wrapper_dep = declare_dependency(
  link_with: giac_wrapper_lib,
  include_directories: include_directories('.'),
  dependencies: [giac_dep, gmp_dep, mpfr_dep, intl_dep],
)

# Install headers
install_headers('giac_impl.h', subdir: 'giac_julia')
