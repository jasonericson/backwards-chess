cmake_minimum_required(VERSION 3.7)
project(BackwardsChess)

find_package(OpenGL REQUIRED)

add_executable(BackwardsChess "main.cpp" "render.cpp" "sprite.cpp" "game.h" "game.cpp" "text.cpp" "text.h")

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=1 -s WASM=1 -s USE_WEBGL2=1 -s ASYNCIFY --shell-file ${PROJECT_SOURCE_DIR}/shell.html --preload-file ${PROJECT_SOURCE_DIR}/build/pieces.png --preload-file ${PROJECT_SOURCE_DIR}/build/font.png")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(BackwardsChess PROPERTIES OUTPUT_NAME "index")
    find_package(SDL2 REQUIRED)
else ()
    add_definitions(-DGLEW_STATIC)
    add_subdirectory(lib/glew EXCLUDE_FROM_ALL)
    add_subdirectory(lib/SDL)
    target_link_libraries(BackwardsChess PRIVATE libglew_static)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

target_link_libraries(BackwardsChess
    PRIVATE SDL2::SDL2
    PRIVATE SDL2::SDL2main
    PRIVATE ${OPENGL_LIBRARY})

target_precompile_headers(BackwardsChess PRIVATE "main.h")

if (WIN32)
    add_custom_command(
        TARGET BackwardsChess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:BackwardsChess> $<TARGET_FILE:SDL2::SDL2>
        VERBATIM
    )
endif()

add_custom_command(
    TARGET BackwardsChess PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:BackwardsChess> ${PROJECT_SOURCE_DIR}/pieces.png ${PROJECT_SOURCE_DIR}/font.png ${PROJECT_SOURCE_DIR}/title_font.png
    VERBATIM
)
