cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0135 NEW)

project(sh1-editor VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
include(FetchContent)

## nlohmann/json
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare (
	nlohmann_json
	URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/json
)
FetchContent_MakeAvailable(nlohmann_json)

## ocornut/imgui
FetchContent_Declare (
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

## raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.zip
)
FetchContent_MakeAvailable(raylib)

## rlImGui
FetchContent_Declare(
    rlImGui
    URL https://github.com/raylib-extras/rlImGui/archive/refs/tags/Raylib_6_0.zip
)
FetchContent_MakeAvailable(rlImGui)

add_library(rlImGui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
    ${rlimgui_SOURCE_DIR}/rlImGui.h
)
target_include_directories(rlImGui PUBLIC
    ${rlimgui_SOURCE_DIR}
)
target_link_libraries(rlImGui PUBLIC
    raylib
    imgui
)

# Target
file(GLOB_RECURSE PROJECT_FILES CONFIGURE_DEPENDS
	"src/*.cpp"
	"include/*.h"
)

add_executable(${PROJECT_NAME} ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    rlImGui
)
