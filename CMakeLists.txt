cmake_minimum_required(VERSION 3.20)
# Keep Debug and RelWithDebInfo, dont want other configs
set(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo" CACHE STRING "" FORCE)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 17) # CXX17 for `filesystem`

if (WANT_X86)
    project(Windows-Game-Patches-i386 CXX)
else()
    project(Windows-Game-Patches-amd64 CXX)
endif()

include_directories(
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/external/inipp"
    "${CMAKE_SOURCE_DIR}/external/imgui"
)

# Add imgui as a static library
file(GLOB IMGUI_SOURCES 
    "${CMAKE_SOURCE_DIR}/external/imgui/*.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/*.h"
)
add_library(imgui STATIC ${IMGUI_SOURCES})

if (WANT_X86)
    # BF3.Patches was removed
else()
    add_subdirectory(source/GoWR.Tweaks)
endif()
