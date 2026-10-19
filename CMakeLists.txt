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
	URL https://github.com/ocornut/imgui/archive/refs/tags/v1.93.0.zip
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui
)
FetchContent_MakeAvailable(imgui)

add_library(imgui INTERFACE)
target_include_directories(imgui INTERFACE
	${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_sources(imgui INTERFACE
	${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# Target
file(GLOB_RECURSE PROJECT_FILES CONFIGURE_DEPENDS
	"src/*.cpp"
	"include/*.h"
)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    nlohmann_json::nlohmann_json
    imgui
)
