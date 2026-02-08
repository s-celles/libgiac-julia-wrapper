cmake_minimum_required(VERSION 3.15)
project(GiacWrapper VERSION 0.1.0 LANGUAGES CXX)

# C++17 required by libcxxwrap-julia
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type defaults to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Add cmake module path for FindGIAC.cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find required packages
find_package(JlCxx REQUIRED)
find_package(GIAC REQUIRED)

# Get Julia executable for include paths
get_target_property(JlCxx_location JlCxx::cxxwrap_julia LOCATION)
get_filename_component(JlCxx_location ${JlCxx_location} DIRECTORY)

# Add subdirectories
add_subdirectory(src)

# Enable testing
enable_testing()
add_subdirectory(tests/cpp)

# Print configuration summary
message(STATUS "")
message(STATUS "GiacWrapper Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  GIAC library: ${GIAC_LIBRARIES}")
message(STATUS "  JlCxx library: ${JlCxx_location}")
message(STATUS "")
