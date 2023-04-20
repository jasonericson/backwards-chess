cmake_minimum_required(VERSION 3.7)
project(BackwardsChess)

find_package(SDL2 REQUIRED)
message(STATUS "Included SDL2 at ${SDL2_INCLUDE_DIRS}")
find_package(OpenGL REQUIRED)

add_executable(BackwardsChess "main.cpp" "render.cpp" "sprite.cpp" "game.h" "game.cpp")

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=1 -s WASM=1 -s USE_WEBGL2=1 -s ASYNCIFY --shell-file ${PROJECT_SOURCE_DIR}/shell.html --preload-file ${PROJECT_SOURCE_DIR}/build/pieces.png")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(BackwardsChess PROPERTIES OUTPUT_NAME "index")
else ()
    add_definitions(-DGLEW_STATIC)
    add_subdirectory(lib/glew EXCLUDE_FROM_ALL)
    target_link_libraries(BackwardsChess PRIVATE libglew_static)
endif ()

target_link_libraries(BackwardsChess
    PRIVATE SDL2::SDL2
    PRIVATE SDL2::SDL2main
    PRIVATE ${OPENGL_LIBRARY})

configure_file(${SDL2_BINDIR}/SDL2.dll ${CMAKE_CURRENT_BINARY_DIR}/SDL2.dll COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/pieces.png ${CMAKE_CURRENT_BINARY_DIR}/pieces.png COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/font.png ${CMAKE_CURRENT_BINARY_DIR}/font.png COPYONLY)

target_precompile_headers(BackwardsChess PRIVATE "main.h")
