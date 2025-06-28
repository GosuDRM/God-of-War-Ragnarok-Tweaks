cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME PROJECT_NAME)
string(REPLACE " " "_" ProjectId ${PROJECT_NAME})
project(${PROJECT_NAME})
include(win32)
file(GLOB SOURCES "*.cpp" "*.h" "../Shared/*.cpp" "../../include/*.h" "../../include/*.hpp")
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Only link imgui if it exists
if(TARGET imgui)
    target_link_libraries(${PROJECT_NAME} imgui)
    message(STATUS "Linking ${PROJECT_NAME} with imgui")
else()
    message(WARNING "imgui target not available - not linking")
endif()

include(win32_install)
