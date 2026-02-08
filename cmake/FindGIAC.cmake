# FindGIAC.cmake
# Find the GIAC computer algebra system library
#
# This module defines:
#   GIAC_FOUND        - True if GIAC was found
#   GIAC_INCLUDE_DIRS - GIAC include directories
#   GIAC_LIBRARIES    - GIAC libraries to link against
#   GIAC_VERSION      - GIAC version string (if available)

# Try pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_GIAC QUIET giac)
endif()

# Find include directory
find_path(GIAC_INCLUDE_DIR
    NAMES giac/giac.h
    HINTS
        ${PC_GIAC_INCLUDEDIR}
        ${PC_GIAC_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
        /opt/homebrew/include
    PATH_SUFFIXES giac
)

# Find library
find_library(GIAC_LIBRARY
    NAMES giac
    HINTS
        ${PC_GIAC_LIBDIR}
        ${PC_GIAC_LIBRARY_DIRS}
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /usr/local/lib64
        /opt/local/lib
        /opt/homebrew/lib
)

# Get version from pkg-config or header
if(PC_GIAC_VERSION)
    set(GIAC_VERSION ${PC_GIAC_VERSION})
elseif(GIAC_INCLUDE_DIR AND EXISTS "${GIAC_INCLUDE_DIR}/giac/config.h")
    file(STRINGS "${GIAC_INCLUDE_DIR}/giac/config.h" GIAC_VERSION_LINE
         REGEX "#define GIAC_VERSION")
    if(GIAC_VERSION_LINE)
        string(REGEX REPLACE ".*\"(.*)\".*" "\\1" GIAC_VERSION "${GIAC_VERSION_LINE}")
    endif()
endif()

# Set output variables
set(GIAC_INCLUDE_DIRS ${GIAC_INCLUDE_DIR})
set(GIAC_LIBRARIES ${GIAC_LIBRARY})

# Handle standard find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GIAC
    REQUIRED_VARS GIAC_LIBRARY GIAC_INCLUDE_DIR
    VERSION_VAR GIAC_VERSION
)

# Create imported target
if(GIAC_FOUND AND NOT TARGET GIAC::giac)
    add_library(GIAC::giac UNKNOWN IMPORTED)
    set_target_properties(GIAC::giac PROPERTIES
        IMPORTED_LOCATION "${GIAC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GIAC_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(GIAC_INCLUDE_DIR GIAC_LIBRARY)
