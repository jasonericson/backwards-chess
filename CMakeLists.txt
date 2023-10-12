cmake_minimum_required(VERSION 3.7)
project(BackwardsChess)

find_package(OpenGL REQUIRED)

add_executable(BackwardsChess
    "main.cpp"
    "render.cpp"
    "sprite.cpp"
    "title.cpp"
    "game.cpp"
    "text.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "imgui.cpp"
    "imgui_impl_opengl3.cpp"
    "imgui_impl_sdl2.cpp"
)

set(DATA_FILES
    "pieces.png"
    "font.png"
    "title_font.png"
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (EMSCRIPTEN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=1 -s WASM=1 -s USE_WEBGL2=1 -s ASYNCIFY --emrun --shell-file ${PROJECT_SOURCE_DIR}/shell.html")
    foreach(DATA_FILE ${DATA_FILES})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file ${PROJECT_SOURCE_DIR}/build_em/${DATA_FILE}")
    endforeach()
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set_target_properties(BackwardsChess PROPERTIES OUTPUT_NAME "index")
    find_package(SDL2 REQUIRED)
else ()
    add_definitions(-DGLEW_STATIC)
    add_subdirectory(lib/glew EXCLUDE_FROM_ALL)
    add_subdirectory(lib/SDL)
    target_link_libraries(BackwardsChess PRIVATE libglew_static)
endif ()

target_link_libraries(BackwardsChess
    PRIVATE SDL2::SDL2
    PRIVATE SDL2::SDL2main
    PRIVATE ${OPENGL_LIBRARY})

target_precompile_headers(BackwardsChess PRIVATE "main.h")

if (WIN32)
    foreach(DATA_FILE ${DATA_FILES})
        add_custom_command(
            TARGET BackwardsChess PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:BackwardsChess> ${PROJECT_SOURCE_DIR}/${DATA_FILE}
            VERBATIM
        )
    endforeach()
    add_custom_command(
        TARGET BackwardsChess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:BackwardsChess> $<TARGET_FILE:SDL2::SDL2>
        VERBATIM
    )
else()
    foreach(DATA_FILE ${DATA_FILES})
        configure_file(${PROJECT_SOURCE_DIR}/${DATA_FILE} ${DATA_FILE} COPYONLY)
    endforeach()
endif()
