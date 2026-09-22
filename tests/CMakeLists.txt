cmake_minimum_required(VERSION 3.30)
project(PhysarumTests)

# make sure to use a Debug build
set(CMAKE_BUILD_TYPE "Debug")


include(Catch)
include(CTest)

set(TEST_SOURCE_FILES
#Observer Pattern Tests
unit/test_observer_pattern.cpp
${CMAKE_SOURCE_DIR}/source/utility/event.cpp
${CMAKE_SOURCE_DIR}/source/utility/observable.cpp
${CMAKE_SOURCE_DIR}/source/utility/observer.cpp
#Parameter Parser Tests
unit/test_parameter_parser.cpp
${CMAKE_SOURCE_DIR}/source/utility/parameter_parser.cpp
)

add_executable(PhysarumTests ${TEST_SOURCE_FILES})

target_link_options(PhysarumTests PRIVATE -coverage -fprofile-arcs -ftest-coverage)
target_compile_options(PhysarumTests PRIVATE -coverage -fprofile-arcs -ftest-coverage)

target_include_directories(PhysarumTests PRIVATE 
${CMAKE_SOURCE_DIR}/source
${imgui_SOURCE_DIR}
)

set_target_properties(PhysarumTests PROPERTIES
CXX_STANDARD 23
CXX_STANDARD_REQUIRED YES
)

target_link_libraries(PhysarumTests PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(PhysarumTests)

add_test(NAME coverage COMMAND PhysarumTests)