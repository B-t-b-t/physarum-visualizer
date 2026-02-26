cmake_minimum_required(VERSION 3.30.0)
project(Physarum VERSION 0.5.0 LANGUAGES CXX)

cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE " -O3")
# force static linking for C/C++ runtime
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

# Set GLEW variables manually for Windows build
if(WIN32)
    set(GLEW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../libraries/glew-2.2.0-win32/include)
    set(GLEW_LIBRARY_RELEASE ${CMAKE_SOURCE_DIR}/../libraries/glew-2.2.0-win32/lib/Release/x64/glew32s.lib)
    set(GLEW_LIBRARY_DEBUG ${CMAKE_SOURCE_DIR}/../libraries/glew-2.2.0-win32/lib/Release/x64/glew32s.lib)
    set(GLEW_LIBRARIES ${GLEW_LIBRARY_RELEASE})

    # Add definition for static GLEW
    add_definitions(-DGLEW_STATIC)
endif()


# Set SDL3 variables manually for Windows build
if(WIN32)
    set(SDL3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../libraries/SDL3-3.2.8-mingw/x86_64-w64-mingw32/include)
    set(SDL3_LIBRARY_RELEASE ${CMAKE_SOURCE_DIR}/../libraries/SDL3-3.2.8-mingw/x86_64-w64-mingw32/lib/libSDL3.dll.a)
    set(SDL3_LIBRARY_DEBUG ${CMAKE_SOURCE_DIR}/../libraries/SDL3-3.2.8-mingw/x86_64-w64-mingw32/lib/libSDL3.dll.a)
    set(SDL3_LIBRARIES ${SDL3_LIBRARY_RELEASE})
endif()

# Set FFTW3 variables manually for Windows build
if(WIN32)
    set(FFTW3_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../libraries/fftw-3.3.5-win32)
    set(FFTW3_LIBRARY_RELEASE ${CMAKE_SOURCE_DIR}/../libraries/fftw-3.3.5-win32/libfftw3-3.dll)
    set(FFTW3_LIBRARY_DEBUG ${CMAKE_SOURCE_DIR}/../libraries/fftw-3.3.5-win32/libfftw3-3.dll)
    set(FFTW3_LIBRARIES ${FFTW3_LIBRARY_RELEASE})
endif()

# Add Windows-specific OpenGL libraries
if(WIN32)
    set(ADDITIONAL_WINDOWS_LIBS opengl32 gdi32 setupapi version imm32)
endif()

find_package(OpenGL REQUIRED)
if(NOT WIN32)
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
    find_package(GLEW REQUIRED STATIC)
    find_package(FFTW3 REQUIRED)
endif()


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

# Set where the ImGui files are stored
set(IMGUI_PATH  "./imgui/")

# Compile as static library 
#file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp) 

set(IMGUI_HEADER_FILES
    ${IMGUI_PATH}imconfig.h
    ${IMGUI_PATH}imgui.h
    ${IMGUI_PATH}imgui_impl_opengl3.h
    ${IMGUI_PATH}imgui_impl_opengl3_loader.h
    ${IMGUI_PATH}imgui_impl_sdl3.h
    ${IMGUI_PATH}imgui_internal.h
    ${IMGUI_PATH}imstb_rectpack.h
    ${IMGUI_PATH}imstb_textedit.h
    ${IMGUI_PATH}imstb_truetype.h
    ${IMGUI_PATH}implot.h
    ${IMGUI_PATH}implot_internal.h
)

set(IMGUI_SOURCE_FILES
    ${IMGUI_PATH}imgui.cpp
    ${IMGUI_PATH}imgui_demo.cpp
    ${IMGUI_PATH}imgui_draw.cpp
    ${IMGUI_PATH}imgui_impl_opengl3.cpp
    ${IMGUI_PATH}imgui_impl_sdl3.cpp
    ${IMGUI_PATH}imgui_tables.cpp
    ${IMGUI_PATH}imgui_widgets.cpp
    ${IMGUI_PATH}implot.cpp
    ${IMGUI_PATH}implot_items.cpp
    ${IMGUI_PATH}implot_demo.cpp
)

add_library("ImGui" STATIC ${IMGUI_SOURCE_FILES})
target_include_directories("ImGui" SYSTEM PUBLIC ${IMGUI_PATH} ${SDL3_INCLUDE_DIR})

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
    ${SDL3_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${ADDITIONAL_WINDOWS_LIBS}  # Add the Windows libraries
)