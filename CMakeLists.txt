cmake_minimum_required(VERSION 3.23)

project(sh1-editor VERSION 0.0.0)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
)