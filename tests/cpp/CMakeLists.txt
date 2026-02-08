# tests/cpp/CMakeLists.txt
# CTest configuration for C++ unit tests

# Find GTest or use a simple test framework
include(CTest)

# Create test executables
# Test for expression evaluation (User Story 1)
add_executable(test_eval test_eval.cpp)
target_link_libraries(test_eval PRIVATE giac_wrapper ${GIAC_LIBRARIES})
target_include_directories(test_eval PRIVATE ${CMAKE_SOURCE_DIR}/src ${GIAC_INCLUDE_DIRS})
add_test(NAME TestEval COMMAND test_eval)

# Test for context management (User Story 2)
add_executable(test_context test_context.cpp)
target_link_libraries(test_context PRIVATE giac_wrapper ${GIAC_LIBRARIES})
target_include_directories(test_context PRIVATE ${CMAKE_SOURCE_DIR}/src ${GIAC_INCLUDE_DIRS})
add_test(NAME TestContext COMMAND test_context)

# Test for Gen objects (User Story 3)
add_executable(test_gen test_gen.cpp)
target_link_libraries(test_gen PRIVATE giac_wrapper ${GIAC_LIBRARIES})
target_include_directories(test_gen PRIVATE ${CMAKE_SOURCE_DIR}/src ${GIAC_INCLUDE_DIRS})
add_test(NAME TestGen COMMAND test_gen)

# Test for warnings (FR-011)
add_executable(test_warnings test_warnings.cpp)
target_link_libraries(test_warnings PRIVATE giac_wrapper ${GIAC_LIBRARIES})
target_include_directories(test_warnings PRIVATE ${CMAKE_SOURCE_DIR}/src ${GIAC_INCLUDE_DIRS})
add_test(NAME TestWarnings COMMAND test_warnings)

# Set test properties
set_tests_properties(TestEval TestContext TestGen TestWarnings
    PROPERTIES
        TIMEOUT 60
        ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/src:$ENV{LD_LIBRARY_PATH}"
)
