# src/CMakeLists.txt
# Build configuration for the GIAC wrapper library

# Define the wrapper library
# Note: giac_impl.cpp includes GIAC headers, giac_wrapper.cpp includes jlcxx headers
# They are kept separate to avoid macro conflicts between GIAC and Julia
add_library(giac_wrapper SHARED
    giac_impl.cpp
    giac_wrapper.cpp
)

# Set different C++ standards for different source files:
# - giac_impl.cpp uses C++14 (GIAC uses deprecated features removed in C++17)
# - giac_wrapper.cpp uses C++17 (required by jlcxx)
set_source_files_properties(giac_impl.cpp PROPERTIES
    COMPILE_FLAGS "-std=c++14"
)
set_source_files_properties(giac_wrapper.cpp PROPERTIES
    COMPILE_FLAGS "-std=c++17"
)

# Set library properties
# Note: Using default visibility for now to allow tests to link
# TODO: Add proper export macros for production builds
set_target_properties(giac_wrapper PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    OUTPUT_NAME "giac_wrapper"
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

# Include directories
# Note: GIAC headers include mpfr.h and gmp.h which are bundled in the same directory
target_include_directories(giac_wrapper
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GIAC_INCLUDE_DIRS}
        ${GIAC_INCLUDE_DIRS}/giac  # For bundled mpfr.h and gmp.h
)

# Link libraries
target_link_libraries(giac_wrapper
    PUBLIC
        JlCxx::cxxwrap_julia
    PRIVATE
        ${GIAC_LIBRARIES}
)

# Compiler options
target_compile_options(giac_wrapper PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra -Wpedantic>
)

# Install rules
install(TARGETS giac_wrapper
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES giac_impl.h
    DESTINATION include/giac_julia
)
