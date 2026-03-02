# src/CMakeLists.txt
# Build configuration for the GIAC wrapper library

# Option: use C-ABI shim for macOS/FreeBSD (bridges g++ GIAC and clang++ CxxWrap)
option(USE_GIAC_CABI "Use C-ABI shim for cross-compiler ABI compatibility" OFF)

if(USE_GIAC_CABI)
    # C-ABI mode: giac_impl.cpp + giac_cabi.cpp are pre-compiled into a static
    # library (libgiac_cabi.a) with g++. Only giac_wrapper.cpp is compiled here
    # with the default (clang++) toolchain. The adapter header redirects all
    # giac_impl.h calls through extern "C" functions in the static lib.

    add_library(giac_wrapper SHARED
        giac_wrapper.cpp
    )

    set_source_files_properties(giac_wrapper.cpp PROPERTIES
        COMPILE_FLAGS "-std=c++17"
    )

    target_compile_definitions(giac_wrapper PRIVATE USE_GIAC_CABI)

    # GIAC_CABI_LIBRARY must point to the pre-built static library
    if(NOT GIAC_CABI_LIBRARY)
        message(FATAL_ERROR "USE_GIAC_CABI=ON requires GIAC_CABI_LIBRARY to be set")
    endif()

    # GCC_RUNTIME_LIBS: full paths to g++'s libstdc++.a and libgcc.a
    # Required because clang++ linker doesn't search g++'s library paths
    if(NOT GCC_RUNTIME_LIBS)
        message(WARNING "GCC_RUNTIME_LIBS not set; libstdc++ may not be found")
    endif()

    # Link the pre-built C-ABI static library and GCC runtime libraries
    target_link_libraries(giac_wrapper
        PUBLIC
            JlCxx::cxxwrap_julia
            JlCxx::cxxwrap_julia_stl
        PRIVATE
            ${GIAC_CABI_LIBRARY}
            ${GIAC_LIBRARIES}
            ${GCC_RUNTIME_LIBS}
    )
else()
    # Standard mode: both files compiled with the same compiler
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

    # Link libraries
    target_link_libraries(giac_wrapper
        PUBLIC
            JlCxx::cxxwrap_julia
            JlCxx::cxxwrap_julia_stl
        PRIVATE
            ${GIAC_LIBRARIES}
    )
endif()

# Set library properties
set_target_properties(giac_wrapper PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    OUTPUT_NAME "giac_wrapper"
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

# Include directories
target_include_directories(giac_wrapper
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GIAC_INCLUDE_DIRS}
        ${GIAC_INCLUDE_DIRS}/giac
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
