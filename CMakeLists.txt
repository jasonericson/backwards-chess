cmake_minimum_required(VERSION 3.7)
project(BackwardsChess)

find_package(SDL2 REQUIRED)

add_executable(BackwardsChess "main.cpp")

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY --shell-file ${PROJECT_SOURCE_DIR}/shell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(BackwardsChess PROPERTIES OUTPUT_NAME "index")
endif ()

target_link_libraries(BackwardsChess SDL2::SDL2 SDL2::SDL2main)
