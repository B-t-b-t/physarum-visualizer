cmake_minimum_required(VERSION 3.30.0)
project(Physarum VERSION 0.5.0 LANGUAGES CXX)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE " -O3")
# force static linking for C/C++ runtime
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

#====== Dependency Management =====

include(FetchContent)

#----- Glew ----- 

FetchContent_Declare(
    glew
    URL https://github.com/nigels-com/glew/releases/download/glew-2.3.1/glew-2.3.1.tgz
)
set(GLEW_STATIC ON CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
FetchContent_MakeAvailable(glew)
add_subdirectory(${glew_SOURCE_DIR}/build/cmake ${glew_BINARY_DIR})
unset(CMAKE_POLICY_VERSION_MINIMUM CACHE)

#----- SDL3 ----- 

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG release-3.4.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

#----- SDL_image ----- 

FetchContent_Declare(
    SDL_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
    GIT_TAG release-3.4.2
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL_image)


#----- FFTW3 ----- 

FetchContent_Declare(
    fftw3
    URL      https://www.fftw.org/fftw-3.3.10.tar.gz
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_FLOAT OFF CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
FetchContent_MakeAvailable(fftw3)
unset(CMAKE_POLICY_VERSION_MINIMUM CACHE)

#----- ImGui ----- 

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.92.6-docking
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(imgui)

#----- ImPlot ----- 

FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot
    GIT_TAG v0.17
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(implot)

#temporarily use a pullrequest to fix C++23 compile warnings
FetchContent_Declare(
    toml11
    GIT_REPOSITORY https://github.com/deckstose/toml11
    GIT_TAG push-usxyownxrltn
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(toml11)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v3.14.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
add_subdirectory(tests)

# platform-specific libraries
if(UNIX)
    set(ADDITIONAL_LINUX_LIBS dl X11)
endif()

if(WIN32)
    set(ADDITIONAL_WINDOWS_LIBS opengl32 gdi32 setupapi version imm32)
endif()


find_package(OpenGL REQUIRED)


#Create symbolic link of resource folders for the executable in the build folder

foreach(BUILD_DIR "build/linux/debug" "build/linux/release" "build/windows/debug" "build/windows/release")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/${BUILD_DIR}")
    #res symlink
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${BUILD_DIR}/res")
        file(CREATE_LINK "${CMAKE_SOURCE_DIR}/res" "${CMAKE_SOURCE_DIR}/${BUILD_DIR}/res" SYMBOLIC RESULT result)
        if(result)
            message(FATAL_ERROR "Failed to create symlink to ${BUILD_DIR}/res")
        endif()
    endif()
    #presets symlink
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${BUILD_DIR}/presets")
        file(CREATE_LINK "${CMAKE_SOURCE_DIR}/presets" "${CMAKE_SOURCE_DIR}/${BUILD_DIR}/presets" SYMBOLIC RESULT result)
        if(result)
            message(FATAL_ERROR "Failed to create symlink to ${BUILD_DIR}/presets")
        endif()
    endif()
endforeach()



set(IMGUI_HEADER_FILES
    ${imgui_SOURCE_DIR}/imconfig.h
    ${imgui_SOURCE_DIR}/imgui.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3_loader.h
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.h
    ${imgui_SOURCE_DIR}/imgui_internal.h
    ${imgui_SOURCE_DIR}/imstb_rectpack.h
    ${imgui_SOURCE_DIR}/imstb_textedit.h
    ${imgui_SOURCE_DIR}/imstb_truetype.h
    ${implot_SOURCE_DIR}/implot.h
    ${implot_SOURCE_DIR}/implot_internal.h
)

set(IMGUI_SOURCE_FILES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
)

add_library("ImGui" STATIC ${IMGUI_SOURCE_FILES})

target_include_directories("ImGui" SYSTEM PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${implot_SOURCE_DIR}
)

target_link_libraries("ImGui" PUBLIC SDL3::SDL3-static)

# Add Windows-specific defines for ImGui
if(WIN32)
    target_compile_definitions("ImGui" PRIVATE
        -D_WIN32
        -DWIN32
        -DIMGUI_IMPL_WIN32_DISABLE_GAMEPAD
        -DWINDOWS_LEAN_AND_MEAN
    )
endif()

# Disable warnings for ImGui
target_compile_options("ImGui" PRIVATE
    -w
)

# compiler flags for strict warnings
add_compile_options(-Werror -Wall -Wextra -Wpedantic -Wfloat-equal -Wsign-conversion -Wshadow)

set(GLM_INCLUDE_PATH PATH ${CMAKE_CURRENT_SOURCE_DIR}/glm)

set(MY_HEADER_FILES
    source/application.h
    source/application_state.h
    source/canvas.h
    source/color_preset_system.h
    source/particle_data.h
    source/preset_system.h
    source/uniforms.h
    source/window.h
    source/audio/audio_device_manager.h
    source/audio/audio_processor.h
    source/audio/audiostream_data.h
    source/audio/audio_system.h
    source/audio/music_analysis.h
    source/graphics/bloom.h
    source/graphics/framebuffer.h
    source/graphics/renderer.h
    source/graphics/shader.h
    source/graphics/shader_program.h
    source/graphics/texture.h
    source/graphics/uniform_buffer_manager.h
    source/graphics/uniform_buffer_object.h
    source/simulation/simulation.h
    source/simulation/trail_map_controller.h
    source/ui/ui_helpers.h
    source/ui/user_interface.h
    source/ui/elements/audio_window.h
    source/ui/elements/debug_window.h
    source/ui/elements/imgui_window.h
    source/ui/elements/new_canvas_modal.h
    source/ui/elements/preset_window.h
    source/ui/elements/slime_config_window.h
    source/ui/elements/visual_settings_window.h
    source/utility/fileHandling.h
    source/utility/parameter_parser.h
    source/utility/observable.h
    source/utility/observer.h
)

set(MY_SOURCE_FILES
    source/main.cpp
    source/application.cpp
    source/application_state.cpp
    source/canvas.cpp
    source/color_preset_system.cpp
    source/particle_data.cpp
    source/preset_system.cpp
    source/window.cpp
    source/audio/music_analysis.cpp
    source/audio/audio_device_manager.cpp
    source/audio/audiostream_data.cpp
    source/audio/audio_system.cpp
    source/audio/audio_processor.cpp
    source/graphics/framebuffer.cpp
    source/graphics/bloom.cpp
    source/graphics/renderer.cpp
    source/graphics/shader.cpp
    source/graphics/shader_program.cpp
    source/graphics/texture.cpp
    source/graphics/uniform_buffer_manager.cpp
    source/graphics/uniform_buffer_object.cpp
    source/simulation/simulation.cpp
    source/simulation/trail_map_controller.cpp
    source/ui/ui_helpers.cpp
    source/ui/user_interface.cpp
    source/ui/elements/audio_window.cpp
    source/ui/elements/debug_window.cpp
    source/ui/elements/new_canvas_modal.cpp
    source/ui/elements/preset_window.cpp
    source/ui/elements/slime_config_window.cpp
    source/ui/elements/visual_settings_window.cpp
    source/utility/fileHandling.cpp
    source/utility/parameter_parser.cpp
    source/utility/observable.cpp
    source/utility/observer.cpp
)

include_directories(
    ${SDL3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${GLM_INCLUDE_PATH}
    ${FFTW3_INCLUDE_DIR}
    ${toml11_SOURCE_DIR}/include
    ${fftw3_SOURCE_DIR}/api
    ${glew_SOURCE_DIR}/include
)

add_executable(Physarum
    ${MY_HEADER_FILES}
    ${MY_SOURCE_FILES}
)

target_compile_definitions(Physarum PUBLIC $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:Release>:NDEBUG>)

target_link_libraries(Physarum
    PUBLIC ImGui
    ${OPENGL_LIBRARY}
    SDL3::SDL3-static
    SDL3_image::SDL3_image-static
    glew_s
    fftw3
    PRIVATE toml11::toml11
    ${ADDITIONAL_WINDOWS_LIBS}  # Add the Windows libraries
    ${ADDITIONAL_LINUX_LIBS}    # Add the Linux libraries (dl, X11)
)
