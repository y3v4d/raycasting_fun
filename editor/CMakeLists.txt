add_executable(editor main.c)
target_sources(editor PUBLIC ../src/map.c)

target_include_directories(editor PUBLIC ../include)

target_include_directories(editor PRIVATE ../libs/follia)
target_link_libraries(editor PRIVATE -lm ${follia})