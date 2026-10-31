cmake_minimum_required(VERSION 3.30)
project(PhysarumTests)

# make sure to use a Debug build
set(CMAKE_BUILD_TYPE "Debug")


include(Catch)
include(CTest)
find_package(OpenGL REQUIRED)

set(TEST_SOURCE_FILES
unit/simulation/test_trail_mask_animation.cpp
${CMAKE_SOURCE_DIR}/source/simulation/trail_mask_animation.cpp

#Scene and trail mask tests
unit/simulation/test_scene_trail_mask.cpp
${CMAKE_SOURCE_DIR}/source/application_state.cpp
${CMAKE_SOURCE_DIR}/source/simulation/scene.cpp
${CMAKE_SOURCE_DIR}/source/simulation/scene_controller.cpp
${CMAKE_SOURCE_DIR}/source/simulation/trail_map_controller.cpp
${CMAKE_SOURCE_DIR}/source/simulation/trail_mask.cpp
${CMAKE_SOURCE_DIR}/source/graphics/font_atlas.cpp
${CMAKE_SOURCE_DIR}/source/graphics/framebuffer.cpp
${CMAKE_SOURCE_DIR}/source/graphics/shader.cpp
${CMAKE_SOURCE_DIR}/source/graphics/shader_program.cpp
${CMAKE_SOURCE_DIR}/source/graphics/text_texture.cpp
${CMAKE_SOURCE_DIR}/source/graphics/texture.cpp
${CMAKE_SOURCE_DIR}/source/utility/fileHandling.cpp

#Observer Pattern Tests
unit/test_observer_pattern.cpp
${CMAKE_SOURCE_DIR}/source/utility/event.cpp
${CMAKE_SOURCE_DIR}/source/utility/observable.cpp
${CMAKE_SOURCE_DIR}/source/utility/observer.cpp
#Vector Math Tests
unit/utility/test_vector_math.cpp
${CMAKE_SOURCE_DIR}/source/utility/vector_math.cpp
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
${glew_SOURCE_DIR}/include
)

target_compile_definitions(PhysarumTests PRIVATE
TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/res"
)

set_target_properties(PhysarumTests PROPERTIES
CXX_STANDARD 23
CXX_STANDARD_REQUIRED YES
)

target_link_libraries(PhysarumTests PRIVATE
Catch2::Catch2WithMain ImGui OpenGL::GL SDL3::SDL3-static SDL3_image-static glew_s toml11::toml11
)

catch_discover_tests(PhysarumTests)

add_test(NAME coverage COMMAND PhysarumTests)