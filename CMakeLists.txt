cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0076 NEW)

project(raycasting)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(raycasting src/main.c)

target_sources(raycasting PUBLIC src/map.c src/tables.c src/renderer.c)
target_include_directories(raycasting PRIVATE include)
target_include_directories(raycasting PRIVATE libs/follia)
find_library(follia NAMES follia PATHS libs REQUIRED)

target_link_libraries(raycasting PRIVATE -lm ${follia})
target_compile_options(raycasting PRIVATE -O2)

add_subdirectory(editor)