# src/CMakeLists.txt
# Build configuration for the GIAC wrapper library

# Define the wrapper library
add_library(giac_wrapper SHARED
    giac_wrapper.cpp
    context.cpp
    gen.cpp
    warnings.cpp
)

# Set library properties
set_target_properties(giac_wrapper PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    OUTPUT_NAME "giac_wrapper"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Include directories
target_include_directories(giac_wrapper
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${GIAC_INCLUDE_DIRS}
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

install(FILES giac_wrapper.h
    DESTINATION include/giac_julia
)
