cmake_minimum_required(VERSION 3.30.0)
project(Physarum VERSION 0.5.0 LANGUAGES CXX)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

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

#----- FFTW3 ----- 

FetchContent_Declare(
    fftw3
    URL      https://www.fftw.org/fftw-3.3.10.tar.gz
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
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

# Add Windows-specific OpenGL libraries
if(WIN32)
    set(ADDITIONAL_WINDOWS_LIBS opengl32 gdi32 setupapi version imm32)
endif()

find_package(OpenGL REQUIRED)


#Create symbolic link of resource folders for the executable in the build folder
if(NOT WIN32)
    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/res" "${CMAKE_SOURCE_DIR}/build/res" SYMBOLIC RESULT result)
    if(result)
        message(FATAL_ERROR "Failed to create symlink to Linux build directory")
    endif()

    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/presets" "${CMAKE_SOURCE_DIR}/build/presets" SYMBOLIC RESULT result)
    if(result)
        message(FATAL_ERROR "Failed to create symlink to Linux build directory")
    endif()
endif()

if(WIN32)
    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/res" "${CMAKE_SOURCE_DIR}/build_windows/res" SYMBOLIC RESULT result)
    if(result)
        message(FATAL_ERROR "Failed to create symlink to Windows build directory")
    endif()

    file(CREATE_LINK "${CMAKE_SOURCE_DIR}/presets" "${CMAKE_SOURCE_DIR}/build_windows/presets" SYMBOLIC RESULT result)
    if(result)
        message(FATAL_ERROR "Failed to create symlink to Windows build directory")
    endif()
endif()


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
    ${SDL3_INCLUDE_DIR}
)

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
    source/canvas.h
    source/display.h
    source/ParticleData.h
    source/graphics/Shader.h
    source/graphics/ShaderProgram.h
    source/graphics/FrameBuffer.h
    source/graphics/Texture.h
    source/ui/UserInterface.h
    source/audio/AudioRecording.h
    source/audio/AudioProcessor.h
    source/audio/MusicAnalysis.h
    source/PresetSystem.h
    source/ColorPresetSystem.h
    source/Uniforms.h
    source/graphics/UniformBufferObject.h
    source/graphics/Bloom.h
)

set(MY_SOURCE_FILES
    source/main.cpp
    source/canvas.cpp
    source/display.cpp
    source/ParticleData.cpp
    source/graphics/Shader.cpp
    source/graphics/ShaderProgram.cpp
    source/graphics/FrameBuffer.cpp
    source/graphics/Texture.cpp
    source/ui/UserInterface.cpp
    source/audio/AudioRecording.cpp
    source/audio/AudioProcessor.cpp
    source/audio/MusicAnalysis.cpp
    source/PresetSystem.cpp
    source/ColorPresetSystem.cpp
    source/graphics/UniformBufferObject.cpp
    source/graphics/Bloom.cpp
)

include_directories(
    ${SDL3_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIR}
    ${GLM_INCLUDE_PATH}
    ${FFTW3_INCLUDE_DIR}
)

add_executable(Physarum
    ${MY_HEADER_FILES}
    ${MY_SOURCE_FILES}
)

target_link_libraries(Physarum
    PUBLIC ImGui
    ${OPENGL_LIBRARY}
    SDL3::SDL3-static
    glew_s
    fftw3
    ${ADDITIONAL_WINDOWS_LIBS}  # Add the Windows libraries
)
